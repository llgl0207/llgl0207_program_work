#include "draw.h"
#include "dac.h"
#include <math.h>
#include <string.h>

typedef struct { uint16_t x0,y0,x1,y1; } Line_t;

// For brevity, include a compact set of patterns (A..Z) copied from main.c
// In a real library we'd store these more compactly or generate them.
static const Line_t pattern_A[] = { {0,0,2048,4096}, {2048,4096,4096,0}, {1024,2048,3072,2048} };
static const Line_t pattern_B[] = { {0,0,0,4096}, {0,4096,2048,4096}, {2048,4096,3072,3072}, {3072,3072,2048,2048}, {2048,2048,0,2048}, {2048,2048,3072,1024}, {3072,1024,2048,0}, {2048,0,0,0} };
static const Line_t pattern_C[] = { {3000,1000,2000,800}, {2000,800,1200,1400}, {1200,1400,1000,2600}, {1000,2600,1200,3200}, {1200,3200,2000,3800}, {2000,3800,3000,3600} };
static const Line_t pattern_D[] = { {1200,800,1200,3800}, {1200,800,2200,1200}, {2200,1200,2600,2000}, {2600,2000,2200,3000}, {2200,3000,1200,3200} };
static const Line_t pattern_E[] = { {1200,800,1200,3800}, {1200,800,3000,800}, {1200,2300,2600,2300}, {1200,3800,3000,3800} };
static const Line_t pattern_F[] = { {1200,3295,1200,295}, {1200,3295,3000,3295}, {1200,1795,2600,1795} };
static const Line_t pattern_G[] = { {3000,1200,2000,800}, {2000,800,1200,1400}, {1200,1400,1000,2600}, {1000,2600,1200,3200}, {1200,3200,2200,3800}, {2200,3800,3000,3600}, {2500,2500,3000,2500} };
static const Line_t pattern_H[] = { {1200,800,1200,3800}, {3000,800,3000,3800}, {1200,2300,3000,2300} };
static const Line_t pattern_I[] = { {2000,800,2800,800}, {2400,800,2400,3800}, {2000,3800,2800,3800} };
static const Line_t pattern_J[] = { {1400,3295,3400,3295}, {2400,3295,2400,1095}, {2400,1095,2000,495} };
static const Line_t pattern_K[] = { {1200,800,1200,3800}, {3000,800,1200,2300}, {1200,2300,3000,3800} };
static const Line_t pattern_L[] = { {1200,3295,1200,295}, {1200,295,3000,295} };
static const Line_t pattern_M[] = { {1200,3295,1200,295}, {1200,3295,2200,2095}, {2200,2095,3000,3295}, {3000,3295,3000,295} };
static const Line_t pattern_N[] = { {1200,3295,1200,295}, {1200,3295,3000,295}, {3000,3295,3000,295} };
static const Line_t pattern_O[] = { {2000,3295,3000,2895}, {3000,2895,3200,1495}, {3200,1495,2600,495}, {2600,495,1600,895}, {1600,895,1400,2095}, {1400,2095,2000,3295} };
static const Line_t pattern_P[] = { {1200,3295,1200,295}, {1200,3295,2600,2995}, {2600,2995,1200,2095} };
static const Line_t pattern_Q[] = { {2000,3295,3000,2895}, {3000,2895,3200,1495}, {3200,1495,2600,495}, {2600,495,1600,895}, {1600,895,1400,2095}, {1400,2095,2000,3295}, {2500,1095,3200,295} };
static const Line_t pattern_R[] = { {1200,3295,1200,295}, {1200,3295,2600,2995}, {2600,2995,1200,2095}, {1200,2095,3000,295} };
static const Line_t pattern_S[] = { {1024,0,3072,1024}, {3072,1024,1024,3072}, {1024,3072,3072,4096} };
static const Line_t pattern_T[] = { {1400,3295,3400,3295}, {2400,3295,2400,295} };
static const Line_t pattern_U[] = { {1200,3295,1200,895}, {1200,895,2200,295}, {2200,295,3200,895}, {3200,895,3200,3295} };
static const Line_t pattern_V[] = { {1200,3295,2200,295}, {2200,295,3200,3295} };
static const Line_t pattern_W[] = { {1200,3295,1600,295}, {1600,295,2200,2095}, {2200,2095,2800,295}, {2800,295,3200,3295} };
static const Line_t pattern_X[] = { {1200,3295,3200,295}, {3200,3295,1200,295} };
static const Line_t pattern_Y[] = { {1200,3295,2200,1895}, {3200,3295,2200,1895}, {2200,1895,2200,295} };
static const Line_t pattern_Z[] = { {1200,3295,3200,3295}, {3200,3295,1200,295}, {1200,295,3200,295} };

// Digits 0-9
static const Line_t pattern_0[] = { {1200,3295,3000,3295}, {3000,3295,3000,295}, {3000,295,1200,295}, {1200,295,1200,3295} };
static const Line_t pattern_1[] = { {2100,3295,2100,295} };
static const Line_t pattern_2[] = { {1200,3295,3000,3295}, {3000,3295,3000,1795}, {3000,1795,1200,1795}, {1200,1795,1200,295}, {1200,295,3000,295} };
static const Line_t pattern_3[] = { {1200,3295,3000,3295}, {3000,3295,3000,295}, {3000,295,1200,295}, {1200,1795,3000,1795} };
static const Line_t pattern_4[] = { {1200,3295,1200,1795}, {1200,1795,3000,1795}, {3000,3295,3000,295} };
static const Line_t pattern_5[] = { {3000,3295,1200,3295}, {1200,3295,1200,1795}, {1200,1795,3000,1795}, {3000,1795,3000,295}, {3000,295,1200,295} };
static const Line_t pattern_6[] = { {3000,3295,1200,3295}, {1200,3295,1200,295}, {1200,295,3000,295}, {3000,295,3000,1795}, {3000,1795,1200,1795} };
static const Line_t pattern_7[] = { {1200,3295,3000,3295}, {3000,3295,3000,295} };
static const Line_t pattern_8[] = { {1200,3295,3000,3295}, {3000,3295,3000,295}, {3000,295,1200,295}, {1200,295,1200,3295}, {1200,1795,3000,1795} };
static const Line_t pattern_9[] = { {3000,295,3000,3295}, {3000,3295,1200,3295}, {1200,3295,1200,1795}, {1200,1795,3000,1795} };

static const Line_t * const patterns[] = {
  pattern_A, pattern_B, pattern_C, pattern_D, pattern_E, pattern_F, pattern_G,
  pattern_H, pattern_I, pattern_J, pattern_K, pattern_L, pattern_M, pattern_N,
  pattern_O, pattern_P, pattern_Q, pattern_R, pattern_S, pattern_T, pattern_U,
  pattern_V, pattern_W, pattern_X, pattern_Y, pattern_Z,
  pattern_0, pattern_1, pattern_2, pattern_3, pattern_4,
  pattern_5, pattern_6, pattern_7, pattern_8, pattern_9
};
static const uint8_t pattern_lengths[] = {
  sizeof(pattern_A)/sizeof(pattern_A[0]), sizeof(pattern_B)/sizeof(pattern_B[0]), sizeof(pattern_C)/sizeof(pattern_C[0]),
  sizeof(pattern_D)/sizeof(pattern_D[0]), sizeof(pattern_E)/sizeof(pattern_E[0]), sizeof(pattern_F)/sizeof(pattern_F[0]),
  sizeof(pattern_G)/sizeof(pattern_G[0]), sizeof(pattern_H)/sizeof(pattern_H[0]), sizeof(pattern_I)/sizeof(pattern_I[0]),
  sizeof(pattern_J)/sizeof(pattern_J[0]), sizeof(pattern_K)/sizeof(pattern_K[0]), sizeof(pattern_L)/sizeof(pattern_L[0]),
  sizeof(pattern_M)/sizeof(pattern_M[0]), sizeof(pattern_N)/sizeof(pattern_N[0]), sizeof(pattern_O)/sizeof(pattern_O[0]),
  sizeof(pattern_P)/sizeof(pattern_P[0]), sizeof(pattern_Q)/sizeof(pattern_Q[0]), sizeof(pattern_R)/sizeof(pattern_R[0]),
  sizeof(pattern_S)/sizeof(pattern_S[0]), sizeof(pattern_T)/sizeof(pattern_T[0]), sizeof(pattern_U)/sizeof(pattern_U[0]),
  sizeof(pattern_V)/sizeof(pattern_V[0]), sizeof(pattern_W)/sizeof(pattern_W[0]), sizeof(pattern_X)/sizeof(pattern_X[0]),
  sizeof(pattern_Y)/sizeof(pattern_Y[0]), sizeof(pattern_Z)/sizeof(pattern_Z[0]),
  sizeof(pattern_0)/sizeof(pattern_0[0]), sizeof(pattern_1)/sizeof(pattern_1[0]), sizeof(pattern_2)/sizeof(pattern_2[0]),
  sizeof(pattern_3)/sizeof(pattern_3[0]), sizeof(pattern_4)/sizeof(pattern_4[0]), sizeof(pattern_5)/sizeof(pattern_5[0]),
  sizeof(pattern_6)/sizeof(pattern_6[0]), sizeof(pattern_7)/sizeof(pattern_7[0]), sizeof(pattern_8)/sizeof(pattern_8[0]),
  sizeof(pattern_9)/sizeof(pattern_9[0])
};
static const uint8_t patterns_count = sizeof(patterns)/sizeof(patterns[0]);

static uint8_t pattern_index = 0;
static const Line_t *current_pattern = NULL;
static uint8_t current_pattern_length = 0;
static uint8_t line_index = 0;

static uint16_t current_step = 0;
static uint16_t step_max = 0;
static uint8_t state_done = 0; // 0=init,1=drawing,2=finished line

static uint32_t switch_interval = 1000;
// Transformation: scale (percent) and offset (in DAC units)
static uint16_t scale_x_pct = 100;
static uint16_t scale_y_pct = 100;
static int32_t offset_x = 0;
static int32_t offset_y = 0;

// transformed endpoints cache for current line
static int32_t tx0, ty0, tx1, ty1;
// Memory Pool Settings
#define MAX_DRAW_OBJS 8
#define MAX_STR_LEN 32

typedef struct {
  uint8_t active;
  char text[MAX_STR_LEN];
  int32_t x, y;
  uint16_t sx, sy;
  uint16_t spacing;
} DrawObj;

static DrawObj draw_pool[MAX_DRAW_OBJS];

// Runtime State
static int8_t current_obj_idx = -1;
static uint8_t current_char_idx = 0;
static int32_t current_char_x = 0;

// helper: compute pattern min/max X
static void compute_pattern_minmax_x(const Line_t *p, uint8_t len, int32_t *minx, int32_t *maxx){
  int32_t mn = 4096, mx = 0;
  uint8_t i;
  for(i=0;i<len;i++){
    if((int32_t)p[i].x0 < mn) mn = p[i].x0;
    if((int32_t)p[i].x1 < mn) mn = p[i].x1;
    if((int32_t)p[i].x0 > mx) mx = p[i].x0;
    if((int32_t)p[i].x1 > mx) mx = p[i].x1;
  }
  *minx = mn; *maxx = mx;
}

// Set pattern by character (A-Z, 0-9). returns 1 if set, 0 otherwise
static uint8_t set_pattern_by_char(char c){
  uint8_t idx = 255;
  if(c>='A' && c<='Z'){
    idx = (uint8_t)(c - 'A');
  } else if(c>='0' && c<='9'){
    idx = (uint8_t)(c - '0') + 26;
  }
  
  if(idx < patterns_count){
    pattern_index = idx;
    current_pattern = patterns[pattern_index];
    current_pattern_length = pattern_lengths[pattern_index];
    if(current_pattern_length == 0) current_pattern_length = 1;
    line_index = 0;
    state_done = 0; // Start at state 0 to ensure first point is drawn
    return 1;
  }
  return 0;
}

void DRAW_Clear(void){
  for(int i=0; i<MAX_DRAW_OBJS; i++) draw_pool[i].active = 0;
  current_obj_idx = -1;
  state_done = 0;
  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2048);
}

void DRAW_Init(uint32_t interval_ms){
  if(patterns_count == 0) return;
  switch_interval = interval_ms;
  DRAW_Clear();
  // start DAC centers
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
}

void DRAW_SetInterval(uint32_t ms){ switch_interval = ms; }

void DRAW_SetScale(uint16_t scale_x_percent, uint16_t scale_y_percent){
  if(scale_x_percent == 0) scale_x_percent = 1;
  if(scale_y_percent == 0) scale_y_percent = 1;
  scale_x_pct = scale_x_percent;
  scale_y_pct = scale_y_percent;
}

void DRAW_SetOffset(int16_t offset_x_param, int16_t offset_y_param){
  offset_x = offset_x_param;
  offset_y = offset_y_param;
}

uint8_t DRAW_AddString(const char *s, uint16_t spacing, int32_t x, int32_t y, uint16_t sx, uint16_t sy){
  int slot = -1;
  for(int i=0; i<MAX_DRAW_OBJS; i++){
    if(!draw_pool[i].active){ slot = i; break; }
  }
  if(slot < 0) return 0;

  strncpy(draw_pool[slot].text, s, MAX_STR_LEN-1);
  draw_pool[slot].text[MAX_STR_LEN-1] = '\0';
  draw_pool[slot].x = x;
  draw_pool[slot].y = y;
  draw_pool[slot].sx = sx;
  draw_pool[slot].sy = sy;
  draw_pool[slot].spacing = spacing;
  draw_pool[slot].active = 1;
  
  if(current_obj_idx == -1){
      current_obj_idx = slot;
      current_char_idx = 0;
      current_char_x = x;
      if(draw_pool[slot].text[0]){
          if(set_pattern_by_char(draw_pool[slot].text[0])){
             int32_t minx, maxx;
             compute_pattern_minmax_x(current_pattern, current_pattern_length, &minx, &maxx);
             int32_t left_offset = current_char_x - (minx * (int32_t)sx) / 100;
             DRAW_SetOffset((int16_t)left_offset, (int16_t)y);
             DRAW_SetScale(sx, sy);
          }
      }
  }
  return 1;
}

void DRAW_DrawString(const char *s, uint16_t spacing, int32_t base_x, int32_t base_y,
                     uint16_t scale_x_percent, uint16_t scale_y_percent){
    DRAW_Clear();
    DRAW_AddString(s, spacing, base_x, base_y, scale_x_percent, scale_y_percent);
}

uint8_t DRAW_IsBusy(void){
  return (uint8_t)(current_obj_idx != -1);
}

void DRAW_SetLetter(char c){
    char buf[2] = {c, 0};
    DRAW_Clear();
    DRAW_AddString(buf, 0, offset_x, offset_y, scale_x_pct, scale_y_pct);
}

void DRAW_AutoTick(uint32_t now){
  // Not supported in pool mode
}

void DRAW_TimerStep(TIM_HandleTypeDef *htim){
  if(htim->Instance != TIM14) return;
  if(current_obj_idx == -1) return;

  if(state_done==0){
    state_done = 1;
    // compute step_max
    // compute transformed endpoints with scale and offset
    int32_t ox0 = (int32_t)current_pattern[line_index].x0;
    int32_t oy0 = (int32_t)current_pattern[line_index].y0;
    int32_t ox1 = (int32_t)current_pattern[line_index].x1;
    int32_t oy1 = (int32_t)current_pattern[line_index].y1;
    tx0 = (ox0 * (int32_t)scale_x_pct) / 100 + offset_x;
    ty0 = (oy0 * (int32_t)scale_y_pct) / 100 + offset_y;
    tx1 = (ox1 * (int32_t)scale_x_pct) / 100 + offset_x;
    ty1 = (oy1 * (int32_t)scale_y_pct) / 100 + offset_y;
    if(tx0 < 0) tx0 = 0; if(tx0 > 4095) tx0 = 4095;
    if(tx1 < 0) tx1 = 0; if(tx1 > 4095) tx1 = 4095;
    if(ty0 < 0) ty0 = 0; if(ty0 > 4095) ty0 = 4095;
    if(ty1 < 0) ty1 = 0; if(ty1 > 4095) ty1 = 4095;
    int32_t dx = tx0 - tx1;
    int32_t dy = ty0 - ty1;
    step_max = (uint16_t)(sqrt((double)dx*dx + (double)dy*dy)*1);
    current_step = 0;
    
    // Output start point immediately to ensure we don't skip it
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint32_t)tx0);
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (uint32_t)ty0);
  }
  else if(state_done==1){
    current_step++;
    if(current_step > step_max){
        state_done = 2;
    } else {
        if(step_max!=0){
          int32_t curX = (tx0 * (int32_t)(step_max - current_step) + tx1 * (int32_t)current_step) / (int32_t)step_max;
          int32_t curY = (ty0 * (int32_t)(step_max - current_step) + ty1 * (int32_t)current_step) / (int32_t)step_max;
          if(curX<0) curX=0; if(curX>4095) curX=4095;
          if(curY<0) curY=0; if(curY>4095) curY=4095;
          HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint32_t)curX);
          HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (uint32_t)curY);
        }
    }
  }
  else if(state_done==2){
    // advance to next line
    line_index = (line_index + 1) % current_pattern_length;
    state_done = 0;
    // If we've just wrapped to line 0, it means the glyph finished one full pass.
    if(line_index == 0){
        // Char finished.
        // 1. Advance X position for next char
        int32_t minx, maxx;
        compute_pattern_minmax_x(current_pattern, current_pattern_length, &minx, &maxx);
        int32_t gw = maxx - minx;
        int32_t scaled_gw = (gw * (int32_t)draw_pool[current_obj_idx].sx) / 100;
        current_char_x += scaled_gw + draw_pool[current_obj_idx].spacing;

        // 2. Advance index
        current_char_idx++;
        
        // 3. Check end of string
        if(current_char_idx >= MAX_STR_LEN || draw_pool[current_obj_idx].text[current_char_idx] == '\0'){
            // Object finished. Find next active object.
            int start_search = current_obj_idx + 1;
            int found = -1;
            // Search forward
            for(int i=0; i<MAX_DRAW_OBJS; i++){
                int idx = (start_search + i) % MAX_DRAW_OBJS;
                if(draw_pool[idx].active){
                    found = idx;
                    break;
                }
            }
            
            if(found != -1){
                current_obj_idx = found;
                current_char_idx = 0;
                current_char_x = draw_pool[found].x;
            } else {
                // No active objects? Should not happen if we are here.
                current_obj_idx = -1;
                return;
            }
        }

        // 4. Setup next char
        char nc = draw_pool[current_obj_idx].text[current_char_idx];
        if(set_pattern_by_char(nc)){
             // Update global scale/offset for the drawing engine
             scale_x_pct = draw_pool[current_obj_idx].sx;
             scale_y_pct = draw_pool[current_obj_idx].sy;
             
             int32_t nminx, nmaxx;
             compute_pattern_minmax_x(current_pattern, current_pattern_length, &nminx, &nmaxx);
             int32_t left_offset = current_char_x - (nminx * (int32_t)scale_x_pct) / 100;
             
             offset_x = (int16_t)left_offset;
             offset_y = (int16_t)draw_pool[current_obj_idx].y;
        } else {
             // Char not found? Skip or stop?
             // For now, set_pattern_by_char returns 0.
             // We should probably handle spaces.
        }
    }
  }
}
