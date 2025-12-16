#include "draw_esp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "draw_patterns.h"
#include "pins.h"

static DAC8554* s_dac = nullptr;
static DrawModeESP s_mode = DRAW_MODE_CPU;
static uint16_t s_scale_x_shift = 0; // shift left by N bits
static uint16_t s_scale_y_shift = 0;
static int32_t s_offset_x = 0;
static int32_t s_offset_y = 0;

// simple buffered points
static const size_t BUF_MAX = 4096;
static uint16_t bufX[BUF_MAX];
static uint16_t bufY[BUF_MAX];
static size_t bufCount = 0;
static size_t bufHead = 0;
static esp_timer_handle_t s_timer = nullptr;
static volatile bool s_frameReady = false;
static uint64_t s_interval_us = 5; // faster default pacing
static int s_burst_per_tick = 16;   // increase burst per tick for denser output

static void timer_cb(void* arg) {
  if (!s_dac) return;
  if (!s_frameReady) return;
  if (bufHead >= bufCount) { s_frameReady = false; bufHead = 0; return; }
  int sent = 0;
  while (bufHead < bufCount && sent < s_burst_per_tick) {
    uint16_t vx = bufX[bufHead];
    uint16_t vy = bufY[bufHead];
    s_dac->setSingleValue(0, vx);
    s_dac->setSingleValue(1, vy);
    bufHead++;
    sent++;
  }
  // Pulse LDAC to latch recent samples
  digitalWrite(DAC_LDAC, HIGH);
  digitalWrite(DAC_LDAC, LOW);
  if (bufHead >= bufCount) { s_frameReady = false; bufHead = 0; }
}

void DRAW_InitESP(DAC8554* dac, uint32_t interval_us) {
  s_dac = dac;
  s_mode = DRAW_MODE_CPU;
  s_scale_x_shift = 4; // default 12->16 bit upshift
  s_scale_y_shift = 4;
  s_offset_x = 0;
  s_offset_y = 0;
  s_interval_us = (interval_us == 0) ? 5 : interval_us;

  if (s_timer) {
    esp_timer_stop(s_timer);
    esp_timer_delete(s_timer);
    s_timer = nullptr;
  }
  esp_timer_create_args_t args = {};
  args.callback = &timer_cb;
  args.arg = nullptr;
  args.dispatch_method = ESP_TIMER_TASK;
  args.name = "draw_timer";
  esp_timer_create(&args, &s_timer);
}

void DRAW_SetModeESP(DrawModeESP mode) {
  s_mode = mode;
  if (!s_timer) return;
  esp_timer_stop(s_timer);
  if (s_mode == DRAW_MODE_TIMER_BUFFERED) {
    esp_timer_start_periodic(s_timer, s_interval_us);
  }
}
void DRAW_SetScaleESP(uint16_t scale_x_shift, uint16_t scale_y_shift) {
  s_scale_x_shift = scale_x_shift; s_scale_y_shift = scale_y_shift;
}
void DRAW_SetOffsetESP(int32_t offset_x, int32_t offset_y) {
  s_offset_x = offset_x; s_offset_y = offset_y;
}

void DRAW_BeginFrameESP() { bufHead = 0; bufCount = 0; s_frameReady = false; }

void DRAW_AddPointESP(uint16_t x12, uint16_t y12) {
  // apply offset + scaling via shift
  uint32_t x16 = ((uint32_t)x12 + (uint32_t)s_offset_x) << s_scale_x_shift;
  uint32_t y16 = ((uint32_t)y12 + (uint32_t)s_offset_y) << s_scale_y_shift;
  if (x16 > 65535) x16 = 65535; if (y16 > 65535) y16 = 65535;
  if (s_mode == DRAW_MODE_CPU) {
    if (s_dac) {
      s_dac->setValue(0, (uint16_t)x16);
      s_dac->setValue(1, (uint16_t)y16);
    }
  } else {
    if (bufCount < BUF_MAX) {
      bufX[bufCount] = (uint16_t)x16;
      bufY[bufCount] = (uint16_t)y16;
      bufCount++;
    }
  }
}

void DRAW_EndFrameESP() {
  if (s_mode == DRAW_MODE_TIMER_BUFFERED) {
    bufHead = 0;
    s_frameReady = true;
  }
}

// Bresenham-like simple interpolation for line into points
void DRAW_RenderLineESP(float x0, float y0, float x1, float y1) {
  // Map original coordinate space (~0..3300) into 12-bit (~0..4095)
  const float to12 = 4095.0f / 3300.0f;
  const int steps = 96; // finer for fidelity yet bounded CPU
  for (int i=0;i<=steps;i++) {
    float t = (float)i / (float)steps;
    float xf = x0 + (x1 - x0) * t;
    float yf = y0 + (y1 - y0) * t;
    int32_t x12 = (int32_t)(xf * to12);
    int32_t y12 = (int32_t)(yf * to12);
    if (x12 < 0) x12 = 0; if (x12 > 4095) x12 = 4095;
    if (y12 < 0) y12 = 0; if (y12 > 4095) y12 = 4095;
    DRAW_AddPointESP((uint16_t)x12, (uint16_t)y12);
  }
}

void DRAW_RenderCharESP(char c, int32_t ox, int32_t oy, uint16_t scale_pct, uint16_t spacing) {
  const GlyphPattern* gp = getGlyphPattern(c);
  if (!gp || gp->count == 0) return;
  float s = (float)scale_pct / 100.0f;
  for (uint8_t i=0;i<gp->count;i++) {
    float x0 = gp->segs[i].x0 * s + (float)ox;
    float y0 = gp->segs[i].y0 * s + (float)oy;
    float x1 = gp->segs[i].x1 * s + (float)ox;
    float y1 = gp->segs[i].y1 * s + (float)oy;
    DRAW_RenderLineESP(x0, y0, x1, y1);
  }
}

void DRAW_RenderStringESP(const char* s, int32_t ox, int32_t oy, uint16_t scale_pct, uint16_t spacing) {
  if (!s) return;
  int32_t x = ox;
  for (const char* p = s; *p; ++p) {
    DRAW_RenderCharESP(*p, x, oy, scale_pct, spacing);
    x += spacing;
  }
}
