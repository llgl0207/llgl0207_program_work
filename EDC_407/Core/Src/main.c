/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "can.h"
#include "dac.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t done=0;
// 当前绘制行（指向当前模式的某一条线）
const uint16_t (*current_Line)[4];

// 定义 A/B/C 三个字母的线段集合（每行为 {x0,y0,x1,y1}）
// 坐标范围为 0..4095（12-bit DAC）
static const uint16_t pattern_A[][4] = {
  {0,0,2048,4096},
  {2048,4096,4096,0},
  {1024,2048,3072,2048}
};
static const uint16_t pattern_B[][4] = {
  {0,0,0,4096},
  {0,4096,2048,4096},
  {2048,4096,3072,3072},
  {3072,3072,2048,2048},
  {2048,2048,0,2048},
  {2048,2048,3072,1024},
  {3072,1024,2048,0},
  {2048,0,0,0}
};
static const uint16_t pattern_C[][4] = {
  {3000, 1000, 2000, 800},
  {2000, 800, 1200, 1400},
  {1200, 1400, 1000, 2600},
  {1000, 2600, 1200, 3200},
  {1200, 3200, 2000, 3800},
  {2000, 3800, 3000, 3600}
};

/* 模式表与状态变量（pattern_A/B/C 已在上面定义） */
static const uint16_t (*patterns[])[4] = { pattern_A, pattern_B, pattern_C };
static const uint8_t pattern_lengths[] = {
  sizeof(pattern_A)/sizeof(pattern_A[0]),
  sizeof(pattern_B)/sizeof(pattern_B[0]),
  sizeof(pattern_C)/sizeof(pattern_C[0])
};
static uint8_t pattern_index = 0;
const uint16_t (*current_pattern)[4];
static uint8_t current_pattern_length = 0;
static uint8_t line_index = 0;

/*
 * @retval int
 */
void SystemClock_Config(void);

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN1_Init();
  MX_DAC_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */
  /* 初始化当前绘制模式与行，避免定时器回调中使用未初始化指针 */
  current_pattern = patterns[pattern_index];
  current_pattern_length = pattern_lengths[pattern_index];
  line_index = 0;
  current_Line = &current_pattern[line_index];
  HAL_TIM_Base_Start_IT(&htim14);  // 启动TIM14中断
HAL_DAC_Start(&hdac, DAC_CHANNEL_1); // ����DACͨ��1
HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
HAL_DAC_Start(&hdac, DAC_CHANNEL_2 );// ����DACͨ��1
HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2048);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* 自动定时切换模式：每 interval_ms 毫秒切换到下一个字母 */
    const uint32_t interval_ms = 1000; // ms
    static uint32_t last_switch = 0;
    uint32_t now = HAL_GetTick();
    if((now - last_switch) >= interval_ms){
      last_switch = now;
      pattern_index = (pattern_index + 1) % (sizeof(patterns)/sizeof(patterns[0]));
      current_pattern = patterns[pattern_index];
      current_pattern_length = pattern_lengths[pattern_index];
      line_index = 0;
      current_Line = &current_pattern[line_index];
      done = 0;
    }

    if(done==2){
      line_index = (line_index + 1) % current_pattern_length;
      current_Line = &current_pattern[line_index];
      done = 0;
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static uint16_t current_step; // 修正拼写
  static uint16_t step_max;     // 移到此处声明
  if(htim->Instance == TIM14)
  {
    if(done==0){
      done=1;
      //使用勾股定理计算步数
      // 移除了 static 初始化，改为赋值
      {
        int32_t dx = (int32_t)(*current_Line)[0] - (int32_t)(*current_Line)[2];
        int32_t dy = (int32_t)(*current_Line)[1] - (int32_t)(*current_Line)[3];
        step_max = (uint16_t)sqrt((double)dx * (double)dx + (double)dy * (double)dy);
        current_step = 0;
      }
    }
    if(done==1){
      current_step++;//步数加1
      if(step_max != 0) { // 防止除以0
        int32_t dx = (int32_t)(*current_Line)[0] - (int32_t)(*current_Line)[2];
        int32_t dy = (int32_t)(*current_Line)[1] - (int32_t)(*current_Line)[3];
        int32_t current_X = dx * (int32_t)(step_max - current_step) / (int32_t)step_max + (int32_t)(*current_Line)[2];
        int32_t current_Y = dy * (int32_t)(step_max - current_step) / (int32_t)step_max + (int32_t)(*current_Line)[3];
        if(current_X < 0) current_X = 0;
        if(current_X > 4095) current_X = 4095;
        if(current_Y < 0) current_Y = 0;
        if(current_Y > 4095) current_Y = 4095;
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint32_t)current_X);
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (uint32_t)current_Y);
      }
      if(current_step>=step_max){ // 增加安全性
        done=2;//完成
      }
    }
  }
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
