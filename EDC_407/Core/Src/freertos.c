/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "draw.h"
#include "tim.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SD_WAVE_MAX_LEN 32768
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern FIL fil;
extern FATFS fs;
extern SD_HandleTypeDef hsd;
extern uint8_t SD_Wave_Buffer[];
extern volatile uint32_t SD_Wave_Idx;
extern volatile uint32_t SD_Wave_Write_Idx;
extern uint8_t SD_Wave_Loaded;
extern uint32_t SD_Wave_Total_Data_Left;

extern uint16_t volume;
extern uint8_t pitch;
extern int scale;
extern uint8_t func;
extern uint8_t Music_Score[];
extern uint8_t Wave[];
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTask02Handle;
osThreadId myTask03Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void AudioTask(void const * argument);
void GuiTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTask02 */
  osThreadDef(myTask02, AudioTask, osPriorityHigh, 0, 1024);
  myTask02Handle = osThreadCreate(osThread(myTask02), NULL);

  /* definition and creation of myTask03 */
  osThreadDef(myTask03, GuiTask, osPriorityNormal, 0, 512);
  myTask03Handle = osThreadCreate(osThread(myTask03), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  
  // --- SD Card Initialization ---
  osDelay(1000); // Wait for power stabilization
  
  FRESULT res;
  
  // Mount SD Card
  res = f_mount(&fs, "0:", 1);
  if(res == FR_OK)
  {
      DRAW_Terminal_Print("MOUNT OK\n");
      
      // Open WAV file
      res = f_open(&fil, "test.wav", FA_READ);
      if(res == FR_OK)
      {
          UINT br;
          uint8_t header[44];
          
          // Read WAV Header
          f_read(&fil, header, 44, &br);
          
          // Simple WAV Validation (RIFF, WAVE)
          if(strncmp((char*)header, "RIFF", 4) == 0 && strncmp((char*)&header[8], "WAVE", 4) == 0)
          {
              // Parse Data Size (Little Endian at offset 40)
              uint32_t data_size = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);
              SD_Wave_Total_Data_Left = data_size;
              
              // Pre-fill Buffer
              uint32_t to_read = (data_size > SD_WAVE_MAX_LEN) ? SD_WAVE_MAX_LEN : data_size;
              
              f_read(&fil, SD_Wave_Buffer, to_read, &br);
              
              SD_Wave_Idx = 0;
              SD_Wave_Write_Idx = (br < SD_WAVE_MAX_LEN) ? br : 0;
              SD_Wave_Total_Data_Left -= br;
              
              SD_Wave_Loaded = 1;
              
              char msg[32];
              sprintf(msg, "PLAYING: %d KB\n", data_size/1024);
              DRAW_Terminal_Print(msg);
              
              // Adjust Timer Frequency for 44.1kHz
              __HAL_TIM_SET_PRESCALER(&htim3, 0);
              __HAL_TIM_SET_AUTORELOAD(&htim3, 1904);
              __HAL_TIM_SET_COUNTER(&htim3, 0);
              DRAW_Terminal_Print("FREQ SET: 44.1kHz\n");
          }
          else
          {
              DRAW_Terminal_Print("INVALID WAV\n");
              f_close(&fil);
          }
      }
      else
      {
          DRAW_Terminal_Print("NO test.wav\n");
      }
  }
  else
  {
      char mnt_err[32];
      sprintf(mnt_err, "MNT ERR:%d\n", res);
      DRAW_Terminal_Print(mnt_err);
  }

  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_AudioTask */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AudioTask */
void AudioTask(void const * argument)
{
  /* USER CODE BEGIN AudioTask */
  /* Infinite loop */
  for(;;)
  {
    if(SD_Wave_Loaded)
    {
        // Streaming Logic
        uint32_t buffered;
        uint32_t current_read_idx = SD_Wave_Idx; // Snapshot volatile variable
        
        if(SD_Wave_Write_Idx >= current_read_idx)
            buffered = SD_Wave_Write_Idx - current_read_idx;
        else
            buffered = SD_WAVE_MAX_LEN - (current_read_idx - SD_Wave_Write_Idx);
        
        // If buffer is less than half full, refill
        if(buffered < (SD_WAVE_MAX_LEN / 2))
        {
            UINT br;
            uint32_t to_read = 4096; // Read 4KB chunks
            FRESULT res;
            
            // Determine where to write
            uint32_t space_at_end = SD_WAVE_MAX_LEN - SD_Wave_Write_Idx;
            
            if(to_read > space_at_end)
            {
                // Split read
                res = f_read(&fil, &SD_Wave_Buffer[SD_Wave_Write_Idx], space_at_end, &br);
                
                if(br < space_at_end) { 
                    f_lseek(&fil, 44); 
                    // Try reading rest from start
                    UINT br2;
                    f_read(&fil, &SD_Wave_Buffer[SD_Wave_Write_Idx + br], space_at_end - br, &br2);
                } 
                
                res = f_read(&fil, &SD_Wave_Buffer[0], to_read - space_at_end, &br);
                
                if(br < (to_read - space_at_end)) { 
                    f_lseek(&fil, 44);
                    UINT br2;
                    f_read(&fil, &SD_Wave_Buffer[br], (to_read - space_at_end) - br, &br2);
                }
                
                SD_Wave_Write_Idx = to_read - space_at_end;
            }
            else
            {
                res = f_read(&fil, &SD_Wave_Buffer[SD_Wave_Write_Idx], to_read, &br);
                
                if(br < to_read) { 
                    f_lseek(&fil, 44); 
                    UINT br2;
                    f_read(&fil, &SD_Wave_Buffer[SD_Wave_Write_Idx + br], to_read - br, &br2);
                } 
                
                SD_Wave_Write_Idx += to_read;
                if(SD_Wave_Write_Idx >= SD_WAVE_MAX_LEN) SD_Wave_Write_Idx = 0;
            }
        }
        osDelay(2); // Check every 2ms
    }
    else
    {
        osDelay(100);
    }
  }
  /* USER CODE END AudioTask */
}

/* USER CODE BEGIN Header_GuiTask */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_GuiTask */
void GuiTask(void const * argument)
{
  /* USER CODE BEGIN GuiTask */
  static uint32_t last_print = 0;
  static int alpha_idx = 0;
  const char alpha_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    
    // --- Alphabet Test ---
    if(HAL_GetTick() - last_print > 500)
    {
         last_print = HAL_GetTick();
         char s[2] = {0};
         s[0] = alpha_map[alpha_idx];
         DRAW_Terminal_Print(s);
         
         alpha_idx++;
         if(alpha_map[alpha_idx] == '\0') alpha_idx = 0;
    }
    
    // Encoder Logic
    static uint16_t last_encoder=0;
    uint16_t encoder = __HAL_TIM_GET_COUNTER(&htim8);
    int delta = encoder - last_encoder;
    if(delta > 1000 || delta < -1000) delta = 0;
    last_encoder = encoder;
    
    if(func==0){ volume += delta/4; }
    if(func==1){ pitch += delta/4; scale = pitch; }
    
    // Music Logic (Legacy)
    if(!SD_Wave_Loaded)
    {
        // Simple fallback music logic
        // Note: We don't have the full music logic here, just a placeholder
        // to avoid compilation errors if we try to use Music_Score
        osDelay(100);
    }
    else
    {
        osDelay(100);
    }
  }
  /* USER CODE END GuiTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
