#include "draw_patterns.h"

// Full glyph set ported from reference/draw.c
#define SEG(name, ...) static const LineSeg name[] = { __VA_ARGS__ }

SEG(pattern_A, {2,1,4,7},{4,7,6,1},{3,4,5,4});
SEG(pattern_a, {3,5,5,5},{5,5,6,4},{6,4,6,1},{6,1,3,1},{3,1,2,2},{2,2,3,3},{3,3,6,3});
SEG(pattern_B, {2,1,2,7},{2,7,4,7},{4,7,5,6},{5,6,5,5},{5,5,4,4},{4,4,2,4},{4,4,5,3},{5,3,5,2},{5,2,4,1},{4,1,2,1});
SEG(pattern_b, {2,7,2,1},{2,1,4,1},{4,1,5,2},{5,2,5,3},{5,3,4,4},{4,4,2,4});
SEG(pattern_C, {6,6,5,7},{5,7,3,7},{3,7,2,6},{2,6,2,2},{2,2,3,1},{3,1,5,1},{5,1,6,2});
SEG(pattern_c, {5,3,4,4},{4,4,3,4},{3,4,2,3},{2,3,2,2},{2,2,3,1},{3,1,4,1},{4,1,5,2});
SEG(pattern_D, {2,1,2,7},{2,7,4,7},{4,7,6,5},{6,5,6,3},{6,3,4,1},{4,1,2,1});
SEG(pattern_d, {5,7,5,1},{5,1,3,1},{3,1,2,2},{2,2,2,3},{2,3,3,4},{3,4,5,4});
SEG(pattern_E, {6,7,2,7},{2,7,2,1},{2,1,6,1},{2,4,5,4});
SEG(pattern_e, {6,1,3,1},{3,1,2,2},{2,2,2,4},{2,4,3,5},{3,5,4,5},{4,5,5,4},{5,4,4,3},{4,3,2,3});
SEG(pattern_F, {2,1,2,7},{2,7,6,7},{2,4,5,4});
SEG(pattern_f, {5,6,4.5,6},{4.5,6,4,5.5},{4,5.5,4,4},{3,4,5,4},{4,4,4,1});
SEG(pattern_G, {6,6,5,7},{5,7,3,7},{3,7,2,6},{2,6,2,2},{2,2,3,1},{3,1,5,1},{5,1,6,2},{6,2,6,4},{6,4,4,4});
SEG(pattern_g, {2,-1,4,-1},{4,-1,5,0},{5,0,5,3},{5,3,4,4},{4,4,3,4},{3,4,2,3},{2,3,2,2},{2,2,3,1},{3,1,4,1},{4,1,5,2});
SEG(pattern_H, {2,1,2,7},{6,1,6,7},{2,4,6,4});
SEG(pattern_h, {3,1,3,7},{3,1,3,3},{3,3,4,4},{4,4,5,4},{5,4,6,3},{6,3,6,1});
SEG(pattern_I, {3,1,5,1},{4,1,4,7},{3,7,5,7});
SEG(pattern_i, {4,6,4,5},{3,4,4,4},{4,4,4,1},{3.5,1,4.5,1});
SEG(pattern_J, {5.5,7,6.5,7},{6,6,6,2},{6,2,5,1},{5,1,4,1});
SEG(pattern_j, {5,7,5,6},{5,3,5,0},{5,0,4,-1});
SEG(pattern_K, {2,7,2,1},{2,4,5,1},{2,4,5,7});
SEG(pattern_k, {3,1,3,6},{3,3,5,4},{3,3,5,1});
SEG(pattern_L, {2,7,2,1},{2,1,6,1});
SEG(pattern_l, {3.5,6.5,4,7},{4,7,4,1},{4,1,4.5,1.5});
SEG(pattern_M, {2,1,2,7},{2,7,4,4},{4,4,6,7},{6,7,6,1});
SEG(pattern_m, {2,1,2,4},{2,4,3,5},{3,5,4,4},{4,4,4,1},{4,1,4,4},{4,4,5,5},{5,5,6,4},{6,4,6,1});
SEG(pattern_N, {2,1,2,7},{2,7,6,1},{6,1,6,7});
SEG(pattern_n, {2,1,2,5},{2,4,3,5},{3,5,4,5},{4,5,5,4},{5,4,5,1});
SEG(pattern_O, {3,1,5,1},{5,1,6,2},{6,2,6,6},{6,6,5,7},{5,7,3,7},{3,7,2,6},{2,6,2,2},{2,2,3,1});
SEG(pattern_o, {3,1,4,1},{4,1,5,2},{5,2,5,4},{5,4,4,5},{4,5,3,5},{3,5,2,4},{2,4,2,2},{2,2,3,1});
SEG(pattern_P, {2,1,2,7},{2,7,5,7},{5,7,6,6},{6,6,6,5},{6,5,5,4},{5,4,2,4});
SEG(pattern_p, {2,-1,2,4},{2,4,4,4},{4,4,5,3},{5,3,4,2},{4,2,2,1});
SEG(pattern_Q, {3,1,5,1},{5,1,6,2},{6,2,6,6},{6,6,5,7},{5,7,3,7},{3,7,2,6},{2,6,2,2},{2,2,3,1},{5,3,7,1});
SEG(pattern_q, {5,-1,5,4},{5,4,3,4},{3,4,2,3},{2,3,2,2},{2,2,3,1},{3,1,5,1});
SEG(pattern_R, {2,1,2,7},{2,7,5,7},{5,7,6,6},{6,6,6,5},{6,5,5,4},{5,4,2,4},{4,4,7,1});
SEG(pattern_r, {2.5,4,3,4},{3,4,3,1},{2.5,1,3.5,1},{3,3,4,4},{4,4,5,4},{5,4,6,3});
SEG(pattern_S, {6,6,5,7},{5,7,3,7},{3,7,2,6},{2,6,2,5},{2,5,3,4},{3,4,5,4},{5,4,6,3},{6,3,6,2},{6,2,5,1},{5,1,3,1},{3,1,2,2});
SEG(pattern_s, {3.75,3,3,3.75},{3,3.75,2.25,3.75},{2.25,3.75,1.5,3},{1.5,3,2.25,2.25},{2.25,2.25,3,2.25},{3,2.25,3.75,1.5},{3.75,1.5,3,0.75},{3,0.75,2.25,0.75},{2.25,0.75,1.5,1.5});
SEG(pattern_T, {2,7,6,7},{4,7,4,1});
SEG(pattern_t, {2,4,4,4},{3,5,3,1.5},{3,1.5,3.5,1},{3.5,1,4,1});
SEG(pattern_U, {2,7,2,2},{2,2,3,1},{3,1,5,1},{5,1,6,2},{6,2,6,7});
SEG(pattern_u, {2,4,2,2},{2,2,3,1},{3,1,4,1},{4,1,5,2},{5,4,5,1});
SEG(pattern_V, {2,7,4,1},{4,1,6,7});
SEG(pattern_v, {2,4,3.5,1},{3.5,1,5,4});
SEG(pattern_W, {2,7,2,1},{2,1,4,4},{4,4,6,1},{6,1,6,7});
SEG(pattern_w, {2,4,2,2},{2,2,3,1},{3,1,4,2},{4,2,4,4},{4,4,4,2},{4,2,5,1},{5,1,6,2},{6,2,6,4});
SEG(pattern_X, {2,7,6,1},{6,7,2,1});
SEG(pattern_x, {2,4,4,1},{4,4,2,1});
SEG(pattern_Y, {2,7,4,4},{6,7,4,4},{4,4,4,1});
SEG(pattern_y, {2,-1,5,4},{3.5,1.5,2,4});
SEG(pattern_Z, {2,7,6,7},{6,7,2,1},{2,1,6,1});
SEG(pattern_z, {2,4,4,4},{4,4,2,1},{2,1,4,1});

SEG(pattern_0, {3,6.5,3,1.5},{3,1.5,3.5,1},{3.5,1,5.5,1},{5.5,1,6,1.5},{6,1.5,6,6.5},{6,6.5,5.5,7},{5.5,7,3.5,7},{3.5,7,3,6.5});
SEG(pattern_1, {3,6,4,7},{4,7,4,1},{3,1,5,1});
SEG(pattern_2, {2,6,3,7},{3,7,5,7},{5,7,6,6},{6,6,2,1},{2,1,6,1});
SEG(pattern_3, {3,7,5,7},{5,7,6,5.5},{6,5.5,5,4},{5,4,3,4},{5,4,6,2.5},{6,2.5,5,1},{5,1,3,1});
SEG(pattern_4, {3,7,2,3},{2,3,6,3},{6,3,4,3},{4,1,4,7});
SEG(pattern_5, {6,7,3,7},{3,7,3,4.5},{3,4.5,3.5,4},{3.5,4,5.5,4},{5.5,4,6,3.5},{6,3.5,6,1.5},{6,1.5,5.5,1},{5.5,1,3,1});
SEG(pattern_6, {6,6.5,5.5,7},{5.5,7,3.5,7},{3.5,7,3,6.5},{3,6.5,3,1.5},{3,1.5,3.5,1},{3.5,1,5.5,1},{5.5,1,6,1.5},{6,1.5,6,3.5},{6,3.5,5.5,4},{5.5,4,3,4});
SEG(pattern_7, {3,7,6,7},{6,7,5,1});
SEG(pattern_8, {5.5,4,3.5,4},{3.5,4,3,3.5},{3,3.5,3.5,4},{3.5,4,3,4.5},{3,4.5,3,6.5},{3,6.5,3.5,7},{3.5,7,5.5,7},{5.5,7,6,6.5},{6,6.5,6,4.5},{6,4.5,5.5,4},{5.5,4,6,3.5},{6,3.5,6,1.5},{6,1.5,5.5,1},{5.5,1,3.5,1},{3.5,1,3,1.5},{3,1.5,3,3.5},{3,3.5,3.5,4});
SEG(pattern_9, {3.5,7,3,6.5},{3,6.5,3,4.5},{3,4.5,3.5,4},{3.5,4,5.5,4},{5.5,4,6,5.5},{6,1.5,6,5.5},{5.5,4,3.5,4},{3,4.5,3,6.5},{3,6.5,3.5,7},{3.5,7,5.5,7},{5.5,7,6,6.5},{6,6.5,6,4.5},{6,4.5,5.5,4},{5.5,4,3.5,4});

SEG(pattern_excl, {2100,3295,2100,1295}, {2100,795,2100,295});
SEG(pattern_apos, {2100,3295,2100,2295});
SEG(pattern_hash, {1600,3295,1600,295}, {2600,3295,2600,295}, {1200,2295,3000,2295}, {1200,1295,3000,1295});
SEG(pattern_pct, {1200,295,3000,3295}, {1400,3095,1600,3095}, {1600,3095,1600,2895}, {1600,2895,1400,2895}, {1400,2895,1400,3095}, {2600,695,2800,695}, {2800,695,2800,495}, {2800,495,2600,495}, {2600,495,2600,695});
SEG(pattern_caret, {1200,1795,2100,3295}, {2100,3295,3000,1795});
SEG(pattern_ast, {1200,2795,3000,795}, {3000,2795,1200,795}, {2100,3295,2100,295}, {1200,1795,3000,1795});
SEG(pattern_under, {1200,295,3000,295});
SEG(pattern_minus, {1200,1795,3000,1795});
SEG(pattern_plus, {2100,3295,2100,295}, {1200,1795,3000,1795});
SEG(pattern_eq, {1200,2295,3000,2295}, {1200,1295,3000,1295});
SEG(pattern_bslash, {1200,3295,3000,295});
SEG(pattern_fslash, {1200,295,3000,3295});
SEG(pattern_lparen, {2600,3295,1600,1795}, {1600,1795,2600,295});
SEG(pattern_rparen, {1600,3295,2600,1795}, {2600,1795,1600,295});
SEG(pattern_lbrack, {2600,3295,1600,3295}, {1600,3295,1600,295}, {1600,295,2600,295});
SEG(pattern_rbrack, {1600,3295,2600,3295}, {2600,3295,2600,295}, {2600,295,1600,295});
SEG(pattern_lbrace, {2600,3295,2100,3295}, {2100,3295,2100,1795}, {2100,1795,1600,1795}, {2100,1795,2100,295}, {2100,295,2600,295});
SEG(pattern_rbrace, {1600,3295,2100,3295}, {2100,3295,2100,1795}, {2100,1795,2600,1795}, {2100,1795,2100,295}, {2100,295,1600,295});
SEG(pattern_quote, {1600,3295,1600,2295}, {2600,3295,2600,2295});
SEG(pattern_semi, {2100,3295,2100,2795}, {2100,1295,1700,295});
SEG(pattern_colon, {2100,3295,2100,2295}, {2100,1295,2100,295});
SEG(pattern_comma, {2100,495,2100,295}, {2100,295,1700,95});
SEG(pattern_period, {1900,495,2300,495}, {2300,495,2300,295}, {2300,295,1900,295}, {1900,295,1900,495});
SEG(pattern_question, {1200,2595,1200,2995}, {1200,2995,1800,3295}, {1800,3295,2400,3295}, {2400,3295,3000,2995}, {3000,2995,3000,2295}, {3000,2295,2100,1595}, {2100,1595,2100,1095}, {2100,595,2100,295});
SEG(pattern_at, {2600,1295,2200,1295}, {2200,1295,1800,1695}, {1800,1695,1800,2095}, {1800,2095,2200,2495}, {2200,2495,2600,2095}, {2600,2095,2600,1695}, {2600,1695,3000,1295}, {3000,1295,3000,2895}, {3000,2895,1400,2895}, {1400,2895,1400,895}, {1400,895,3000,895});
SEG(pattern_dollar, {2600,3295,1600,3295}, {1600,3295,1600,2095}, {1600,2095,2600,2095}, {2600,2095,2600,895}, {2600,895,1600,895}, {2100,3695,2100,495});
SEG(pattern_lt, {2600,3295,1200,1795}, {1200,1795,2600,295});
SEG(pattern_gt, {1200,3295,2600,1795}, {2600,1795,1200,295});
SEG(pattern_pipe, {2100,3295,2100,295});
SEG(pattern_tilde, {1200,1295,1600,2295}, {1600,2295,2200,1295}, {2200,1295,2600,2295});

// Map table for lookup
typedef struct { char c; const LineSeg* segs; uint8_t count; } MapEntry;
static const MapEntry mapTable[] = {
  {'A', pattern_A, (uint8_t)(sizeof(pattern_A)/sizeof(pattern_A[0]))},
  {'B', pattern_B, (uint8_t)(sizeof(pattern_B)/sizeof(pattern_B[0]))},
  {'C', pattern_C, (uint8_t)(sizeof(pattern_C)/sizeof(pattern_C[0]))},
  {'D', pattern_D, (uint8_t)(sizeof(pattern_D)/sizeof(pattern_D[0]))},
  {'E', pattern_E, (uint8_t)(sizeof(pattern_E)/sizeof(pattern_E[0]))},
  {'F', pattern_F, (uint8_t)(sizeof(pattern_F)/sizeof(pattern_F[0]))},
  {'G', pattern_G, (uint8_t)(sizeof(pattern_G)/sizeof(pattern_G[0]))},
  {'H', pattern_H, (uint8_t)(sizeof(pattern_H)/sizeof(pattern_H[0]))},
  {'I', pattern_I, (uint8_t)(sizeof(pattern_I)/sizeof(pattern_I[0]))},
  {'J', pattern_J, (uint8_t)(sizeof(pattern_J)/sizeof(pattern_J[0]))},
  {'K', pattern_K, (uint8_t)(sizeof(pattern_K)/sizeof(pattern_K[0]))},
  {'L', pattern_L, (uint8_t)(sizeof(pattern_L)/sizeof(pattern_L[0]))},
  {'M', pattern_M, (uint8_t)(sizeof(pattern_M)/sizeof(pattern_M[0]))},
  {'N', pattern_N, (uint8_t)(sizeof(pattern_N)/sizeof(pattern_N[0]))},
  {'O', pattern_O, (uint8_t)(sizeof(pattern_O)/sizeof(pattern_O[0]))},
  {'P', pattern_P, (uint8_t)(sizeof(pattern_P)/sizeof(pattern_P[0]))},
  {'Q', pattern_Q, (uint8_t)(sizeof(pattern_Q)/sizeof(pattern_Q[0]))},
  {'R', pattern_R, (uint8_t)(sizeof(pattern_R)/sizeof(pattern_R[0]))},
  {'S', pattern_S, (uint8_t)(sizeof(pattern_S)/sizeof(pattern_S[0]))},
  {'T', pattern_T, (uint8_t)(sizeof(pattern_T)/sizeof(pattern_T[0]))},
  {'U', pattern_U, (uint8_t)(sizeof(pattern_U)/sizeof(pattern_U[0]))},
  {'V', pattern_V, (uint8_t)(sizeof(pattern_V)/sizeof(pattern_V[0]))},
  {'W', pattern_W, (uint8_t)(sizeof(pattern_W)/sizeof(pattern_W[0]))},
  {'X', pattern_X, (uint8_t)(sizeof(pattern_X)/sizeof(pattern_X[0]))},
  {'Y', pattern_Y, (uint8_t)(sizeof(pattern_Y)/sizeof(pattern_Y[0]))},
  {'Z', pattern_Z, (uint8_t)(sizeof(pattern_Z)/sizeof(pattern_Z[0]))},
  {'0', pattern_0, (uint8_t)(sizeof(pattern_0)/sizeof(pattern_0[0]))},
  {'1', pattern_1, (uint8_t)(sizeof(pattern_1)/sizeof(pattern_1[0]))},
  {'2', pattern_2, (uint8_t)(sizeof(pattern_2)/sizeof(pattern_2[0]))},
  {'3', pattern_3, (uint8_t)(sizeof(pattern_3)/sizeof(pattern_3[0]))},
  {'4', pattern_4, (uint8_t)(sizeof(pattern_4)/sizeof(pattern_4[0]))},
  {'5', pattern_5, (uint8_t)(sizeof(pattern_5)/sizeof(pattern_5[0]))},
  {'6', pattern_6, (uint8_t)(sizeof(pattern_6)/sizeof(pattern_6[0]))},
  {'7', pattern_7, (uint8_t)(sizeof(pattern_7)/sizeof(pattern_7[0]))},
  {'8', pattern_8, (uint8_t)(sizeof(pattern_8)/sizeof(pattern_8[0]))},
  {'9', pattern_9, (uint8_t)(sizeof(pattern_9)/sizeof(pattern_9[0]))},
  {'!', pattern_excl, (uint8_t)(sizeof(pattern_excl)/sizeof(pattern_excl[0]))},
  {'\'', pattern_apos, (uint8_t)(sizeof(pattern_apos)/sizeof(pattern_apos[0]))},
  {'#', pattern_hash, (uint8_t)(sizeof(pattern_hash)/sizeof(pattern_hash[0]))},
  {'%', pattern_pct, (uint8_t)(sizeof(pattern_pct)/sizeof(pattern_pct[0]))},
  {'^', pattern_caret, (uint8_t)(sizeof(pattern_caret)/sizeof(pattern_caret[0]))},
  {'*', pattern_ast, (uint8_t)(sizeof(pattern_ast)/sizeof(pattern_ast[0]))},
  {'_', pattern_under, (uint8_t)(sizeof(pattern_under)/sizeof(pattern_under[0]))},
  {'-', pattern_minus, (uint8_t)(sizeof(pattern_minus)/sizeof(pattern_minus[0]))},
  {'+', pattern_plus, (uint8_t)(sizeof(pattern_plus)/sizeof(pattern_plus[0]))},
  {'=', pattern_eq, (uint8_t)(sizeof(pattern_eq)/sizeof(pattern_eq[0]))},
  {'\\', pattern_bslash, (uint8_t)(sizeof(pattern_bslash)/sizeof(pattern_bslash[0]))},
  {'/', pattern_fslash, (uint8_t)(sizeof(pattern_fslash)/sizeof(pattern_fslash[0]))},
  {'(', pattern_lparen, (uint8_t)(sizeof(pattern_lparen)/sizeof(pattern_lparen[0]))},
  {')', pattern_rparen, (uint8_t)(sizeof(pattern_rparen)/sizeof(pattern_rparen[0]))},
  {'[', pattern_lbrack, (uint8_t)(sizeof(pattern_lbrack)/sizeof(pattern_lbrack[0]))},
  {']', pattern_rbrack, (uint8_t)(sizeof(pattern_rbrack)/sizeof(pattern_rbrack[0]))},
  {'{', pattern_lbrace, (uint8_t)(sizeof(pattern_lbrace)/sizeof(pattern_lbrace[0]))},
  {'}', pattern_rbrace, (uint8_t)(sizeof(pattern_rbrace)/sizeof(pattern_rbrace[0]))},
  {'"', pattern_quote, (uint8_t)(sizeof(pattern_quote)/sizeof(pattern_quote[0]))},
  {';', pattern_semi, (uint8_t)(sizeof(pattern_semi)/sizeof(pattern_semi[0]))},
  {':', pattern_colon, (uint8_t)(sizeof(pattern_colon)/sizeof(pattern_colon[0]))},
  {',', pattern_comma, (uint8_t)(sizeof(pattern_comma)/sizeof(pattern_comma[0]))},
  {'.', pattern_period, (uint8_t)(sizeof(pattern_period)/sizeof(pattern_period[0]))},
  {'?', pattern_question, (uint8_t)(sizeof(pattern_question)/sizeof(pattern_question[0]))},
  {'@', pattern_at, (uint8_t)(sizeof(pattern_at)/sizeof(pattern_at[0]))},
  {'$', pattern_dollar, (uint8_t)(sizeof(pattern_dollar)/sizeof(pattern_dollar[0]))},
  {'<', pattern_lt, (uint8_t)(sizeof(pattern_lt)/sizeof(pattern_lt[0]))},
  {'>', pattern_gt, (uint8_t)(sizeof(pattern_gt)/sizeof(pattern_gt[0]))},
  {'|', pattern_pipe, (uint8_t)(sizeof(pattern_pipe)/sizeof(pattern_pipe[0]))},
  {'~', pattern_tilde, (uint8_t)(sizeof(pattern_tilde)/sizeof(pattern_tilde[0]))},
  {'a', pattern_a, (uint8_t)(sizeof(pattern_a)/sizeof(pattern_a[0]))},
  {'b', pattern_b, (uint8_t)(sizeof(pattern_b)/sizeof(pattern_b[0]))},
  {'c', pattern_c, (uint8_t)(sizeof(pattern_c)/sizeof(pattern_c[0]))},
  {'d', pattern_d, (uint8_t)(sizeof(pattern_d)/sizeof(pattern_d[0]))},
  {'e', pattern_e, (uint8_t)(sizeof(pattern_e)/sizeof(pattern_e[0]))},
  {'f', pattern_f, (uint8_t)(sizeof(pattern_f)/sizeof(pattern_f[0]))},
  {'g', pattern_g, (uint8_t)(sizeof(pattern_g)/sizeof(pattern_g[0]))},
  {'h', pattern_h, (uint8_t)(sizeof(pattern_h)/sizeof(pattern_h[0]))},
  {'i', pattern_i, (uint8_t)(sizeof(pattern_i)/sizeof(pattern_i[0]))},
  {'j', pattern_j, (uint8_t)(sizeof(pattern_j)/sizeof(pattern_j[0]))},
  {'k', pattern_k, (uint8_t)(sizeof(pattern_k)/sizeof(pattern_k[0]))},
  {'l', pattern_l, (uint8_t)(sizeof(pattern_l)/sizeof(pattern_l[0]))},
  {'m', pattern_m, (uint8_t)(sizeof(pattern_m)/sizeof(pattern_m[0]))},
  {'n', pattern_n, (uint8_t)(sizeof(pattern_n)/sizeof(pattern_n[0]))},
  {'o', pattern_o, (uint8_t)(sizeof(pattern_o)/sizeof(pattern_o[0]))},
  {'p', pattern_p, (uint8_t)(sizeof(pattern_p)/sizeof(pattern_p[0]))},
  {'q', pattern_q, (uint8_t)(sizeof(pattern_q)/sizeof(pattern_q[0]))},
  {'r', pattern_r, (uint8_t)(sizeof(pattern_r)/sizeof(pattern_r[0]))},
  {'s', pattern_s, (uint8_t)(sizeof(pattern_s)/sizeof(pattern_s[0]))},
  {'t', pattern_t, (uint8_t)(sizeof(pattern_t)/sizeof(pattern_t[0]))},
  {'u', pattern_u, (uint8_t)(sizeof(pattern_u)/sizeof(pattern_u[0]))},
  {'v', pattern_v, (uint8_t)(sizeof(pattern_v)/sizeof(pattern_v[0]))},
  {'w', pattern_w, (uint8_t)(sizeof(pattern_w)/sizeof(pattern_w[0]))},
  {'x', pattern_x, (uint8_t)(sizeof(pattern_x)/sizeof(pattern_x[0]))},
  {'y', pattern_y, (uint8_t)(sizeof(pattern_y)/sizeof(pattern_y[0]))},
  {'z', pattern_z, (uint8_t)(sizeof(pattern_z)/sizeof(pattern_z[0]))},
};

const GlyphPattern* getGlyphPattern(char c) {
  static GlyphPattern gp;
  for (size_t i=0;i<sizeof(mapTable)/sizeof(mapTable[0]);i++) {
    if (mapTable[i].c == c) {
      gp.segs = mapTable[i].segs;
      gp.count = mapTable[i].count;
      return &gp;
    }
  }
  // default to space
  static const LineSeg pattern_space[] = {};
  gp.segs = pattern_space;
  gp.count = 0;
  return &gp;
}
