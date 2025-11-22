#ifndef __DRAW_H__
#define __DRAW_H__

#include "main.h"
#include "tim.h"

#ifdef __cplusplus
extern "C" {
#endif

void DRAW_Init(uint32_t interval_ms);
void DRAW_TimerStep(TIM_HandleTypeDef *htim);
void DRAW_AutoTick(uint32_t now);
void DRAW_SetLetter(char c);
void DRAW_SetInterval(uint32_t ms);
// Position and scale control (scale in percent, 100 = original size)
void DRAW_SetScale(uint16_t scale_x_percent, uint16_t scale_y_percent);
void DRAW_SetOffset(int16_t offset_x, int16_t offset_y);
// Add a string to the display pool. Returns 1 if successful, 0 if pool full.
uint8_t DRAW_AddString(const char *s, uint16_t spacing, int32_t x, int32_t y,
                       uint16_t scale_x, uint16_t scale_y);
// Draw string: copies up to 63 characters. spacing is extra DAC units between glyphs.
// (Legacy/Single mode wrapper or deprecated)
void DRAW_DrawString(const char *s, uint16_t spacing, int32_t base_x, int32_t base_y,
					 uint16_t scale_x_percent, uint16_t scale_y_percent);
// Clear drawing (stop and center DAC outputs)
void DRAW_Clear(void);
// Return non-zero while string drawing is in progress
uint8_t DRAW_IsBusy(void);

#ifdef __cplusplus
}
#endif

#endif // __DRAW_H__
