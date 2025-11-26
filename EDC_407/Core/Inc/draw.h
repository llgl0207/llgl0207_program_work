#ifndef __DRAW_H__
#define __DRAW_H__

#include "main.h"
#include "tim.h"
#include "dac.h"

#ifdef __cplusplus
extern "C" {
#endif

// Buffer size for DMA (adjust based on RAM availability and complexity)
// Increased to 16384 to support more text (requires ~64KB RAM)
#define DRAW_BUF_SIZE 16384

void DRAW_Init(uint32_t interval_ms);
void DRAW_SetLetter(char c);
void DRAW_SetScale(uint16_t scale_x_percent, uint16_t scale_y_percent);
void DRAW_SetOffset(int16_t offset_x, int16_t offset_y);
// Add a string to the display pool. Returns 1 if successful, 0 if pool full.
uint8_t DRAW_AddString(const char *s, uint16_t spacing, int32_t x, int32_t y,
                       uint16_t scale_x, uint16_t scale_y);
// Clear drawing
void DRAW_Clear(void);

// Internal function to update the DMA buffer
void DRAW_Render(void);

// Terminal functions
void DRAW_Terminal_Init(uint16_t scale_pct);
void DRAW_Terminal_Print(const char *str);

#ifdef __cplusplus
}
#endif

#endif // __DRAW_H__
