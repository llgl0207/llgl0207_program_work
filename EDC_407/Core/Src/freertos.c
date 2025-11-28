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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs.h"
#include "draw.h"
#include "tim.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    UI_MENU_MAIN,
    UI_MENU_MUSIC_LIST,
    UI_PLAYING,
    UI_MENU_TEXT_LIST,
    UI_TEXT_VIEWER
} UI_State;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SD_WAVE_MAX_LEN 16384
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern FIL fil;
extern FATFS fs;
extern SD_HandleTypeDef hsd;
extern uint8_t *SD_Wave_Buffer; // Changed to pointer
extern volatile uint32_t SD_Wave_Idx;
extern volatile uint32_t SD_Wave_Write_Idx;
extern uint8_t SD_Wave_Loaded;
extern uint32_t SD_Wave_Total_Data_Left;
extern uint8_t Video_Mode;

extern uint16_t volume;
extern uint8_t pitch;
extern int scale;
extern uint8_t func;
extern uint8_t Music_Score[];
extern uint8_t Wave[];

// Music Player Globals
#define MAX_MUSIC_FILES 20
#define MAX_FILENAME_LEN 32
char music_files[MAX_MUSIC_FILES][MAX_FILENAME_LEN];
int music_file_count = 0;
char current_playing_file[MAX_FILENAME_LEN];

// --- Text Viewer Variables ---
#define MAX_TEXT_LINES 1000
char *text_lines[MAX_TEXT_LINES];
int total_text_lines = 0;
int current_text_line = 0;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTask02Handle;
osThreadId myTask03Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Scan_Music_Files(const char* path);
void Play_Music(char* filename);
void Play_Video(char* filename);
void Stop_Music(void);
void Scan_Text_Files(const char* path);
void Open_Text_File(char* filename);

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
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTask02 */
  osThreadDef(myTask02, AudioTask, osPriorityHigh, 0, 1024);
  myTask02Handle = osThreadCreate(osThread(myTask02), NULL);

  /* definition and creation of myTask03 */
  osThreadDef(myTask03, GuiTask, osPriorityNormal, 0, 1024);
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
      
      /* 
      // Auto-play test.wav DISABLED to prevent interference with Menu
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
      */
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
  
  UI_State ui_state = UI_MENU_MAIN;
  
  // Main Menu Items
  const char *main_menu_items[] = {
      "MUSIC PLAYER",
      "VIDEO PLAYER",
      "TEXT BROWSER",
      "SETTINGS",   
      "ABOUT",
      "EXIT"
  };
  const int main_menu_count = 6;
  
  // State Variables
  int menu_index = 0;
  int menu_scroll = 0;
  int last_menu_index = -1;
  int last_menu_scroll = -1;
  int menu_mode = 0; // 0: Music, 1: Video, 2: Text
  
  // UI Constants
  const int line_height = 500;
  const int start_y = 3500;
  const int visible_lines = 6;
  
  DRAW_Clear();
  
  static uint16_t last_encoder = 0;
  static int32_t encoder_acc = 0;
  
  /* Infinite loop */
  for(;;)
  {
    // Update Drawing Animation
    if(!Video_Mode) {
        DRAW_Update();
    }

    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    
    // Encoder Logic
    uint16_t encoder = __HAL_TIM_GET_COUNTER(&htim8);
    int16_t delta = (int16_t)(encoder - last_encoder);
    last_encoder = encoder;
    
    // Handle Volume in Playing State
    if(ui_state == UI_PLAYING) {
        if(delta != 0) {
             int32_t new_vol = (int32_t)volume + (delta * 10);
             if(new_vol > 2000) new_vol = 2000;
             if(new_vol < 0) new_vol = 0;
             volume = (uint16_t)new_vol;
        }
    } else if(ui_state == UI_TEXT_VIEWER) {
        // Text Scrolling
        encoder_acc += delta;
        if(abs(encoder_acc) >= 4) {
            int steps = encoder_acc / 4;
            current_text_line += steps;
            encoder_acc -= steps * 4;
            
            if(current_text_line < 0) current_text_line = 0;
            if(current_text_line > total_text_lines - visible_lines) current_text_line = total_text_lines - visible_lines;
            if(current_text_line < 0) current_text_line = 0;
        }
    } else {
        // Menu Navigation
        encoder_acc += delta;
        if(abs(encoder_acc) >= 4) {
            int steps = encoder_acc / 4;
            menu_index += steps;
            encoder_acc -= steps * 4;
            
            int max_items = 0;
            if(ui_state == UI_MENU_MAIN) max_items = main_menu_count;
            else if(ui_state == UI_MENU_MUSIC_LIST || ui_state == UI_MENU_TEXT_LIST) max_items = music_file_count;
            
            if(menu_index < 0) menu_index = 0;
            if(menu_index >= max_items) menu_index = max_items - 1;
        }
    }
    
    // Button Logic (RS_Pin)
    if(HAL_GPIO_ReadPin(RS_GPIO_Port, RS_Pin) == GPIO_PIN_RESET) {
        osDelay(50); // Debounce
        if(HAL_GPIO_ReadPin(RS_GPIO_Port, RS_Pin) == GPIO_PIN_RESET) {
             while(HAL_GPIO_ReadPin(RS_GPIO_Port, RS_Pin) == GPIO_PIN_RESET) osDelay(10);
             
             if(ui_state == UI_MENU_MAIN) {
                 if(menu_index == 0) { // Music Player
                     Scan_Music_Files("/music");
                     ui_state = UI_MENU_MUSIC_LIST;
                     menu_mode = 0;
                     menu_index = 0;
                     menu_scroll = 0;
                     last_menu_index = -1; 
                 } else if(menu_index == 1) { // Video Player
                     Scan_Music_Files("/video");
                     ui_state = UI_MENU_MUSIC_LIST;
                     menu_mode = 1;
                     menu_index = 0;
                     menu_scroll = 0;
                     last_menu_index = -1;
                 } else if(menu_index == 2) { // Text Browser
                     Scan_Text_Files("/text");
                     ui_state = UI_MENU_TEXT_LIST;
                     menu_mode = 2;
                     menu_index = 0;
                     menu_scroll = 0;
                     last_menu_index = -1;
                 }
             } else if(ui_state == UI_MENU_MUSIC_LIST) {
                 if(menu_index == 0) { // Back
                     ui_state = UI_MENU_MAIN;
                     menu_index = 0;
                     menu_scroll = 0;
                     last_menu_index = -1;
                 } else {
                     // Play File
                     strncpy(current_playing_file, music_files[menu_index], MAX_FILENAME_LEN);
                     if(menu_mode == 0) Play_Music(current_playing_file);
                     else Play_Video(current_playing_file);
                     ui_state = UI_PLAYING;
                     last_menu_index = -1;
                 }
             } else if(ui_state == UI_MENU_TEXT_LIST) {
                 if(menu_index == 0) { // Back
                     ui_state = UI_MENU_MAIN;
                     menu_index = 0;
                     menu_scroll = 0;
                     last_menu_index = -1;
                 } else {
                     // Open Text File
                     Open_Text_File(music_files[menu_index]);
                     ui_state = UI_TEXT_VIEWER;
                     last_menu_index = -1; // Force redraw
                 }
             } else if(ui_state == UI_PLAYING) {
                 Stop_Music();
                 ui_state = UI_MENU_MUSIC_LIST;
                 last_menu_index = -1;
             } else if(ui_state == UI_TEXT_VIEWER) {
                 ui_state = UI_MENU_TEXT_LIST;
                 last_menu_index = -1;
             }
        }
    }
    
    // Render UI
    // Check if redraw needed
    int redraw = 0;
    if(ui_state == UI_TEXT_VIEWER) {
        static int last_text_line = -1;
        if(current_text_line != last_text_line) {
            redraw = 1;
            last_text_line = current_text_line;
        }
    } else {
        if(menu_index != last_menu_index || menu_scroll != last_menu_scroll || ui_state == UI_PLAYING) {
            redraw = 1;
        }
    }
    
    if(redraw && !(ui_state == UI_PLAYING && menu_mode == 1)) {
        DRAW_Clear();
        
        if(ui_state == UI_PLAYING) {
            DRAW_AddString("PLAYING:", 100, 100, 3500, 15, 15);
            DRAW_AddString(current_playing_file, 100, 100, 3000, 15, 15);
            
            char vol_str[16];
            sprintf(vol_str, "VOL: %d", volume);
            DRAW_AddString(vol_str, 100, 100, 2000, 15, 15);
            
            DRAW_AddString("[PRESS TO STOP]", 100, 100, 1000, 10, 10);
        } else if(ui_state == UI_TEXT_VIEWER) {
            // Render Text Lines
            for(int i=0; i<visible_lines; i++) {
                int line_idx = current_text_line + i;
                if(line_idx >= total_text_lines) break;
                
                int y_pos = start_y - (i * 600);
                // Use larger font for text: 15% scale//调节字号
                DRAW_AddString(text_lines[line_idx], 75, 100, y_pos, 15, 15);
            }
        } else {
            // Menu Rendering
            int count = 0;
            if(ui_state == UI_MENU_MAIN) count = main_menu_count;
            else count = music_file_count;
            
            // Calculate Scroll
            if(menu_index < menu_scroll) menu_scroll = menu_index;
            if(menu_index >= menu_scroll + visible_lines) menu_scroll = menu_index - visible_lines + 1;
            
            for(int i=0; i<visible_lines; i++) {
                int item_idx = menu_scroll + i;
                if(item_idx >= count) break;
                
                int y_pos = start_y - (i * line_height);
                
                // Draw Cursor
                if(item_idx == menu_index) {
                    DRAW_AddString(">", 100, 100, y_pos, 15, 15);
                    
                    // Scrolling Text
                    const char *text = (ui_state == UI_MENU_MAIN) ? main_menu_items[item_idx] : music_files[item_idx];
                    DRAW_AddString(text, 100, 400, y_pos, 15, 15);
                } else {
                    // Normal Text
                    const char *text = (ui_state == UI_MENU_MAIN) ? main_menu_items[item_idx] : music_files[item_idx];
                    DRAW_AddString(text, 100, 400, y_pos, 15, 15);
                }
            }
        }
        
        if(ui_state != UI_TEXT_VIEWER) DRAW_AddRect(0, 0, 4095, 4095);
        
        last_menu_index = menu_index;
        last_menu_scroll = menu_scroll;
    }
    
    osDelay(50);
  }
  /* USER CODE END GuiTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void Scan_Music_Files(const char* path) {
    DIR dir;
    FILINFO fno;
    FRESULT res;
    
    // Clear file list
    memset(music_files, 0, sizeof(music_files));
    music_file_count = 0;
    
    // Add "Back" option
    strcpy(music_files[0], "Back");
    music_file_count++;
    
    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if (fno.fattrib & AM_DIR) continue;
            
            // Check extension .wav
            if (strstr(fno.fname, ".wav") || strstr(fno.fname, ".WAV")) {
                if (music_file_count < MAX_MUSIC_FILES) {
                    strncpy(music_files[music_file_count], fno.fname, MAX_FILENAME_LEN - 1);
                    music_files[music_file_count][MAX_FILENAME_LEN - 1] = '\0';
                    music_file_count++;
                }
            }
        }
        f_closedir(&dir);
    }
}

void Play_Music(char* filename) {
    Stop_Music();
    
    char path[64];
    sprintf(path, "/music/%s", filename);
    
    FRESULT res = f_open(&fil, path, FA_READ);
    if(res == FR_OK) {
        UINT br;
        uint8_t header[44];
        f_read(&fil, header, 44, &br);
        
        if(strncmp((char*)header, "RIFF", 4) == 0 && strncmp((char*)&header[8], "WAVE", 4) == 0) {
             uint32_t data_size = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);
             SD_Wave_Total_Data_Left = data_size;
             
             uint32_t to_read = (data_size > SD_WAVE_MAX_LEN) ? SD_WAVE_MAX_LEN : data_size;
             f_read(&fil, SD_Wave_Buffer, to_read, &br);
             
             SD_Wave_Idx = 0;
             SD_Wave_Write_Idx = (br < SD_WAVE_MAX_LEN) ? br : 0;
             SD_Wave_Total_Data_Left -= br;
             
             SD_Wave_Loaded = 1;
             
             // Set Frequency (Assuming 44.1kHz for now)
             __HAL_TIM_SET_PRESCALER(&htim3, 0);
             __HAL_TIM_SET_AUTORELOAD(&htim3, 1904);
             __HAL_TIM_SET_COUNTER(&htim3, 0);
        } else {
            f_close(&fil);
        }
    }
}

void Play_Video(char* filename) {
    Stop_Music();
    
    char path[64];
    sprintf(path, "/video/%s", filename);
    
    FRESULT res = f_open(&fil, path, FA_READ);
    if(res == FR_OK) {
        UINT br;
        uint8_t header[44];
        f_read(&fil, header, 44, &br);
        
        if(strncmp((char*)header, "RIFF", 4) == 0 && strncmp((char*)&header[8], "WAVE", 4) == 0) {
             uint32_t data_size = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);
             SD_Wave_Total_Data_Left = data_size;
             
             uint32_t to_read = (data_size > SD_WAVE_MAX_LEN) ? SD_WAVE_MAX_LEN : data_size;
             f_read(&fil, SD_Wave_Buffer, to_read, &br);
             
             SD_Wave_Idx = 0;
             SD_Wave_Write_Idx = (br < SD_WAVE_MAX_LEN) ? br : 0;
             SD_Wave_Total_Data_Left -= br;
             
             SD_Wave_Loaded = 1;
             Video_Mode = 1;
             
             // --- Configure DAC for Immediate Output (No Trigger) ---
             extern DAC_HandleTypeDef hdac;
             HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
             HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
             HAL_DAC_Stop(&hdac, DAC_CHANNEL_1);
             HAL_DAC_Stop(&hdac, DAC_CHANNEL_2);
             
             DAC_ChannelConfTypeDef sConfig = {0};
             sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
             sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
             
             if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK) {
                 DRAW_Terminal_Print("DAC1 CFG ERR\n");
             }
             if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK) {
                 DRAW_Terminal_Print("DAC2 CFG ERR\n");
             }
             
             HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
             HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
             
             // Center Beam Initially
             HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
             HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2048);
             // -----------------------------------------------------
             
             // Set Frequency (Assuming 44.1kHz for now)
             __HAL_TIM_SET_PRESCALER(&htim3, 0);
             __HAL_TIM_SET_AUTORELOAD(&htim3, 1904);
             __HAL_TIM_SET_COUNTER(&htim3, 0);
        } else {
            f_close(&fil);
        }
    }
}

void Stop_Music(void) {
    SD_Wave_Loaded = 0;
    
    if(Video_Mode) {
        // --- Restore DAC for Drawing (Timer 6 Trigger) ---
        extern DAC_HandleTypeDef hdac;
        HAL_DAC_Stop(&hdac, DAC_CHANNEL_1);
        HAL_DAC_Stop(&hdac, DAC_CHANNEL_2);
        
        DAC_ChannelConfTypeDef sConfig = {0};
        sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
        sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
        
        HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);
        HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2);
        // DMA will be restarted by GuiTask -> DRAW_Render
        // -------------------------------------------------
    }
    
    Video_Mode = 0;
    osDelay(10); // Wait for AudioTask to pause
    f_close(&fil);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
}


void Scan_Text_Files(const char* path) {
    DIR dir;
    FILINFO fno;
    FRESULT res;
    
    // Reuse music_files array
    memset(music_files, 0, sizeof(music_files));
    music_file_count = 0;
    
    // Add "Back" option
    strcpy(music_files[0], "Back");
    music_file_count++;
    
    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if (fno.fattrib & AM_DIR) continue;
            
            // Check extension .txt
            if (strstr(fno.fname, ".txt") || strstr(fno.fname, ".TXT")) {
                if (music_file_count < MAX_MUSIC_FILES) {
                    strncpy(music_files[music_file_count], fno.fname, MAX_FILENAME_LEN - 1);
                    music_files[music_file_count][MAX_FILENAME_LEN - 1] = '\0';
                    music_file_count++;
                }
            }
        }
        f_closedir(&dir);
    }
}

void Open_Text_File(char* filename) {
    char path[64];
    sprintf(path, "/text/%s", filename);
    
    FRESULT res = f_open(&fil, path, FA_READ);
    if(res == FR_OK) {
        UINT br;
        // Read file with margin for expansion (Limit to 12KB)
        f_read(&fil, SD_Wave_Buffer, 12000, &br);
        SD_Wave_Buffer[br] = '\0'; // Null terminate
        f_close(&fil);
        
        // --- Auto Word Wrap Pass ---
        int col = 0;
        uint32_t len = br;
        for(int i=0; i<len; i++) {
            if(SD_Wave_Buffer[i] == '\n') {
                col = 0;
            } else if(SD_Wave_Buffer[i] == '\r') {
                // ignore
            } else {
                col++;
                if(col >= 10) { // Max chars per line (10 chars for scale 15)
                    // Insert newline
                    if(len < SD_WAVE_MAX_LEN - 2) {
                        // Shift data
                        memmove(&SD_Wave_Buffer[i+1], &SD_Wave_Buffer[i], len - i + 1); // +1 for null terminator
                        SD_Wave_Buffer[i] = '\n';
                        len++;
                        col = 0;
                    } else {
                        break; // Buffer full
                    }
                }
            }
        }
        // ---------------------------
        
        // Parse lines
        total_text_lines = 0;
        current_text_line = 0;
        
        char *p = (char*)SD_Wave_Buffer;
        text_lines[total_text_lines++] = p;
        
        while(*p && total_text_lines < MAX_TEXT_LINES) {
            if(*p == '\n') {
                *p = '\0'; // Replace newline with null
                // Handle Windows \r\n
                if(p > (char*)SD_Wave_Buffer && *(p-1) == '\r') *(p-1) = '\0';
                
                text_lines[total_text_lines++] = p + 1;
            }
            p++;
        }
    }
}
/* USER CODE END Application */
