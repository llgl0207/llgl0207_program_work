#pragma once
#include <Arduino.h>
#include "DAC8554.h"

// Modes
typedef enum {
  DRAW_MODE_CPU = 0,
  DRAW_MODE_TIMER_BUFFERED = 1
} DrawModeESP;

void DRAW_InitESP(DAC8554* dac, uint32_t interval_us);
void DRAW_SetModeESP(DrawModeESP mode);
void DRAW_SetScaleESP(uint16_t scale_x_shift, uint16_t scale_y_shift);
void DRAW_SetOffsetESP(int32_t offset_x, int32_t offset_y);

// Render a single line segment collection as points
void DRAW_BeginFrameESP();
void DRAW_AddPointESP(uint16_t x12, uint16_t y12);
void DRAW_EndFrameESP();

// Render helpers for glyphs/strings (12-bit coordinate space)
void DRAW_RenderLineESP(float x0, float y0, float x1, float y1);
void DRAW_RenderCharESP(char c, int32_t ox, int32_t oy, uint16_t scale_pct, uint16_t spacing);
void DRAW_RenderStringESP(const char* s, int32_t ox, int32_t oy, uint16_t scale_pct, uint16_t spacing);
