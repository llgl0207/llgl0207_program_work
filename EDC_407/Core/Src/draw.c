#include "draw.h"
#include "dac.h"
#include <math.h>
#include <string.h>

// DMA Buffers
uint16_t DAC_Buff_X[DRAW_BUF_SIZE];
uint16_t DAC_Buff_Y[DRAW_BUF_SIZE];
uint32_t DAC_Buff_Count = 0;

typedef struct { int16_t x0,y0,x1,y1; } Line_t;

static uint8_t set_pattern_by_char(char c);
static void compute_pattern_minmax_x(const Line_t *p, uint8_t len, int32_t *minx, int32_t *maxx);

// For brevity, include a compact set of patterns (A..Z) copied from main.c
// In a real library we'd store these more compactly or generate them.
static const Line_t pattern_A[] = { {2, 1, 4 ,7},{4,7,6,1},{3,4,5,4}};
static const Line_t pattern_a[] = { {3, 5, 5, 5},{5, 5, 6 , 4 } ,{ 6 , 4,  6, 1},{ 6, 1, 3 ,1},{3,1,2,2},{2,2,3,3},{3,3,6,3}};
static const Line_t pattern_B[] = { {2,1,2,7},{2,7,4,7},{4,7,5,6},{5,6,5,5},{5,5,4,4},{4,4,2,4},{4,4,5,3},{5,3,5,2},{5,2,4,1},{4,1,2,1}};
static const Line_t pattern_b[] = { {2,7,2,1},{2,1,4,1},{4,1,5,2},{5,2,5,3},{5,3,4,4},{4,4,2,4}};
static const Line_t pattern_C[] = { {6,6,5,7},{5,7,3,7},{3,7,2,6},{2,6,2,2},{2,2,3,1},{3,1,5,1},{5,1,6,2}};
static const Line_t pattern_c[] = { {5,3,4,4},{4,4,3,4},{3,4,2,3},{2,3,2,2},{2,2,3,1},{3,1,4,1},{4,1,5,2}};
static const Line_t pattern_D[] = { {2,1,2,7},{2,7,4,7},{4,7,6,5},{6,5,6,3},{6,3,4,1},{4,1,2,1}};
static const Line_t pattern_d[] = { {5,7,5,1},{5,1,3,1},{3,1,2,2},{2,2,2,3},{2,3,3,4},{3,4,5,4}};
static const Line_t pattern_E[] = { {6,7,2,7},{2,7,2,1},{2,1,6,1},{2,4,5,4}};
static const Line_t pattern_e[] = { {6,1,3,1},{3,1,2,2},{2,2,2,4},{2,4,3,5},{3,5,4,5},{4,5,5,4},{5,4,4,3},{4,3,2,3}};
static const Line_t pattern_F[] = { {2,1,2,7},{2,7,6,7},{2,4,5,4}};
static const Line_t pattern_f[] = { {5,6,4.5,6},{4.5,6,4,5.5},{4,5.5,4,4},{3,4,5,4},{4,4,4,1}};
static const Line_t pattern_G[] = { {6,6,5,7},{5,7,3,7},{3,7,2,6},{2,6,2,2},{2,2,3,1},{3,1,5,1},{5,1,6,2},{6,2,6,4},{6,4,4,4}};
static const Line_t pattern_g[] = { {2,-1,4,-1},{4,-1,5,0},{5,0,5,3},{5,3,4,4},{4,4,3,4},{3,4,2,3},{2,3,2,2},{2,2,3,1},{3,1,4,1},{4,1,5,2}};
static const Line_t pattern_H[] = { {2,1,2,7},{6,1,6,7},{2,4,6,4}};
static const Line_t pattern_h[] = { {3,1,3,7},{3,1,3,3},{3,3,4,4},{4,4,5,4},{5,4,6,3},{6,3,6,1}};
static const Line_t pattern_I[] = { {3,1,5,1},{4,1,4,7},{3,7,5,7}};
static const Line_t pattern_i[] = { {4,6,4,5},{3,4,4,4},{4,4,4,1},{3.5,1,4.5,1}};
static const Line_t pattern_J[] = { {5.5,7,6.5,7},{6,6,6,2},{6,2,5,1},{5,1,4,1}};
static const Line_t pattern_j[] = { {5,7,5,6},{5,3,5,0},{5,0,4,-1}};
static const Line_t pattern_K[] = { {2,7,2,1},{2,4,5,1},{2,4,5,7}};
static const Line_t pattern_k[] = { {3,1,3,6},{3,3,5,4},{3,3,5,1}};
static const Line_t pattern_L[] = { {2,7,2,1},{2,1,6,1}};
static const Line_t pattern_l[] = { {3.5,6.5,4,7},{4,7,4,1},{4,1,4.5,1.5}};
static const Line_t pattern_M[] = { {2,1,2,7},{2,7,4,4},{4,4,6,7},{6,7,6,1}};
static const Line_t pattern_m[] = { {2,1,2,4},{2,4,3,5},{3,5,4,4},{4,4,4,1},{4,1,4,4},{4,4,5,5},{5,5,6,4},{6,4,6,1}};
static const Line_t pattern_N[] = { {2,1,2,7},{2,7,6,1},{6,1,6,7}};
static const Line_t pattern_n[] = { {2,1,2,5},{2,4,3,5},{3,5,4,5},{4,5,5,4},{5,4,5,1}};
static const Line_t pattern_O[] = { {3,1,5,1},{5,1,6,2},{6,2,6,6},{6,6,5,7},{5,7,3,7},{3,7,2,6},{2,6,2,2},{2,2,3,1}};
static const Line_t pattern_o[] = { {3,1,4,1},{4,1,5,2},{5,2,5,4},{5,4,4,5},{4,5,3,5},{3,5,2,4},{2,4,2,2},{2,2,3,1}};
static const Line_t pattern_P[] = { {2,1,2,7},{2,7,5,7},{5,7,6,6},{6,6,6,5},{6,5,5,4},{5,4,2,4}};
static const Line_t pattern_p[] = { {2,-1,2,4},{2,4,4,4},{4,4,5,3},{5,3,4,2},{4,2,2,1}};
static const Line_t pattern_Q[] = { {3,1,5,1},{5,1,6,2},{6,2,6,6},{6,6,5,7},{5,7,3,7},{3,7,2,6},{2,6,2,2},{2,2,3,1},{5,3,7,1}};
static const Line_t pattern_q[] = { {5,-1,5,4},{5,4,3,4},{3,4,2,3},{2,3,2,2},{2,2,3,1},{3,1,5,1}};
static const Line_t pattern_R[] = { {2,1,2,7},{2,7,5,7},{5,7,6,6},{6,6,6,5},{6,5,5,4},{5,4,2,4},{4,4,7,1}};
static const Line_t pattern_r[] = { {2.5,4,3,4},{3,4,3,1},{2.5,1,3.5,1},{3,3,4,4},{4,4,5,4},{5,4,6,3}};
static const Line_t pattern_S[] = { {6,6,5,7},{5,7,3,7},{3,7,2,6},{2,6,2,5},{2,5,3,4},{3,4,5,4},{5,4,6,3},{6,3,6,2},{6,2,5,1},{5,1,3,1},{3,1,2,2}};
static const Line_t pattern_s[] = { {3.75,3.00,3.00,3.75},{3.00,3.75,2.25,3.75},{2.25,3.75,1.50,3.00},{1.50,3.00,2.25,2.25},{2.25,2.25,3.00,2.25},{3.00,2.25,3.75,1.50},{3.75,1.50,3.00,0.75},{3.00,0.75,2.25,0.75},{2.25,0.75,1.50,1.50}};
static const Line_t pattern_T[] = { {2,7,6,7},{4,7,4,1}};
static const Line_t pattern_t[] = { {2,4,4,4},{3,5,3,1.5},{3,1.5,3.5,1},{3.5,1,4,1}};
static const Line_t pattern_U[] = { {2,7,2,2},{2,2,3,1},{3,1,5,1},{5,1,6,2},{6,2,6,7}};
static const Line_t pattern_u[] = { {2,4,2,2},{2,2,3,1},{3,1,4,1},{4,1,5,2},{5,4,5,1}};
static const Line_t pattern_V[] = { {2,7,4,1},{4,1,6,7}};
static const Line_t pattern_v[] = { {2,4,3.5,1},{3.5,1,5,4}};
static const Line_t pattern_W[] = { {2,7,2,1},{2,1,4,4},{4,4,6,1},{6,1,6,7}};
static const Line_t pattern_w[] = { {2,4,2,2},{2,2,3,1},{3,1,4,2},{4,2,4,4},{4,4,4,2},{4,2,5,1},{5,1,6,2},{6,2,6,4}};
static const Line_t pattern_X[] = { {2,7,6,1},{6,7,2,1}};
static const Line_t pattern_x[] = { {2,4,4,1},{4,4,2,1}};
static const Line_t pattern_Y[] = { {2,7,4,4},{6,7,4,4},{4,4,4,1}};
static const Line_t pattern_y[] = { {2,-1,5,4},{3.5,1.5,2,4}};
static const Line_t pattern_Z[] = { {2,7,6,7},{6,7,2,1},{2,1,6,1}};
static const Line_t pattern_z[] = { {2,4,4,4},{4,4,2,1},{2,1,4,1}};



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
  pattern_at, pattern_dollar, pattern_lt, pattern_gt, pattern_pipe, pattern_tilde,
  pattern_a, pattern_b, pattern_c, pattern_d, pattern_e, pattern_f, pattern_g,
  pattern_h, pattern_i, pattern_j, pattern_k, pattern_l, pattern_m, pattern_n,
  pattern_o, pattern_p, pattern_q, pattern_r, pattern_s, pattern_t, pattern_u,
  pattern_v, pattern_w, pattern_x, pattern_y, pattern_z
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
  sizeof(pattern_gt)/sizeof(pattern_gt[0]), sizeof(pattern_pipe)/sizeof(pattern_pipe[0]), sizeof(pattern_tilde)/sizeof(pattern_tilde[0]),
  sizeof(pattern_a)/sizeof(pattern_a[0]), sizeof(pattern_b)/sizeof(pattern_b[0]), sizeof(pattern_c)/sizeof(pattern_c[0]),
  sizeof(pattern_d)/sizeof(pattern_d[0]), sizeof(pattern_e)/sizeof(pattern_e[0]), sizeof(pattern_f)/sizeof(pattern_f[0]),
  sizeof(pattern_g)/sizeof(pattern_g[0]), sizeof(pattern_h)/sizeof(pattern_h[0]), sizeof(pattern_i)/sizeof(pattern_i[0]),
  sizeof(pattern_j)/sizeof(pattern_j[0]), sizeof(pattern_k)/sizeof(pattern_k[0]), sizeof(pattern_l)/sizeof(pattern_l[0]),
  sizeof(pattern_m)/sizeof(pattern_m[0]), sizeof(pattern_n)/sizeof(pattern_n[0]), sizeof(pattern_o)/sizeof(pattern_o[0]),
  sizeof(pattern_p)/sizeof(pattern_p[0]), sizeof(pattern_q)/sizeof(pattern_q[0]), sizeof(pattern_r)/sizeof(pattern_r[0]),
  sizeof(pattern_s)/sizeof(pattern_s[0]), sizeof(pattern_t)/sizeof(pattern_t[0]), sizeof(pattern_u)/sizeof(pattern_u[0]),
  sizeof(pattern_v)/sizeof(pattern_v[0]), sizeof(pattern_w)/sizeof(pattern_w[0]), sizeof(pattern_x)/sizeof(pattern_x[0]),
  sizeof(pattern_y)/sizeof(pattern_y[0]), sizeof(pattern_z)/sizeof(pattern_z[0])
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
#define MAX_DRAW_OBJS 32
#define MAX_STR_LEN 64

typedef struct {
  uint8_t active;
  DrawType type;
  union {
      struct {
          char text[MAX_STR_LEN];
          int32_t x, y;
          uint16_t sx, sy;
          uint16_t spacing;
          int32_t scroll_offset;
          uint32_t last_scroll_time;
          int32_t total_width;
          int32_t view_width;
      } text_data;
      struct {
          int32_t x0, y0, x1, y1;
      } line_data;
      struct {
          int32_t x, y, w, h;
      } rect_data;
      struct {
          int32_t x, y, r;
      } circle_data;
  } data;
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
  } else if(c>='a' && c<='z'){
    idx = (uint8_t)(c - 'a') + 66;
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

void DRAW_Update(void){
    uint32_t now = HAL_GetTick();
    uint8_t need_render = 0;
    
    for(int i=0; i<MAX_DRAW_OBJS; i++){
        if(draw_pool[i].active && draw_pool[i].type == DRAW_TYPE_TEXT){
            // Check if scrolling is needed
            if(draw_pool[i].data.text_data.total_width > draw_pool[i].data.text_data.view_width){
                if(now - draw_pool[i].data.text_data.last_scroll_time > 20){ // 20ms update rate
                    draw_pool[i].data.text_data.last_scroll_time = now;
                    draw_pool[i].data.text_data.scroll_offset += 50; // Scroll speed
                    
                    // Wrap around
                    if(draw_pool[i].data.text_data.scroll_offset > draw_pool[i].data.text_data.total_width + 500){
                        draw_pool[i].data.text_data.scroll_offset = -draw_pool[i].data.text_data.view_width;
                    }
                    need_render = 1;
                }
            }
        }
    }
    
    if(need_render){
        DRAW_Render();
    }
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
            
            if(draw_pool[i].type == DRAW_TYPE_TEXT)
            {
                // Render string
                int32_t cursor_x = draw_pool[i].data.text_data.x;
                int32_t cursor_y = draw_pool[i].data.text_data.y;
                uint16_t sx = draw_pool[i].data.text_data.sx;
                uint16_t sy = draw_pool[i].data.text_data.sy;
                int32_t scroll = draw_pool[i].data.text_data.scroll_offset;
                
                for(int c=0; c<MAX_STR_LEN; c++){
                    char ch = draw_pool[i].data.text_data.text[c];
                    if(ch == 0) break;
                    if(ch == ' '){
                        cursor_x += (2000 * (int32_t)sx) / 100 + draw_pool[i].data.text_data.spacing;
                        continue;
                    }
                    
                    if(set_pattern_by_char(ch)){
                        int32_t minx, maxx;
                        compute_pattern_minmax_x(current_pattern, current_pattern_length, &minx, &maxx);
                        
                        // Scale up for letters (Unit Length Mode -> DAC Mode)
                        int32_t pre_scale = 1;
                        if((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')){
                            pre_scale = 512;
                        }
                        
                        minx *= pre_scale;
                        maxx *= pre_scale;
                        
                        int32_t char_w = ((maxx - minx) * (int32_t)sx) / 100;
                        int32_t draw_x = cursor_x - scroll;
                        
                        // Visibility Check
                        if(draw_x + char_w < 0) {
                             cursor_x += char_w + draw_pool[i].data.text_data.spacing;
                             continue; 
                        }
                        if(draw_x > 4096) {
                             break; 
                        }

                        // Draw each line in the pattern
                        for(int l=0; l<current_pattern_length; l++){
                            int32_t x0 = current_pattern[l].x0 * pre_scale;
                            int32_t y0 = current_pattern[l].y0 * pre_scale;
                            int32_t x1 = current_pattern[l].x1 * pre_scale;
                            int32_t y1 = current_pattern[l].y1 * pre_scale;
                            
                            // Transform
                            int32_t tx0 = (x0 * (int32_t)sx) / 100 + cursor_x - (minx * (int32_t)sx) / 100 - scroll;
                            int32_t ty0 = (y0 * (int32_t)sy) / 100 + cursor_y;
                            int32_t tx1 = (x1 * (int32_t)sx) / 100 + cursor_x - (minx * (int32_t)sx) / 100 - scroll;
                            int32_t ty1 = (y1 * (int32_t)sy) / 100 + cursor_y;
                            
                            // Interpolate line
                            int32_t dx = tx1 - tx0;
                            int32_t dy = ty1 - ty0;
                            
                            // --- DRAWING SPEED / DENSITY CONTROL ---
                            int steps = (int)sqrt((double)dx*dx + (double)dy*dy) / 5; 
                            
                            if(steps < 2) steps = 2; // At least start and end points
                            
                            for(int s=0; s<=steps; s++){
                                if(DAC_Buff_Count >= DRAW_BUF_SIZE) break;
                                int32_t px = tx0 + (dx * s) / steps;
                                int32_t py = ty0 + (dy * s) / steps;
                                
                                // Clamp
                                if(px < 0) px = 0;
                                if(px > 4095) px = 4095;
                                if(py < 0) py = 0;
                                if(py > 4095) py = 4095;
                                
                                DAC_Buff_X[DAC_Buff_Count] = (uint16_t)px;
                                DAC_Buff_Y[DAC_Buff_Count] = (uint16_t)py;
                                
                                DAC_Buff_Count++;
                            }
                        }
                        
                        // Advance cursor
                        cursor_x += char_w + draw_pool[i].data.text_data.spacing;
                    }
                }
            }
            else if(draw_pool[i].type == DRAW_TYPE_LINE)
            {
                int32_t x0 = draw_pool[i].data.line_data.x0;
                int32_t y0 = draw_pool[i].data.line_data.y0;
                int32_t x1 = draw_pool[i].data.line_data.x1;
                int32_t y1 = draw_pool[i].data.line_data.y1;
                
                int32_t dx = x1 - x0;
                int32_t dy = y1 - y0;
                int steps = (int)sqrt((double)dx*dx + (double)dy*dy) / 5;
                if(steps < 2) steps = 2;
                
                for(int s=0; s<=steps; s++){
                    if(DAC_Buff_Count >= DRAW_BUF_SIZE) break;
                    DAC_Buff_X[DAC_Buff_Count] = x0 + (dx * s) / steps;
                    DAC_Buff_Y[DAC_Buff_Count] = y0 + (dy * s) / steps;
                    if(DAC_Buff_X[DAC_Buff_Count] > 4095) DAC_Buff_X[DAC_Buff_Count] = 4095;
                    if(DAC_Buff_Y[DAC_Buff_Count] > 4095) DAC_Buff_Y[DAC_Buff_Count] = 4095;
                    DAC_Buff_Count++;
                }
            }
            else if(draw_pool[i].type == DRAW_TYPE_RECT)
            {
                int32_t x = draw_pool[i].data.rect_data.x;
                int32_t y = draw_pool[i].data.rect_data.y;
                int32_t w = draw_pool[i].data.rect_data.w;
                int32_t h = draw_pool[i].data.rect_data.h;
                
                // 4 Lines
                int32_t pts[5][2] = { {x,y}, {x+w,y}, {x+w,y+h}, {x,y+h}, {x,y} };
                
                for(int l=0; l<4; l++){
                    int32_t x0 = pts[l][0];
                    int32_t y0 = pts[l][1];
                    int32_t x1 = pts[l+1][0];
                    int32_t y1 = pts[l+1][1];
                    
                    int32_t dx = x1 - x0;
                    int32_t dy = y1 - y0;
                    int steps = (int)sqrt((double)dx*dx + (double)dy*dy) / 5;
                    if(steps < 2) steps = 2;
                    
                    for(int s=0; s<=steps; s++){
                        if(DAC_Buff_Count >= DRAW_BUF_SIZE) break;
                        DAC_Buff_X[DAC_Buff_Count] = x0 + (dx * s) / steps;
                        DAC_Buff_Y[DAC_Buff_Count] = y0 + (dy * s) / steps;
                        if(DAC_Buff_X[DAC_Buff_Count] > 4095) DAC_Buff_X[DAC_Buff_Count] = 4095;
                        if(DAC_Buff_Y[DAC_Buff_Count] > 4095) DAC_Buff_Y[DAC_Buff_Count] = 4095;
                        DAC_Buff_Count++;
                    }
                }
            }
            else if(draw_pool[i].type == DRAW_TYPE_CIRCLE)
            {
                int32_t cx = draw_pool[i].data.circle_data.x;
                int32_t cy = draw_pool[i].data.circle_data.y;
                int32_t r = draw_pool[i].data.circle_data.r;
                
                // Circumference approx 2*pi*r
                int steps = (int)(6.28 * r) / 5;
                if(steps < 10) steps = 10;
                
                for(int s=0; s<=steps; s++){
                    if(DAC_Buff_Count >= DRAW_BUF_SIZE) break;
                    double angle = (double)s / steps * 6.283185307;
                    DAC_Buff_X[DAC_Buff_Count] = cx + (int32_t)(cos(angle) * r);
                    DAC_Buff_Y[DAC_Buff_Count] = cy + (int32_t)(sin(angle) * r);
                    if(DAC_Buff_X[DAC_Buff_Count] > 4095) DAC_Buff_X[DAC_Buff_Count] = 4095;
                    if(DAC_Buff_Y[DAC_Buff_Count] > 4095) DAC_Buff_Y[DAC_Buff_Count] = 4095;
                    DAC_Buff_Count++;
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

uint8_t set_pattern_by_char(char c); // Forward declaration fix if needed, but it's static

int16_t DRAW_AddString(const char *s, uint16_t spacing, int32_t x, int32_t y, uint16_t sx, uint16_t sy){
  int slot = -1;
  for(int i=0; i<MAX_DRAW_OBJS; i++){
    if(!draw_pool[i].active){ slot = i; break; }
  }
  if(slot < 0) return -1;

  draw_pool[slot].type = DRAW_TYPE_TEXT;
  strncpy(draw_pool[slot].data.text_data.text, s, MAX_STR_LEN-1);
  draw_pool[slot].data.text_data.text[MAX_STR_LEN-1] = '\0';
  draw_pool[slot].data.text_data.x = x;
  draw_pool[slot].data.text_data.y = y;
  draw_pool[slot].data.text_data.sx = sx;
  draw_pool[slot].data.text_data.sy = sy;
  draw_pool[slot].data.text_data.spacing = spacing;
  
  // Initialize scrolling
  draw_pool[slot].data.text_data.scroll_offset = 0;
  draw_pool[slot].data.text_data.last_scroll_time = HAL_GetTick();
  draw_pool[slot].data.text_data.view_width = 4096; // Default to full screen width
  
  // Calculate total width
  int32_t width = 0;
  for(int c=0; c<MAX_STR_LEN; c++){
      char ch = draw_pool[slot].data.text_data.text[c];
      if(ch == 0) break;
      if(ch == ' '){
          width += (2000 * (int32_t)sx) / 100 + spacing;
          continue;
      }
      if(set_pattern_by_char(ch)){
          int32_t minx, maxx;
          compute_pattern_minmax_x(current_pattern, current_pattern_length, &minx, &maxx);
          int32_t pre_scale = 1;
          if((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')){
              pre_scale = 512;
          }
          minx *= pre_scale;
          maxx *= pre_scale;
          width += ((maxx - minx) * (int32_t)sx) / 100 + spacing;
      }
  }
  draw_pool[slot].data.text_data.total_width = width;

  draw_pool[slot].active = 1;
  
  // Update buffer immediately
  DRAW_Render();
  
  return (int16_t)slot;
}

int32_t DRAW_GetTextScroll(const char *text) {
    for(int i=0; i<MAX_DRAW_OBJS; i++){
        if(draw_pool[i].active && draw_pool[i].type == DRAW_TYPE_TEXT){
            if(strncmp(draw_pool[i].data.text_data.text, text, MAX_STR_LEN) == 0){
                return draw_pool[i].data.text_data.scroll_offset;
            }
        }
    }
    return 0;
}

void DRAW_SetTextScroll(int16_t slot, int32_t scroll) {
    if(slot >= 0 && slot < MAX_DRAW_OBJS && draw_pool[slot].active && draw_pool[slot].type == DRAW_TYPE_TEXT){
        draw_pool[slot].data.text_data.scroll_offset = scroll;
    }
}

uint8_t DRAW_AddLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1){
  int slot = -1;
  for(int i=0; i<MAX_DRAW_OBJS; i++){
    if(!draw_pool[i].active){ slot = i; break; }
  }
  if(slot < 0) return 0;

  draw_pool[slot].type = DRAW_TYPE_LINE;
  draw_pool[slot].data.line_data.x0 = x0;
  draw_pool[slot].data.line_data.y0 = y0;
  draw_pool[slot].data.line_data.x1 = x1;
  draw_pool[slot].data.line_data.y1 = y1;
  draw_pool[slot].active = 1;
  
  DRAW_Render();
  return 1;
}

uint8_t DRAW_AddRect(int32_t x, int32_t y, int32_t w, int32_t h){
  int slot = -1;
  for(int i=0; i<MAX_DRAW_OBJS; i++){
    if(!draw_pool[i].active){ slot = i; break; }
  }
  if(slot < 0) return 0;

  draw_pool[slot].type = DRAW_TYPE_RECT;
  draw_pool[slot].data.rect_data.x = x;
  draw_pool[slot].data.rect_data.y = y;
  draw_pool[slot].data.rect_data.w = w;
  draw_pool[slot].data.rect_data.h = h;
  draw_pool[slot].active = 1;
  
  DRAW_Render();
  return 1;
}

uint8_t DRAW_AddCircle(int32_t x, int32_t y, int32_t r){
  int slot = -1;
  for(int i=0; i<MAX_DRAW_OBJS; i++){
    if(!draw_pool[i].active){ slot = i; break; }
  }
  if(slot < 0) return 0;

  draw_pool[slot].type = DRAW_TYPE_CIRCLE;
  draw_pool[slot].data.circle_data.x = x;
  draw_pool[slot].data.circle_data.y = y;
  draw_pool[slot].data.circle_data.r = r;
  draw_pool[slot].active = 1;
  
  DRAW_Render();
  return 1;
}

void DRAW_SetLetter(char c){
    char buf[2] = {c, 0};
    DRAW_Clear();
    DRAW_AddString(buf, 0, offset_x, offset_y, scale_x_pct, scale_y_pct);
}

void DRAW_Terminal_Init(uint16_t scale_pct, int32_t spacing){
    if(scale_pct < 1) scale_pct = 1;
    DRAW_Clear();
    term_scale = scale_pct;
    
    term_char_spacing = spacing;
    
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

void DRAW_Terminal_SetSpacing(int32_t spacing){
    term_char_spacing = spacing;
}

void DRAW_Terminal_Print(const char *str){
    // If no active line, start one
    if(term_current_line == 0 && !draw_pool[0].active){
        draw_pool[0].active = 1;
        draw_pool[0].type = DRAW_TYPE_TEXT;
        draw_pool[0].data.text_data.x = 0;
        draw_pool[0].data.text_data.y = term_cursor_y;
        draw_pool[0].data.text_data.sx = term_scale;
        draw_pool[0].data.text_data.sy = term_scale;
        draw_pool[0].data.text_data.spacing = term_char_spacing;
        draw_pool[0].data.text_data.text[0] = '\0';
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
                    // Only copy text data if it's text type
                    if(draw_pool[j+1].type == DRAW_TYPE_TEXT){
                        draw_pool[j].type = DRAW_TYPE_TEXT;
                        strcpy(draw_pool[j].data.text_data.text, draw_pool[j+1].data.text_data.text);
                        draw_pool[j].active = draw_pool[j+1].active;
                    }
                }
                // Clear last line
                draw_pool[term_max_lines-1].data.text_data.text[0] = '\0';
                term_current_line = term_max_lines - 1;
            }
            
            // Setup new line
            draw_pool[term_current_line].active = 1;
            draw_pool[term_current_line].type = DRAW_TYPE_TEXT;
            draw_pool[term_current_line].data.text_data.x = 0;
            // Calculate Y for this slot
            draw_pool[term_current_line].data.text_data.y = 4096 - (term_current_line + 1) * term_line_height;
            draw_pool[term_current_line].data.text_data.sx = term_scale;
            draw_pool[term_current_line].data.text_data.sy = term_scale;
            draw_pool[term_current_line].data.text_data.spacing = term_char_spacing;
            draw_pool[term_current_line].data.text_data.text[0] = '\0';
            
            term_cursor_x = 0;
            continue;
        }
        
        // Check width (rough estimation)
        // Char width approx 2000 units unscaled
        int32_t char_w = (2000 * (int32_t)term_scale) / 100 + term_char_spacing;
        if(term_cursor_x + char_w > 4096){
             // Auto wrap
             term_current_line++;
             if(term_current_line >= term_max_lines){
                for(int j=0; j<term_max_lines-1; j++){
                    if(draw_pool[j+1].type == DRAW_TYPE_TEXT){
                        draw_pool[j].type = DRAW_TYPE_TEXT;
                        strcpy(draw_pool[j].data.text_data.text, draw_pool[j+1].data.text_data.text);
                        draw_pool[j].active = draw_pool[j+1].active;
                    }
                }
                draw_pool[term_max_lines-1].data.text_data.text[0] = '\0';
                term_current_line = term_max_lines - 1;
             }
             draw_pool[term_current_line].active = 1;
             draw_pool[term_current_line].type = DRAW_TYPE_TEXT;
             draw_pool[term_current_line].data.text_data.x = 0;
             draw_pool[term_current_line].data.text_data.y = 4096 - (term_current_line + 1) * term_line_height;
             draw_pool[term_current_line].data.text_data.sx = term_scale;
             draw_pool[term_current_line].data.text_data.sy = term_scale;
             draw_pool[term_current_line].data.text_data.spacing = term_char_spacing;
             draw_pool[term_current_line].data.text_data.text[0] = '\0';
             term_cursor_x = 0;
        }
        
        // Append char
        int cur_len = strlen(draw_pool[term_current_line].data.text_data.text);
        if(cur_len < MAX_STR_LEN - 1){
            draw_pool[term_current_line].data.text_data.text[cur_len] = c;
            draw_pool[term_current_line].data.text_data.text[cur_len+1] = '\0';
            term_cursor_x += char_w;
        }
    }
    
    DRAW_Render();
}

// End of file
