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
uint16_t (*current_Line)[4];
#define queue_LENGTH 8
// 4 个不重叠的 X，每个 X 用两条对角线表示（共 8 条线）
// 中心分别位于 (512,512), (512,3072), (3072,512), (3072,3072)，半径 r=400
uint16_t Draw_queue[queue_LENGTH][4] = {
  {112, 112,  912,  912}, // X1 diag A
  {112, 912,  912,  112}, // X1 diag B
  {112, 2672, 912, 3472}, // X2 diag A
  {112, 3472, 912, 2672}, // X2 diag B
  {2672, 112, 3472,  912}, // X3 diag A
  {2672, 912, 3472,  112}, // X3 diag B
  {2672,2672, 3472, 3472}, // X4 diag A
  {2672,3472, 3472, 2672}  // X4 diag B
};
uint8_t Draw_queue_index=0;




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
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
  /* 初始化当前绘制行，避免定时器回调中使用未初始化指针 */
  current_Line = &Draw_queue[Draw_queue_index];
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
    
		if(done==2){
      Draw_queue_index = (Draw_queue_index + 1) % queue_LENGTH;
      current_Line = &Draw_queue[Draw_queue_index];
      done = 0;
    }
		//HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
		//HAL_Delay(1000);
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
