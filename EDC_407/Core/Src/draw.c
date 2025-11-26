#include "draw.h"
#include "dac.h"
#include <math.h>
#include <string.h>

// DMA Buffers
uint16_t DAC_Buff_X[DRAW_BUF_SIZE];
uint16_t DAC_Buff_Y[DRAW_BUF_SIZE];
uint32_t DAC_Buff_Count = 0;

typedef struct { uint16_t x0,y0,x1,y1; } Line_t;

static uint8_t set_pattern_by_char(char c);
static void compute_pattern_minmax_x(const Line_t *p, uint8_t len, int32_t *minx, int32_t *maxx);

// For brevity, include a compact set of patterns (A..Z) copied from main.c
// In a real library we'd store these more compactly or generate them.
static const Line_t pattern_A[] = { {0,0,2048,4096}, {2048,4096,4096,0}, {1024,2048,3072,2048} };
static const Line_t pattern_B[] = { {0,0,0,4096}, {0,4096,1365,4096}, {1365,4096,2048,3413}, {2048,3413,2048,2730}, {2048,2730,1365,2048}, {1365,2048,0,2048}, {0,2048,1365,2048}, {1365,2048,2048,1365},{2048,1365,2048,682},{2048,682,1365,0},{1365,0,0,0} };
static const Line_t pattern_C[] = { {3000,1000,2000,800}, {2000,800,1200,1400}, {1200,1400,1000,2600}, {1000,2600,1200,3200}, {1200,3200,2000,3800}, {2000,3800,3000,3600} };
static const Line_t pattern_D[] = { {1200,800,1200,3800}, {1200,3800,2200,3800}, {2200,3800,2600,2800}, {2600,2800,2600,1800}, {2600,1800,2200,800},{2200,800,1200,800} };
static const Line_t pattern_E[] = { {1200,800,1200,3800}, {1200,800,3000,800}, {1200,2300,2600,2300}, {1200,3800,3000,3800} };
static const Line_t pattern_F[] = { {1200,3295,1200,295}, {1200,3295,3000,3295}, {1200,1795,2600,1795} };
static const Line_t pattern_G[] = { {3000,1200,2000,800}, {2000,800,1200,1400}, {1200,1400,1000,2600}, {1000,2600,1200,3200}, {1200,3200,2200,3800}, {2200,3800,3000,3600}, {2500,2500,3000,2500} };
static const Line_t pattern_H[] = { {1200,800,1200,3800}, {3000,800,3000,3800}, {1200,2300,3000,2300} };
static const Line_t pattern_I[] = { {2000,800,2500,800}, {2250,800,2250,3800}, {2000,3800,2500,3800} };
static const Line_t pattern_J[] = { {1700,3295,2300,3295}, {2000,3295,2000,1095}, {2000,1095,1800,1000},{1800,1000,1700,1000} };
static const Line_t pattern_K[] = { {1200,800,1200,3800}, {2500,800,1200,2300}, {1200,2300,2500,3800} };
static const Line_t pattern_L[] = { {1200,3295,1200,295}, {1200,295,2400,295} };
static const Line_t pattern_M[] = { {1200,3295,1200,295}, {1200,3295,2200,2095}, {2200,2095,3000,3295}, {3000,3295,3000,295} };
static const Line_t pattern_N[] = { {1200,3295,1200,295}, {1200,3295,3000,295}, {3000,3295,3000,295} };
static const Line_t pattern_O[] = { {2000,3295,3000,2895}, {3000,2895,3200,1495}, {3200,1495,2600,495}, {2600,495,1600,895}, {1600,895,1400,2095}, {1400,2095,2000,3295} };
static const Line_t pattern_P[] = { {1200,3295,1200,295}, {1200,3295,2300,3295}, {2300,3295,2500,2795},{2500,2795,2500,2395},{2500,2395,2300,1795},{2300,1795,1200,1795} };
static const Line_t pattern_Q[] = { {2000,3295,3000,2895}, {3000,2895,3200,1495}, {3200,1495,2600,495}, {2600,495,1600,895}, {1600,895,1400,2095}, {1400,2095,2000,3295}, {2500,1095,3200,295} };
static const Line_t pattern_R[] = { {1200,3295,1200,295}, {1200,3295,2300,3295}, {2300,3295,2500,2795},{2500,2795,2500,2395},{2500,2395,2300,1795},{2300,1795,1200,1795}, {1200,1795,3000,295} };
static const Line_t pattern_S[] = { {1365,0,2048,0}, {2048,0,3072,1024}, {3072,1024,1024,3072},{1024,3072,2048,4096},{2048,4096,2730,4096} };
static const Line_t pattern_T[] = { {2000,3295,2800,3295}, {2400,3295,2400,295} };
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

// Symbols
static const Line_t pattern_excl[] = { {2100,3295,2100,1295}, {2100,795,2100,295} }; // !
static const Line_t pattern_apos[] = { {2100,3295,2100,2295} }; // '
static const Line_t pattern_hash[] = { {1600,3295,1600,295}, {2600,3295,2600,295}, {1200,2295,3000,2295}, {1200,1295,3000,1295} }; // #
static const Line_t pattern_pct[] = { {1200,295,3000,3295}, {1400,3095,1600,3095}, {1600,3095,1600,2895}, {1600,2895,1400,2895}, {1400,2895,1400,3095}, {2600,695,2800,695}, {2800,695,2800,495}, {2800,495,2600,495}, {2600,495,2600,695} }; // % (simplified circles)
static const Line_t pattern_caret[] = { {1200,1795,2100,3295}, {2100,3295,3000,1795} }; // ^
static const Line_t pattern_ast[] = { {1200,2795,3000,795}, {3000,2795,1200,795}, {2100,3295,2100,295}, {1200,1795,3000,1795} }; // *
static const Line_t pattern_under[] = { {1200,295,3000,295} }; // _
static const Line_t pattern_minus[] = { {1200,1795,3000,1795} }; // -
static const Line_t pattern_plus[] = { {2100,3295,2100,295}, {1200,1795,3000,1795} }; // +
static const Line_t pattern_eq[] = { {1200,2295,3000,2295}, {1200,1295,3000,1295} }; // =
static const Line_t pattern_bslash[] = { {1200,3295,3000,295} }; // \ (backslash)
static const Line_t pattern_fslash[] = { {1200,295,3000,3295} }; // /
static const Line_t pattern_lparen[] = { {2600,3295,1600,1795}, {1600,1795,2600,295} }; // (
static const Line_t pattern_rparen[] = { {1600,3295,2600,1795}, {2600,1795,1600,295} }; // )
static const Line_t pattern_lbrack[] = { {2600,3295,1600,3295}, {1600,3295,1600,295}, {1600,295,2600,295} }; // [
static const Line_t pattern_rbrack[] = { {1600,3295,2600,3295}, {2600,3295,2600,295}, {2600,295,1600,295} }; // ]
static const Line_t pattern_lbrace[] = { {2600,3295,2100,3295}, {2100,3295,2100,1795}, {2100,1795,1600,1795}, {2100,1795,2100,295}, {2100,295,2600,295} }; // {
static const Line_t pattern_rbrace[] = { {1600,3295,2100,3295}, {2100,3295,2100,1795}, {2100,1795,2600,1795}, {2100,1795,2100,295}, {2100,295,1600,295} }; // }
static const Line_t pattern_quote[] = { {1600,3295,1600,2295}, {2600,3295,2600,2295} }; // "
static const Line_t pattern_semi[] = { {2100,3295,2100,2795}, {2100,1295,1700,295} }; // ; (dot + line)
static const Line_t pattern_colon[] = { {2100,3295,2100,2295}, {2100,1295,2100,295} }; // :
static const Line_t pattern_comma[] = { {2100,495,2100,295}, {2100,295,1700,95} }; // ,
static const Line_t pattern_period[] = { {1900,495,2300,495}, {2300,495,2300,295}, {2300,295,1900,295}, {1900,295,1900,495} }; // .
static const Line_t pattern_question[] = { {1200,2595,1200,2995}, {1200,2995,1800,3295}, {1800,3295,2400,3295}, {2400,3295,3000,2995}, {3000,2995,3000,2295}, {3000,2295,2100,1595}, {2100,1595,2100,1095}, {2100,595,2100,295} }; // ?
static const Line_t pattern_at[] = { {2600,1295,2200,1295}, {2200,1295,1800,1695}, {1800,1695,1800,2095}, {1800,2095,2200,2495}, {2200,2495,2600,2095}, {2600,2095,2600,1695}, {2600,1695,3000,1295}, {3000,1295,3000,2895}, {3000,2895,1400,2895}, {1400,2895,1400,895}, {1400,895,3000,895} }; // @
static const Line_t pattern_dollar[] = { {2600,3295,1600,3295}, {1600,3295,1600,2095}, {1600,2095,2600,2095}, {2600,2095,2600,895}, {2600,895,1600,895}, {2100,3695,2100,495} }; // $
static const Line_t pattern_lt[] = { {2600,3295,1200,1795}, {1200,1795,2600,295} }; // <
static const Line_t pattern_gt[] = { {1200,3295,2600,1795}, {2600,1795,1200,295} }; // >
static const Line_t pattern_pipe[] = { {2100,3295,2100,295} }; // |
static const Line_t pattern_tilde[] = { {1200,1295,1600,2295}, {1600,2295,2200,1295}, {2200,1295,2600,2295} }; // ~

static const Line_t * const patterns[] = {
  pattern_A, pattern_B, pattern_C, pattern_D, pattern_E, pattern_F, pattern_G,
  pattern_H, pattern_I, pattern_J, pattern_K, pattern_L, pattern_M, pattern_N,
  pattern_O, pattern_P, pattern_Q, pattern_R, pattern_S, pattern_T, pattern_U,
  pattern_V, pattern_W, pattern_X, pattern_Y, pattern_Z,
  pattern_0, pattern_1, pattern_2, pattern_3, pattern_4,
  pattern_5, pattern_6, pattern_7, pattern_8, pattern_9,
  pattern_excl, pattern_apos, pattern_hash, pattern_pct, pattern_caret,
  pattern_ast, pattern_under, pattern_minus, pattern_plus, pattern_eq,
  pattern_bslash, pattern_fslash, pattern_lparen, pattern_rparen,
  pattern_lbrack, pattern_rbrack, pattern_lbrace, pattern_rbrace,
  pattern_quote, pattern_semi, pattern_colon,
  pattern_comma, pattern_period, pattern_question,
  pattern_at, pattern_dollar, pattern_lt, pattern_gt, pattern_pipe, pattern_tilde
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
  sizeof(pattern_9)/sizeof(pattern_9[0]),
  sizeof(pattern_excl)/sizeof(pattern_excl[0]), sizeof(pattern_apos)/sizeof(pattern_apos[0]), sizeof(pattern_hash)/sizeof(pattern_hash[0]),
  sizeof(pattern_pct)/sizeof(pattern_pct[0]), sizeof(pattern_caret)/sizeof(pattern_caret[0]), sizeof(pattern_ast)/sizeof(pattern_ast[0]),
  sizeof(pattern_under)/sizeof(pattern_under[0]), sizeof(pattern_minus)/sizeof(pattern_minus[0]), sizeof(pattern_plus)/sizeof(pattern_plus[0]),
  sizeof(pattern_eq)/sizeof(pattern_eq[0]), sizeof(pattern_bslash)/sizeof(pattern_bslash[0]), sizeof(pattern_fslash)/sizeof(pattern_fslash[0]),
  sizeof(pattern_lparen)/sizeof(pattern_lparen[0]), sizeof(pattern_rparen)/sizeof(pattern_rparen[0]), sizeof(pattern_lbrack)/sizeof(pattern_lbrack[0]),
  sizeof(pattern_rbrack)/sizeof(pattern_rbrack[0]), sizeof(pattern_lbrace)/sizeof(pattern_lbrace[0]), sizeof(pattern_rbrace)/sizeof(pattern_rbrace[0]),
  sizeof(pattern_quote)/sizeof(pattern_quote[0]), sizeof(pattern_semi)/sizeof(pattern_semi[0]), sizeof(pattern_colon)/sizeof(pattern_colon[0]),
  sizeof(pattern_comma)/sizeof(pattern_comma[0]), sizeof(pattern_period)/sizeof(pattern_period[0]), sizeof(pattern_question)/sizeof(pattern_question[0]),
  sizeof(pattern_at)/sizeof(pattern_at[0]), sizeof(pattern_dollar)/sizeof(pattern_dollar[0]), sizeof(pattern_lt)/sizeof(pattern_lt[0]),
  sizeof(pattern_gt)/sizeof(pattern_gt[0]), sizeof(pattern_pipe)/sizeof(pattern_pipe[0]), sizeof(pattern_tilde)/sizeof(pattern_tilde[0])
};
static const uint8_t patterns_count = sizeof(patterns)/sizeof(patterns[0]);

static uint8_t pattern_index = 0;
static const Line_t *current_pattern = NULL;
static uint8_t current_pattern_length = 0;

// Transformation: scale (percent) and offset (in DAC units)
static uint16_t scale_x_pct = 100;
static uint16_t scale_y_pct = 100;
static int32_t offset_x = 0;
static int32_t offset_y = 0;

// Memory Pool Settings
#define MAX_DRAW_OBJS 16
#define MAX_STR_LEN 64

typedef struct {
  uint8_t active;
  char text[MAX_STR_LEN];
  int32_t x, y;
  uint16_t sx, sy;
  uint16_t spacing;
} DrawObj;

static DrawObj draw_pool[MAX_DRAW_OBJS];

// Terminal State
static uint16_t term_scale = 10;
static int32_t term_line_height = 400;
static int32_t term_char_spacing = 100;
static int8_t term_current_line = 0;
static int32_t term_cursor_x = 0;
static int32_t term_cursor_y = 4096; // Start from top (Y is inverted? No, usually 0 is bottom or top depending on DAC. Let's assume 4096 is top for now, will adjust)
// Actually, in previous code: ty0 = (oy0 * scale) + offset_y.
// If offset_y is 0, it draws at bottom?
// Let's check pattern coordinates. pattern_A: {0,0,2048,4096}. Y goes from 0 to 4096.
// So 0 is bottom, 4096 is top.
// Terminal should start at top (4096) and go down.

static uint8_t term_max_lines = MAX_DRAW_OBJS;

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

// Set pattern by character (A-Z, 0-9, symbols). returns 1 if set, 0 otherwise
static uint8_t set_pattern_by_char(char c){
  uint8_t idx = 255;
  if(c>='A' && c<='Z'){
    idx = (uint8_t)(c - 'A');
  } else if(c>='0' && c<='9'){
    idx = (uint8_t)(c - '0') + 26;
  } else {
    switch(c){
      case '!': idx = 36; break;
      case '\'': idx = 37; break;
      case '#': idx = 38; break;
      case '%': idx = 39; break;
      case '^': idx = 40; break;
      case '*': idx = 41; break;
      case '_': idx = 42; break;
      case '-': idx = 43; break;
      case '+': idx = 44; break;
      case '=': idx = 45; break;
      case '\\': idx = 46; break;
      case '/': idx = 47; break;
      case '(': idx = 48; break;
      case ')': idx = 49; break;
      case '[': idx = 50; break;
      case ']': idx = 51; break;
      case '{': idx = 52; break;
      case '}': idx = 53; break;
      case '"': idx = 54; break;
      case ';': idx = 55; break;
      case ':': idx = 56; break;
      case ',': idx = 57; break;
      case '.': idx = 58; break;
      case '?': idx = 59; break;
      case '@': idx = 60; break;
      case '$': idx = 61; break;
      case '<': idx = 62; break;
      case '>': idx = 63; break;
      case '|': idx = 64; break;
      case '~': idx = 65; break;
    }
  }
  
  if(idx < patterns_count){
    pattern_index = idx;
    current_pattern = patterns[pattern_index];
    current_pattern_length = pattern_lengths[pattern_index];
    if(current_pattern_length == 0) current_pattern_length = 1;
    return 1;
  }
  return 0;
}

void DRAW_Render(void){
    DAC_Buff_Count = 0;
    
    // If no objects, output center point
    int active_found = 0;
    for(int i=0; i<MAX_DRAW_OBJS; i++){
        if(draw_pool[i].active) { active_found = 1; break; }
    }
    
    if(!active_found){
        // Fill with center point
        for(int i=0; i<100; i++){ // minimal buffer
             DAC_Buff_X[i] = 2048;
             DAC_Buff_Y[i] = 2048;
        }
        DAC_Buff_Count = 100;
    } else {
        // Render objects
        for(int i=0; i<MAX_DRAW_OBJS; i++){
            if(!draw_pool[i].active) continue;
            
            // Render string
            int32_t cursor_x = draw_pool[i].x;
            int32_t cursor_y = draw_pool[i].y;
            uint16_t sx = draw_pool[i].sx;
            uint16_t sy = draw_pool[i].sy;
            
            for(int c=0; c<MAX_STR_LEN; c++){
                char ch = draw_pool[i].text[c];
                if(ch == 0) break;
                if(ch == ' '){
                    cursor_x += (2000 * (int32_t)sx) / 100 + draw_pool[i].spacing;
                    continue;
                }
                
                if(set_pattern_by_char(ch)){
                    int32_t minx, maxx;
                    compute_pattern_minmax_x(current_pattern, current_pattern_length, &minx, &maxx);
                    
                    // Draw each line in the pattern
                    for(int l=0; l<current_pattern_length; l++){
                        int32_t x0 = current_pattern[l].x0;
                        int32_t y0 = current_pattern[l].y0;
                        int32_t x1 = current_pattern[l].x1;
                        int32_t y1 = current_pattern[l].y1;
                        
                        // Transform
                        int32_t tx0 = (x0 * (int32_t)sx) / 100 + cursor_x - (minx * (int32_t)sx) / 100;
                        int32_t ty0 = (y0 * (int32_t)sy) / 100 + cursor_y;
                        int32_t tx1 = (x1 * (int32_t)sx) / 100 + cursor_x - (minx * (int32_t)sx) / 100;
                        int32_t ty1 = (y1 * (int32_t)sy) / 100 + cursor_y;
                        
                        // Interpolate line
                        int32_t dx = tx1 - tx0;
                        int32_t dy = ty1 - ty0;
                        // Reduce divisor to increase point count (slower drawing, brighter lines, less visible jumps)
                        // Aggressively reduce points to fit more text. 
                        // Distance / 80 means a 2000 unit line has 25 points.
                        int steps = (int)sqrt((double)dx*dx + (double)dy*dy) / 10; 
                        if(steps < 2) steps = 2; // At least start and end points
                        
                        for(int s=0; s<=steps; s++){
                            if(DAC_Buff_Count >= DRAW_BUF_SIZE) break;
                            DAC_Buff_X[DAC_Buff_Count] = tx0 + (dx * s) / steps;
                            DAC_Buff_Y[DAC_Buff_Count] = ty0 + (dy * s) / steps;
                            
                            // Clip
                            if(DAC_Buff_X[DAC_Buff_Count] > 4095) DAC_Buff_X[DAC_Buff_Count] = 4095;
                            if(DAC_Buff_Y[DAC_Buff_Count] > 4095) DAC_Buff_Y[DAC_Buff_Count] = 4095;
                            
                            DAC_Buff_Count++;
                        }
                    }
                    
                    // Advance cursor
                    cursor_x += ((maxx - minx) * (int32_t)sx) / 100 + draw_pool[i].spacing;
                }
            }
        }
    }
    
    // Update DMA
    if(DAC_Buff_Count > 0){
        HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
        HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
        HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)DAC_Buff_X, DAC_Buff_Count, DAC_ALIGN_12B_R);
        HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*)DAC_Buff_Y, DAC_Buff_Count, DAC_ALIGN_12B_R);
    }
}

void DRAW_Clear(void){
  for(int i=0; i<MAX_DRAW_OBJS; i++) draw_pool[i].active = 0;
  DRAW_Render();
}

void DRAW_Init(uint32_t interval_ms){
  if(patterns_count == 0) return;
  DRAW_Clear();
}

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
  
  // Update buffer immediately
  DRAW_Render();
  
  return 1;
}

void DRAW_SetLetter(char c){
    char buf[2] = {c, 0};
    DRAW_Clear();
    DRAW_AddString(buf, 0, offset_x, offset_y, scale_x_pct, scale_y_pct);
}

void DRAW_Terminal_Init(uint16_t scale_pct){
    if(scale_pct < 1) scale_pct = 1;
    DRAW_Clear();
    term_scale = scale_pct;
    
    term_char_spacing = (500 * (int32_t)scale_pct) / 100; // spacing
    
    // Estimate line height: 4096 (full height) * scale / 100.
    // A char is roughly 4096 units high in pattern space.
    int32_t char_h = (4096 * (int32_t)scale_pct) / 100;
    
    // Add some padding (use char spacing as vertical padding)
    term_line_height = char_h + term_char_spacing; 
    
    // Calculate max visible lines
    // Formula: (4096 + padding) / (height + padding)
    term_max_lines = (4096 + term_char_spacing) / term_line_height;
    
    if(term_max_lines > MAX_DRAW_OBJS) term_max_lines = MAX_DRAW_OBJS;
    if(term_max_lines < 1) term_max_lines = 1;
    
    term_current_line = 0;
    // Start Y at top - line_height (so the first line is visible)
    // Wait, if Y=0 is bottom, then top line is at Y=4096 - height.
    term_cursor_y = 4096 - term_line_height;
    term_cursor_x = 0;
}

void DRAW_Terminal_Print(const char *str){
    // If no active line, start one
    if(term_current_line == 0 && !draw_pool[0].active){
        draw_pool[0].active = 1;
        draw_pool[0].x = 0;
        draw_pool[0].y = term_cursor_y;
        draw_pool[0].sx = term_scale;
        draw_pool[0].sy = term_scale;
        draw_pool[0].spacing = term_char_spacing;
        draw_pool[0].text[0] = '\0';
    }

    int len = strlen(str);
    for(int i=0; i<len; i++){
        char c = str[i];
        
        // Handle Newline
        if(c == '\n'){
            // Move to next line
            term_current_line++;
            if(term_current_line >= term_max_lines){
                // Scroll up
                for(int j=0; j<term_max_lines-1; j++){
                    strcpy(draw_pool[j].text, draw_pool[j+1].text);
                    // Ensure active status is propagated (though usually all are active when scrolling)
                    draw_pool[j].active = draw_pool[j+1].active;
                    // Coordinates are fixed per slot, so we don't copy them.
                    // Slot 0 is always top line, Slot 1 is second line...
                    // But wait, we need to ensure slot 0 is active if slot 1 was active.
                }
                // Clear last line
                draw_pool[term_max_lines-1].text[0] = '\0';
                term_current_line = term_max_lines - 1;
            }
            
            // Setup new line
            draw_pool[term_current_line].active = 1;
            draw_pool[term_current_line].x = 0;
            // Calculate Y for this slot
            draw_pool[term_current_line].y = 4096 - (term_current_line + 1) * term_line_height;
            draw_pool[term_current_line].sx = term_scale;
            draw_pool[term_current_line].sy = term_scale;
            draw_pool[term_current_line].spacing = term_char_spacing;
            draw_pool[term_current_line].text[0] = '\0';
            
            term_cursor_x = 0;
            continue;
        }
        
        // Check width (rough estimation)
        // Char width approx 2000 units unscaled
        int32_t char_w = (2000 * (int32_t)term_scale) / 100 + term_char_spacing;
        if(term_cursor_x + char_w > 4096){
             // Auto wrap
             // Recursive call with newline? Or just duplicate logic
             // Let's just trigger newline logic
             term_current_line++;
             if(term_current_line >= term_max_lines){
                for(int j=0; j<term_max_lines-1; j++){
                    strcpy(draw_pool[j].text, draw_pool[j+1].text);
                    draw_pool[j].active = draw_pool[j+1].active;
                }
                draw_pool[term_max_lines-1].text[0] = '\0';
                term_current_line = term_max_lines - 1;
             }
             draw_pool[term_current_line].active = 1;
             draw_pool[term_current_line].x = 0;
             draw_pool[term_current_line].y = 4096 - (term_current_line + 1) * term_line_height;
             draw_pool[term_current_line].sx = term_scale;
             draw_pool[term_current_line].sy = term_scale;
             draw_pool[term_current_line].spacing = term_char_spacing;
             draw_pool[term_current_line].text[0] = '\0';
             term_cursor_x = 0;
        }
        
        // Append char
        int cur_len = strlen(draw_pool[term_current_line].text);
        if(cur_len < MAX_STR_LEN - 1){
            draw_pool[term_current_line].text[cur_len] = c;
            draw_pool[term_current_line].text[cur_len+1] = '\0';
            term_cursor_x += char_w;
        }
    }
    
    DRAW_Render();
}

// End of file
