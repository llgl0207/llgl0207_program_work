/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <ctype.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

FATFS fs;
FIL fil;
FRESULT fresult;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define FILE_BUFFER_SIZE 512
char file_buffer[FILE_BUFFER_SIZE];
#define LINE_BUFFER_SIZE 64
char line_buffer[LINE_BUFFER_SIZE];


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void trim(char *str) {
    int i;
    int len = strlen(str);

    // ??????
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }

    // ??????
    i = 0;
    while (isspace((unsigned char)str[i])) {
        i++;
    }
    if (i > 0) {
        memmove(str, str + i, len - i + 1);
    }
}//ues for the word in the commands.txt processing

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
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	
	//creat a text.txt to solve data
	  fresult = f_mount(&fs,"",1);
	 if(fresult != FR_OK)
	 {
	  Error_Handler();
	 }
  
	  fresult = f_open(&fil, "text.txt", FA_CREATE_ALWAYS | FA_WRITE);
	  if(fresult == FR_OK)
	  {
		 char data[] = "Hello STM32F429 SDIO!\r\n";
	   UINT BYTES_WRITTEN;
		 fresult = f_write(&fil, data,strlen(data),&BYTES_WRITTEN);
		 f_close(&fil);
	  }
	
//the function of data_read in sd card && control the f4 with the word in commands.txt
		    //read the data and save them in the buffer
		fresult = f_open(&fil, "commands.txt", FA_READ);
    if (fresult == FR_OK) 
   {
    UINT bytes_read;
    f_read(&fil, file_buffer, FILE_BUFFER_SIZE - 1, &bytes_read);
    file_buffer[bytes_read] = '\0';
     f_close(&fil);
   } 
   else 
    {
    Error_Handler();
	  }
	
		char *current_line = file_buffer;
    char *next_line;
		
		   //work for the commands 
		while(1)
    {
		      next_line = strchr(current_line, '\n');
          if (next_line != NULL) {*next_line = '\0';}
					
          if (strlen(current_line) == 0) 
						{
              if (next_line == NULL) break;
              current_line = next_line + 1;
              continue;
            }
						
          size_t len = strlen(current_line);
          if (len > 0 && current_line[len - 1] == '\r') 
					{
           current_line[len - 1] = '\0';
          }
					
					trim(current_line);
          
   /**
      * @brief  This function is executed in case of error occurrence.
      * @retval None
					for example:
					   if (strcmp(current_line, "LED_ON") == 0)
              {
               HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); 
               HAL_Delay(200);
              }
              else if (strcmp(current_line, "LED_OFF") == 0)
             {
               HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
               HAL_Delay(200);
             }
              else if (strncmp(current_line, "DELAY:", 6) == 0)
             {
              int delay_ms = atoi(current_line + 6);
              if (delay_ms > 0 && delay_ms <= 5000) 
               {
                HAL_Delay(delay_ms);
               }  
             }
					
      */

		if (next_line == NULL)  break;
    current_line = next_line + 1;
  }
					
			   
		 
		
		
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while(1)
	{
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
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* USER CODE END Error_Handler_Debug */
	__disable_irq();
	while (1){}
}

#ifdef  USE_FULL_ASSERT
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
