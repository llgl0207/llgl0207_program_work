#include "draw.h"
#include "dac.h"
#include "tim.h"
#include <math.h>
#include <string.h>
#include <stdint.h>

// --- Resolution Control ---
// Define the virtual canvas resolution bits (e.g., 12 for 4096, 13 for 8192)
// Only powers of 2 are allowed to enable efficient bit-shifting
#define CANVAS_BITS 10 
#define DAC_BITS    12

// Canvas Dimensions (Square)
#define CANVAS_SIZE  (1 << CANVAS_BITS)

// Coordinate Type Definition based on Canvas Size
#if CANVAS_BITS < 8
    typedef int8_t Coord_t;
#elif CANVAS_BITS < 16
    typedef int16_t Coord_t;
#else
    typedef int32_t Coord_t;
#endif

// Coordinate Conversion Macros
#if CANVAS_BITS > DAC_BITS
  #define MAP_TO_DAC(v) ((int32_t)(v) >> (CANVAS_BITS - DAC_BITS))
#elif CANVAS_BITS < DAC_BITS
  #define MAP_TO_DAC(v) ((int32_t)(v) << (DAC_BITS - CANVAS_BITS))
#else
  #define MAP_TO_DAC(v) (v)
#endif

// DMA 缓冲区
// 用于存储 DAC 输出的 X 和 Y 坐标数据
uint16_t DAC_Buff_X[DRAW_BUF_SIZE];
uint16_t DAC_Buff_Y[DRAW_BUF_SIZE];
uint32_t DAC_Buff_Count = 0;

// 线段结构体，定义矢量字体的笔画
typedef struct { int8_t x0,y0,x1,y1; } Line_t;

// 内部函数声明
static uint8_t set_pattern_by_char(char c);
static void compute_pattern_minmax_x(const Line_t *p, uint8_t len, int8_t *minx, int8_t *maxx);

// 矢量字体数据定义 (A-Z, a-z, 0-9, 符号)
// 每个字符由一系列线段组成，坐标范围通常在 0-16 之间 (原 0-8 放大 2 倍)
static const Line_t pattern_A[] = { {4, 2, 8, 14}, {8, 14, 12, 2}, {6, 8, 10, 8} };
static const Line_t pattern_a[] = { {6, 10, 10, 10}, {10, 10, 12, 8}, {12, 8, 12, 2}, {12, 2, 6, 2}, {6, 2, 4, 4}, {4, 4, 6, 6}, {6, 6, 12, 6} };
static const Line_t pattern_B[] = { {4, 2, 4, 14}, {4, 14, 8, 14}, {8, 14, 10, 12}, {10, 12, 10, 10}, {10, 10, 8, 8}, {8, 8, 4, 8}, {8, 8, 10, 6}, {10, 6, 10, 4}, {10, 4, 8, 2}, {8, 2, 4, 2} };
static const Line_t pattern_b[] = { {4, 14, 4, 2}, {4, 2, 8, 2}, {8, 2, 10, 4}, {10, 4, 10, 6}, {10, 6, 8, 8}, {8, 8, 4, 8} };
static const Line_t pattern_C[] = { {12, 12, 10, 14}, {10, 14, 6, 14}, {6, 14, 4, 12}, {4, 12, 4, 4}, {4, 4, 6, 2}, {6, 2, 10, 2}, {10, 2, 12, 4} };
static const Line_t pattern_c[] = { {10, 6, 8, 8}, {8, 8, 6, 8}, {6, 8, 4, 6}, {4, 6, 4, 4}, {4, 4, 6, 2}, {6, 2, 8, 2}, {8, 2, 10, 4} };
static const Line_t pattern_D[] = { {4, 2, 4, 14}, {4, 14, 8, 14}, {8, 14, 12, 10}, {12, 10, 12, 6}, {12, 6, 8, 2}, {8, 2, 4, 2} };
static const Line_t pattern_d[] = { {10, 14, 10, 2}, {10, 2, 6, 2}, {6, 2, 4, 4}, {4, 4, 4, 6}, {4, 6, 6, 8}, {6, 8, 10, 8} };
static const Line_t pattern_E[] = { {12, 14, 4, 14}, {4, 14, 4, 2}, {4, 2, 12, 2}, {4, 8, 10, 8} };
static const Line_t pattern_e[] = { {12, 2, 6, 2}, {6, 2, 4, 4}, {4, 4, 4, 8}, {4, 8, 6, 10}, {6, 10, 8, 10}, {8, 10, 10, 8}, {10, 8, 8, 6}, {8, 6, 4, 6} };
static const Line_t pattern_F[] = { {4, 2, 4, 14}, {4, 14, 12, 14}, {4, 8, 10, 8} };
static const Line_t pattern_f[] = { {10, 12, 9, 12}, {9, 12, 8, 11}, {8, 11, 8, 8}, {6, 8, 10, 8}, {8, 8, 8, 2} };
static const Line_t pattern_G[] = { {12, 12, 10, 14}, {10, 14, 6, 14}, {6, 14, 4, 12}, {4, 12, 4, 4}, {4, 4, 6, 2}, {6, 2, 10, 2}, {10, 2, 12, 4}, {12, 4, 12, 8}, {12, 8, 8, 8} };
static const Line_t pattern_g[] = { {4, -2, 8, -2}, {8, -2, 10, 0}, {10, 0, 10, 6}, {10, 6, 8, 8}, {8, 8, 6, 8}, {6, 8, 4, 6}, {4, 6, 4, 4}, {4, 4, 6, 2}, {6, 2, 8, 2}, {8, 2, 10, 4} };
static const Line_t pattern_H[] = { {4, 2, 4, 14}, {12, 2, 12, 14}, {4, 8, 12, 8} };
static const Line_t pattern_h[] = { {6, 2, 6, 14}, {6, 2, 6, 6}, {6, 6, 8, 8}, {8, 8, 10, 8}, {10, 8, 12, 6}, {12, 6, 12, 2} };
static const Line_t pattern_I[] = { {6, 2, 10, 2}, {8, 2, 8, 14}, {6, 14, 10, 14} };
static const Line_t pattern_i[] = { {8, 12, 8, 10}, {6, 8, 8, 8}, {8, 8, 8, 2}, {7, 2, 9, 2} };
static const Line_t pattern_J[] = { {11, 14, 13, 14}, {12, 12, 12, 4}, {12, 4, 10, 2}, {10, 2, 8, 2} };
static const Line_t pattern_j[] = { {10, 14, 10, 12}, {10, 6, 10, 0}, {10, 0, 8, -2} };
static const Line_t pattern_K[] = { {4, 14, 4, 2}, {4, 8, 10, 2}, {4, 8, 10, 14} };
static const Line_t pattern_k[] = { {6, 2, 6, 12}, {6, 6, 10, 8}, {6, 6, 10, 2} };
static const Line_t pattern_L[] = { {4, 14, 4, 2}, {4, 2, 12, 2} };
static const Line_t pattern_l[] = { {7, 13, 8, 14}, {8, 14, 8, 2}, {8, 2, 9, 3} };
static const Line_t pattern_M[] = { {4, 2, 4, 14}, {4, 14, 8, 8}, {8, 8, 12, 14}, {12, 14, 12, 2} };
static const Line_t pattern_m[] = { {4, 2, 4, 8}, {4, 8, 6, 10}, {6, 10, 8, 8}, {8, 8, 8, 2}, {8, 2, 8, 8}, {8, 8, 10, 10}, {10, 10, 12, 8}, {12, 8, 12, 2} };
static const Line_t pattern_N[] = { {4, 2, 4, 14}, {4, 14, 12, 2}, {12, 2, 12, 14} };
static const Line_t pattern_n[] = { {4, 2, 4, 10}, {4, 8, 6, 10}, {6, 10, 8, 10}, {8, 10, 10, 8}, {10, 8, 10, 2} };
static const Line_t pattern_O[] = { {6, 2, 10, 2}, {10, 2, 12, 4}, {12, 4, 12, 12}, {12, 12, 10, 14}, {10, 14, 6, 14}, {6, 14, 4, 12}, {4, 12, 4, 4}, {4, 4, 6, 2} };
static const Line_t pattern_o[] = { {6, 2, 8, 2}, {8, 2, 10, 4}, {10, 4, 10, 8}, {10, 8, 8, 10}, {8, 10, 6, 10}, {6, 10, 4, 8}, {4, 8, 4, 4}, {4, 4, 6, 2} };
static const Line_t pattern_P[] = { {4, 2, 4, 14}, {4, 14, 10, 14}, {10, 14, 12, 12}, {12, 12, 12, 10}, {12, 10, 10, 8}, {10, 8, 4, 8} };
static const Line_t pattern_p[] = { {4, -2, 4, 8}, {4, 8, 8, 8}, {8, 8, 10, 6}, {10, 6, 8, 4}, {8, 4, 4, 2} };
static const Line_t pattern_Q[] = { {6, 2, 10, 2}, {10, 2, 12, 4}, {12, 4, 12, 12}, {12, 12, 10, 14}, {10, 14, 6, 14}, {6, 14, 4, 12}, {4, 12, 4, 4}, {4, 4, 6, 2}, {10, 6, 14, 2} };
static const Line_t pattern_q[] = { {10, -2, 10, 8}, {10, 8, 6, 8}, {6, 8, 4, 6}, {4, 6, 4, 4}, {4, 4, 6, 2}, {6, 2, 10, 2} };
static const Line_t pattern_R[] = { {4, 2, 4, 14}, {4, 14, 10, 14}, {10, 14, 12, 12}, {12, 12, 12, 10}, {12, 10, 10, 8}, {10, 8, 4, 8}, {8, 8, 14, 2} };
static const Line_t pattern_r[] = { {5, 8, 6, 8}, {6, 8, 6, 2}, {5, 2, 7, 2}, {6, 6, 8, 8}, {8, 8, 10, 8}, {10, 8, 12, 6} };
static const Line_t pattern_S[] = { {12, 12, 10, 14}, {10, 14, 6, 14}, {6, 14, 4, 12}, {4, 12, 4, 10}, {4, 10, 6, 8}, {6, 8, 10, 8}, {10, 8, 12, 6}, {12, 6, 12, 4}, {12, 4, 10, 2}, {10, 2, 6, 2}, {6, 2, 4, 4} };
static const Line_t pattern_s[] = { {7, 6, 6, 7}, {6, 7, 4, 7}, {4, 7, 3, 6}, {3, 6, 4, 4}, {4, 4, 6, 4}, {6, 4, 7, 3}, {7, 3, 6, 1}, {6, 1, 4, 1}, {4, 1, 3, 3} };
static const Line_t pattern_T[] = { {4, 14, 12, 14}, {8, 14, 8, 2} };
static const Line_t pattern_t[] = { {4, 8, 8, 8}, {6, 10, 6, 3}, {6, 3, 7, 2}, {7, 2, 8, 2} };
static const Line_t pattern_U[] = { {4, 14, 4, 4}, {4, 4, 6, 2}, {6, 2, 10, 2}, {10, 2, 12, 4}, {12, 4, 12, 14} };
static const Line_t pattern_u[] = { {4, 8, 4, 4}, {4, 4, 6, 2}, {6, 2, 8, 2}, {8, 2, 10, 4}, {10, 8, 10, 2} };
static const Line_t pattern_V[] = { {4, 14, 8, 2}, {8, 2, 12, 14} };
static const Line_t pattern_v[] = { {4, 8, 7, 2}, {7, 2, 10, 8} };
static const Line_t pattern_W[] = { {4, 14, 4, 2}, {4, 2, 8, 8}, {8, 8, 12, 2}, {12, 2, 12, 14} };
static const Line_t pattern_w[] = { {4, 8, 4, 4}, {4, 4, 6, 2}, {6, 2, 8, 4}, {8, 4, 8, 8}, {8, 8, 8, 4}, {8, 4, 10, 2}, {10, 2, 12, 4}, {12, 4, 12, 8} };
static const Line_t pattern_X[] = { {4, 14, 12, 2}, {12, 14, 4, 2} };
static const Line_t pattern_x[] = { {4, 8, 8, 2}, {8, 8, 4, 2} };
static const Line_t pattern_Y[] = { {4, 14, 8, 8}, {12, 14, 8, 8}, {8, 8, 8, 2} };
static const Line_t pattern_y[] = { {4, -2, 10, 8}, {7, 3, 4, 8} };
static const Line_t pattern_Z[] = { {4, 14, 12, 14}, {12, 14, 4, 2}, {4, 2, 12, 2} };
static const Line_t pattern_z[] = { {4, 8, 8, 8}, {8, 8, 4, 2}, {4, 2, 8, 2} };

static const Line_t pattern_0[] = { {6, 13, 6, 3}, {6, 3, 7, 2}, {7, 2, 11, 2}, {11, 2, 12, 3}, {12, 3, 12, 13}, {12, 13, 11, 14}, {11, 14, 7, 14}, {7, 14, 6, 13} };
static const Line_t pattern_1[] = { {6, 12, 8, 14}, {8, 14, 8, 2}, {6, 2, 10, 2} };
static const Line_t pattern_2[] = { {4, 12, 6, 14}, {6, 14, 10, 14}, {10, 14, 12, 12}, {12, 12, 4, 2}, {4, 2, 12, 2} };
static const Line_t pattern_3[] = { {6, 14, 10, 14}, {10, 14, 12, 11}, {12, 11, 10, 8}, {10, 8, 6, 8}, {10, 8, 12, 5}, {12, 5, 10, 2}, {10, 2, 6, 2} };
static const Line_t pattern_4[] = { {6, 14, 4, 6}, {4, 6, 12, 6}, {12, 6, 8, 6}, {8, 2, 8, 14} };
static const Line_t pattern_5[] = { {12, 14, 6, 14}, {6, 14, 6, 9}, {6, 9, 7, 8}, {7, 8, 11, 8}, {11, 8, 12, 7}, {12, 7, 12, 3}, {12, 3, 11, 2}, {11, 2, 6, 2} };
static const Line_t pattern_6[] = { {12, 13, 11, 14}, {11, 14, 7, 14}, {7, 14, 6, 13}, {6, 13, 6, 3}, {6, 3, 7, 2}, {7, 2, 11, 2}, {11, 2, 12, 3}, {12, 3, 12, 7}, {12, 7, 11, 8}, {11, 8, 6, 8} };
static const Line_t pattern_7[] = { {6, 14, 12, 14}, {12, 14, 10, 2} };
static const Line_t pattern_8[] = { {11, 8, 7, 8}, {7, 8, 6, 7}, {6, 7, 7, 8}, {7, 8, 6, 9}, {6, 9, 6, 13}, {6, 13, 7, 14}, {7, 14, 11, 14}, {11, 14, 12, 13}, {12, 13, 12, 9}, {12, 9, 11, 8}, {11, 8, 12, 7}, {12, 7, 12, 3}, {12, 3, 11, 2}, {11, 2, 7, 2}, {7, 2, 6, 3}, {6, 3, 6, 7}, {6, 7, 7, 8} };
static const Line_t pattern_9[] = { {7, 14, 6, 13}, {6, 13, 6, 9}, {6, 9, 7, 8}, {7, 8, 11, 8}, {11, 8, 12, 11}, {12, 3, 12, 11}, {11, 8, 7, 8}, {6, 9, 6, 13}, {6, 13, 7, 14}, {7, 14, 11, 14}, {11, 14, 12, 13}, {12, 13, 12, 9}, {12, 9, 11, 8}, {11, 8, 7, 8} };
// Symbols
static const Line_t pattern_excl[] = { {8, 14, 8, 6}, {8, 4, 8, 2} }; // !
static const Line_t pattern_apos[] = { {8, 14, 8, 10} }; // '
static const Line_t pattern_hash[] = { {6, 14, 6, 2}, {10, 14, 10, 2}, {4, 10, 12, 10}, {4, 6, 12, 6} }; // #
static const Line_t pattern_pct[] = { {4, 2, 12, 14}, {5, 13, 7, 13}, {7, 13, 7, 11}, {7, 11, 5, 11}, {5, 11, 5, 13}, {9, 5, 11, 5}, {11, 5, 11, 3}, {11, 3, 9, 3}, {9, 3, 9, 5} }; // %
static const Line_t pattern_caret[] = { {4, 8, 8, 14}, {8, 14, 12, 8} }; // ^
static const Line_t pattern_ast[] = { {4, 12, 12, 4}, {12, 12, 4, 4}, {8, 14, 8, 2}, {4, 8, 12, 8} }; // *
static const Line_t pattern_under[] = { {4, 2, 12, 2} }; // _
static const Line_t pattern_minus[] = { {4, 8, 12, 8} }; // -
static const Line_t pattern_plus[] = { {8, 14, 8, 2}, {4, 8, 12, 8} }; // +
static const Line_t pattern_eq[] = { {4, 10, 12, 10}, {4, 6, 12, 6} }; // =
static const Line_t pattern_bslash[] = { {4, 14, 12, 2} }; // \ (backslash)
static const Line_t pattern_fslash[] = { {4, 2, 12, 14} }; // /
static const Line_t pattern_lparen[] = { {10, 14, 6, 8}, {6, 8, 10, 2} }; // (
static const Line_t pattern_rparen[] = { {6, 14, 10, 8}, {10, 8, 6, 2} }; // )
static const Line_t pattern_lbrack[] = { {10, 14, 6, 14}, {6, 14, 6, 2}, {6, 2, 10, 2} }; // [
static const Line_t pattern_rbrack[] = { {6, 14, 10, 14}, {10, 14, 10, 2}, {10, 2, 6, 2} }; // ]
static const Line_t pattern_lbrace[] = { {10, 14, 8, 14}, {8, 14, 8, 8}, {8, 8, 6, 8}, {8, 8, 8, 2}, {8, 2, 10, 2} }; // {
static const Line_t pattern_rbrace[] = { {6, 14, 8, 14}, {8, 14, 8, 8}, {8, 8, 10, 8}, {8, 8, 8, 2}, {8, 2, 6, 2} }; // }
static const Line_t pattern_quote[] = { {6, 14, 6, 10}, {10, 14, 10, 10} }; // "
static const Line_t pattern_semi[] = { {8, 14, 8, 12}, {8, 6, 6, 2} }; // ;
static const Line_t pattern_colon[] = { {8, 13, 8, 11}, {8, 5, 8, 3} }; // :
static const Line_t pattern_comma[] = { {8, 4, 8, 2}, {8, 2, 6, 0} }; // ,
static const Line_t pattern_period[] = { {8, 2, 8, 4} }; // .
static const Line_t pattern_question[] = { {4, 10, 4, 12}, {4, 12, 7, 14}, {7, 14, 9, 14}, {9, 14, 12, 12}, {12, 12, 12, 10}, {12, 10, 8, 7}, {8, 7, 8, 5}, {8, 3, 8, 2} }; // ?
static const Line_t pattern_at[] = { {10, 6, 8, 6}, {8, 6, 7, 8}, {7, 8, 8, 10}, {8, 10, 10, 8}, {10, 8, 10, 6}, {10, 6, 12, 4}, {12, 4, 12, 12}, {12, 12, 4, 12}, {4, 12, 4, 4}, {4, 4, 12, 4} }; // @
static const Line_t pattern_dollar[] = { {10, 14, 6, 14}, {6, 14, 6, 8}, {6, 8, 10, 8}, {10, 8, 10, 2}, {10, 2, 6, 2}, {8, 15, 8, 1} }; // $
static const Line_t pattern_lt[] = { {10, 14, 4, 8}, {4, 8, 10, 2} }; // <
static const Line_t pattern_gt[] = { {4, 14, 10, 8}, {10, 8, 4, 2} }; // >
static const Line_t pattern_pipe[] = { {8, 14, 8, 2} }; // |
static const Line_t pattern_tilde[] = { {4, 6, 6, 10}, {6, 10, 10, 6}, {10, 6, 12, 10} }; // ~

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

// 当前选中的字体模式
static uint8_t pattern_index = 0;
static const Line_t *current_pattern = NULL;
static uint8_t current_pattern_length = 0;

// 变换参数：缩放（移位位数）和偏移（DAC单位）
static uint8_t global_scale_shift = 0;
static Coord_t offset_x = 0;
static Coord_t offset_y = 0;

// 绘图对象池设置
#define MAX_DRAW_OBJS 40
#define MAX_STR_LEN 64

// 绘图对象结构体
typedef struct {
  uint8_t active; // 是否激活
  DrawType type;  // 类型：文本、线、矩形、圆
  union {
      struct {
          char text[MAX_STR_LEN];
          Coord_t x, y;
          uint8_t scale;
          uint16_t spacing;
          Coord_t scroll_offset; // 滚动偏移量
          uint32_t last_scroll_time; // 上次滚动时间
          Coord_t total_width; // 总宽度
          Coord_t view_width; // 可视宽度
      } text_data;
      struct {
          Coord_t x0, y0, x1, y1;
      } line_data;
      struct {
          Coord_t x, y, w, h;
      } rect_data;
      struct {
          Coord_t x, y, r;
      } circle_data;
  } data;
} DrawObj;

static DrawObj draw_pool[MAX_DRAW_OBJS];

// 终端模式状态变量
static uint8_t term_scale = 0;
static Coord_t term_line_height = CANVAS_SIZE / 10;
static Coord_t term_char_spacing = CANVAS_SIZE / 40;
static int8_t term_current_line = 0;
static Coord_t term_cursor_x = 0;
static Coord_t term_cursor_y = CANVAS_SIZE; // 从顶部开始
static uint8_t term_max_lines = MAX_DRAW_OBJS;

// 绘图模式配置
static DrawMode current_draw_mode = DRAW_MODE_DMA;
static uint32_t cpu_draw_delay = 10; // CPU 绘图速度调节参数
static uint32_t cpu_jump_dwell = 0; // CPU 跳跃等待时间调节参数
static uint32_t draw_density = 100; // 绘图密度: 100 = 1.0x (正常), 200 = 2.0x (更慢/更亮), 50 = 0.5x (更快)

// 辅助函数：计算字符图案的最小/最大 X 坐标，用于确定字符宽度
static void compute_pattern_minmax_x(const Line_t *p, uint8_t len, int8_t *minx, int8_t *maxx){
}

// 根据字符设置当前使用的图案 (A-Z, 0-9, 符号)。如果设置成功返回 1，否则返回 0
static uint8_t set_pattern_by_char(char c){
    return 0;
}

// 更新绘图状态（例如处理文本滚动）
void DRAW_Update(void){
}

// 设置 CPU 绘图延迟 (控制绘制速度)
void DRAW_SetCPUDelay(uint32_t delay){
}

uint32_t DRAW_GetCPUDelay(void){
    return 0;
}

void DRAW_SetCPUJumpDwell(uint32_t dwell){
}

uint32_t DRAW_GetCPUJumpDwell(void){
    return 0;
}

void DRAW_SetDrawDensity(uint32_t density){
}

uint32_t DRAW_GetDrawDensity(void){
    return 0;
}

void DRAW_SetMode(DrawMode mode){
}

// CPU 模式渲染函数 (直接控制 DAC 寄存器)
static void DRAW_Render_CPU(void){
}

// 主渲染函数
void DRAW_Render(void){
}

// 清除所有绘图对象
void DRAW_Clear(void){
}

// 初始化绘图系统
void DRAW_Init(uint32_t interval_ms){
}

// 设置全局缩放比例 (移位位数)
void DRAW_SetScale(uint8_t scale_shift){
}

// 设置全局偏移量
void DRAW_SetOffset(Coord_t offset_x_param, Coord_t offset_y_param){
}

// 添加字符串到绘图池
int16_t DRAW_AddString(const char *s, uint16_t spacing, Coord_t x, Coord_t y, uint8_t scale){
    return -1;
}

// 获取文本滚动偏移量
Coord_t DRAW_GetTextScroll(const char *text) {
    return 0;
}

// 设置文本滚动偏移量
void DRAW_SetTextScroll(int16_t slot, Coord_t scroll) {
}

// 添加线段到绘图池
uint8_t DRAW_AddLine(Coord_t x0, Coord_t y0, Coord_t x1, Coord_t y1){
    return 0;
}

// 添加矩形到绘图池
uint8_t DRAW_AddRect(Coord_t x, Coord_t y, Coord_t w, Coord_t h){
    return 0;
}

// 添加圆到绘图池
uint8_t DRAW_AddCircle(Coord_t x, Coord_t y, Coord_t r){
    return 0;
}

// 绘制单个字符 (用于调试)
void DRAW_SetLetter(char c){
}

// 初始化终端模式
void DRAW_Terminal_Init(uint8_t scale_shift, Coord_t spacing){
}

// 设置终端字符间距
void DRAW_Terminal_SetSpacing(Coord_t spacing){
}

// 终端打印函数 (支持自动换行和滚动)
void DRAW_Terminal_Print(const char *str){
}
