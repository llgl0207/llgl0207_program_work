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

// Shape Types
typedef enum {
    DRAW_TYPE_TEXT = 0,
    DRAW_TYPE_LINE,
    DRAW_TYPE_RECT,
    DRAW_TYPE_CIRCLE
} DrawType;

typedef enum {
    DRAW_MODE_DMA = 0,
    DRAW_MODE_CPU
} DrawMode;

void DRAW_Init(uint32_t interval_ms);
void DRAW_SetMode(DrawMode mode);
void DRAW_SetCPUDelay(uint32_t delay);
uint32_t DRAW_GetCPUDelay(void);
void DRAW_SetCPUJumpDwell(uint32_t dwell);
uint32_t DRAW_GetCPUJumpDwell(void);
void DRAW_SetDrawDensity(uint32_t density);
uint32_t DRAW_GetDrawDensity(void);
void DRAW_SetLetter(char c);
void DRAW_SetScale(uint16_t scale_x_percent, uint16_t scale_y_percent);
void DRAW_SetOffset(int16_t offset_x, int16_t offset_y);

// Add a string to the display pool. Returns slot index if successful, -1 if pool full.
int16_t DRAW_AddString(const char *s, uint16_t spacing, int32_t x, int32_t y,
                       uint16_t scale_x, uint16_t scale_y);

// Get/Set scroll offset for text objects
int32_t DRAW_GetTextScroll(const char *text);
void DRAW_SetTextScroll(int16_t slot, int32_t scroll);

// Add geometric shapes
uint8_t DRAW_AddLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
uint8_t DRAW_AddRect(int32_t x, int32_t y, int32_t w, int32_t h);
uint8_t DRAW_AddCircle(int32_t x, int32_t y, int32_t r);

// Clear drawing
void DRAW_Clear(void);

// Internal function to update the DMA buffer
void DRAW_Render(void);

// Periodic update for animations (scrolling)
void DRAW_Update(void);

// Terminal functions
void DRAW_Terminal_Init(uint16_t scale_pct, int32_t spacing);
void DRAW_Terminal_SetSpacing(int32_t spacing);
void DRAW_Terminal_Print(const char *str);

#ifdef __cplusplus
}
#endif

#endif // __DRAW_H__
