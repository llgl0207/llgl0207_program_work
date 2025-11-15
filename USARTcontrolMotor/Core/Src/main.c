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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "dvc_serialplot.h"
#include "drv_bsp.h"
#include "drv_can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// 滤波器编号
#define CAN_FILTER(x) ((x) << 3)

// 接收队列
#define CAN_FIFO_0 (0 << 2)
#define CAN_FIFO_1 (1 << 2)

//标准帧或扩展帧
#define CAN_STDID (0 << 1)
#define CAN_EXTID (1 << 1)

// 数据帧或遥控帧
#define CAN_DATA_TYPE (0 << 0)
#define CAN_REMOTE_TYPE (1 << 0)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rx_byte; 
int direction=1;
int count=0;
int16_t current_value;

//Class_Serialplot serialplot;

int16_t Rx_Encoder, Rx_Omega, Rx_Torque, Rx_Temperature;
float Tx_Encoder, Tx_Omega, Tx_Torque, Tx_Temperature;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

void Function_For_1(void);
void Function_For_0(void);
void Function_For_m1(void);

void CAN_SendMessage(uint8_t *data);
void CAN_ReceiveHandler(uint32_t id, uint8_t *data, uint8_t length);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief CAN报文回调函数
 *
 * @param Rx_Buffer CAN接收的信息结构体
 */
void CAN_Motor_Call_Back(struct Struct_CAN_Rx_Buffer *Rx_Buffer)
{
  const char msg[] = "Into Call Back\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
    uint8_t *Rx_Data = Rx_Buffer->Data;
    switch (Rx_Buffer->Header.StdId)
    {
    case (0x209):
    {
        const char msg[] = "Into 0x209\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
        Rx_Encoder = (Rx_Data[0] << 8) | Rx_Data[1];
        Rx_Omega = (Rx_Data[2] << 8) | Rx_Data[3];
        Rx_Torque = (Rx_Data[4] << 8) | Rx_Data[5];
        Rx_Temperature = Rx_Data[6];
    }
    break;
    }
}


/**
 * @brief 初始化CAN总线
 *
 * @param hcan CAN编号
 * @param Callback_Function 处理回调函数
 */
/*void CAN_Init(CAN_HandleTypeDef *hcan)
{
  HAL_CAN_Start(hcan);
  __HAL_CAN_ENABLE_IT(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
  __HAL_CAN_ENABLE_IT(hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
}*/
/**
 * @brief 配置CAN的滤波器
 *
 * @param hcan CAN编号
 * @param Object_Para 编号 | FIFOx | ID类型 | 帧类型
 * @param ID ID
 * @param Mask_ID 屏蔽位(0x3ff, 0x1fffffff)
 */
/*
 void CAN_Filter_Mask_Config(CAN_HandleTypeDef *hcan, uint8_t Object_Para, uint32_t ID, uint32_t Mask_ID)
{
  CAN_FilterTypeDef can_filter_init_structure;

  // 检测关键传参
  assert_param(hcan != NULL);

  if ((Object_Para & 0x02))
  {
    // 标准帧
    // 掩码后ID的高16bit
    can_filter_init_structure.FilterIdHigh = ID << 3 >> 16;
    // 掩码后ID的低16bit
    can_filter_init_structure.FilterIdLow = ID << 3 | ((Object_Para & 0x03) << 1);
    // ID掩码值高16bit
    can_filter_init_structure.FilterMaskIdHigh = Mask_ID << 3 << 16;
    // ID掩码值低16bit
    can_filter_init_structure.FilterMaskIdLow = Mask_ID << 3 | ((Object_Para & 0x03) << 1);
  }
  else
  {
    // 扩展帧
    // 掩码后ID的高16bit
    can_filter_init_structure.FilterIdHigh = ID << 5;
    // 掩码后ID的低16bit
    can_filter_init_structure.FilterIdLow = ((Object_Para & 0x03) << 1);
    // ID掩码值高16bit
    can_filter_init_structure.FilterMaskIdHigh = Mask_ID << 5;
    // ID掩码值低16bit
    can_filter_init_structure.FilterMaskIdLow = ((Object_Para & 0x03) << 1);
  }

  // 滤波器序号, 0-27, 共28个滤波器, can1是0~13, can2是14~27
  can_filter_init_structure.FilterBank = Object_Para >> 3;
  // 滤波器绑定FIFOx, 只能绑定一个
  can_filter_init_structure.FilterFIFOAssignment = (Object_Para >> 2) & 0x01;
  // 使能滤波器
  can_filter_init_structure.FilterActivation = ENABLE;
  // 滤波器模式, 设置ID掩码模式
  can_filter_init_structure.FilterMode = CAN_FILTERMODE_IDMASK;
  // 32位滤波
  can_filter_init_structure.FilterScale = CAN_FILTERSCALE_32BIT;
    //从机模式选择开始单元
  can_filter_init_structure.SlaveStartFilterBank = 14;

  HAL_CAN_ConfigFilter(hcan, &can_filter_init_structure);
} */

/**
 * @brief 发送数据帧
 *
 * @param hcan CAN编号
 * @param ID ID
 * @param Data 被发送的数据指针
 * @param Length 长度
 * @return uint8_t 执行状态
 */
 /*uint8_t CAN_Send_Data(CAN_HandleTypeDef *hcan, uint16_t ID, uint8_t *Data, uint16_t Length)
{
  CAN_TxHeaderTypeDef tx_header;
  uint32_t used_mailbox; 

  // 检测关键传参
  assert_param(hcan != NULL);

  tx_header.StdId = ID;
  tx_header.ExtId = 0;
  tx_header.IDE = 0;
  tx_header.RTR = 0;
  tx_header.DLC = Length;

  return (HAL_CAN_AddTxMessage(hcan, &tx_header, Data, &used_mailbox));
}*/
/**
 * @brief 点灯
 *
 * @param data 收到的数据
 */
void LED_Control(uint8_t data)
{
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_1, ((data & 1) == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, ((data & 2) == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, ((data & 4) == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, ((data & 8) == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, ((data & 16) == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, ((data & 32) == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, ((data & 64) == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8, ((data & 128) == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
/**
 * @brief HAL库CAN接收FIFO1中断
 *
 * @param hcan CAN编号
 */
/*
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef header;
  uint8_t data;

  HAL_CAN_GetRxMessage(hcan, CAN_FILTER_FIFO1, &header, &data);
  const char msg[] = "Receive Interrupt\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
  LED_Control(data);
}
*/










void CAN_SendMessage(uint8_t *data) {
  CAN_TxHeaderTypeDef TxHeader;
  uint32_t TxMailbox;

  // ���÷�����Ϣͷ
  TxHeader.StdId = 0x2FE;
  TxHeader.ExtId = 0;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.DLC = 8;
  TxHeader.TransmitGlobalTime = DISABLE;

  if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, data, &TxMailbox) != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @brief CAN接收FIFO0消息回调函数
  * @param hcan: CAN句柄指针
  */
/*
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef RxHeader;
  uint8_t RxData[8];
  const char msg[] = "Receive Interrupt\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
  // 检查是否是CAN1
  if (hcan->Instance == CAN1)
  {
    // 获取接收到的消息
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
    {
      // 处理接收到的数据
      CAN_ReceiveHandler(RxHeader.StdId, RxData, RxHeader.DLC);
    }
  }
}
*/
/**
  * @brief CAN接收数据处理函数
  * @param id: CAN标识符
  * @param data: 数据数组
  * @param length: 数据长度
  */
/**********************************************************************************贵物代码
void CAN_ReceiveHandler(uint32_t id, uint8_t *data, uint8_t length)
{
	char uart_buffer[50];
  switch(id)
  {
    case 0x2FE:  // 你的设备ID
    {
      // 提取电机数据（前两个字节）
      int16_t motor_value = data[0] | (data[1] << 8);
      
      // 在这里处理电机数据
      //printf("收到电机数据: ID=0x%03X, 数值=%d\n", id, motor_value);
			snprintf(uart_buffer, sizeof(uart_buffer), "Motor Value: %d\r\n", motor_value);
      HAL_UART_Transmit(&huart1, (uint8_t*)uart_buffer, strlen(uart_buffer), 100);
      
      // 转换为百分比显示
      float percentage = (float)motor_value / 163.84f;
      //printf("相当于: %.2f%%\n", percentage);
      break;
    }
    
    case 0x201:  // 其他设备ID示例
    {
      //printf("收到设备0x201数据: ");
      for(int i = 0; i < length; i++) {
        //printf("%02X ", data[i]);
      }
      //printf("\n");
      break;
    }
    
    default:
      //printf("未知CAN ID: 0x%03X\n", id);
      break;
  }
}

*/

















void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  if(huart->Instance == USART1)
  {

    if(rx_byte == '+')
    {
      Function_For_1(); 
    }
    else if(rx_byte == '0')
    {
      Function_For_0(); 
    }
		else if(rx_byte == '-')
		{
			Function_For_m1();
		}

  
    HAL_UART_Transmit(&huart1, &rx_byte, 1, 0xFF);

 
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  }
}

void Function_For_1(void)
{
  const char msg[] = "-> Received '1', executing Clockwise Rotation...\r\n";
  direction=1;
	HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100); 
 
}


void Function_For_0(void)
{
  const char msg[] = "-> Received '0', executing Stop...\r\n";
	direction=0;
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100); // ??????????
 
}

void Function_For_m1(void)
{
  const char msg[] = "-> Received '-1', executing Anticlockwise Rotation...\r\n";
	direction=-1;
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100); // ??????????
 
}
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
  MX_USART1_UART_Init();
  MX_CAN1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_UART_Receive
	_IT(&huart1, &rx_byte, 1);
	
	
	
	
	
	
	/*
	if (HAL_CAN_Start(&hcan1) != HAL_OK) {
  // Start CAN.
  Error_Handler();
}*/
	uint8_t Send_Data = 0;

  //CAN_Init(&hcan1);
  //CAN_Filter_Mask_Config(&hcan1, CAN_FILTER(13) | CAN_FIFO_1 | CAN_STDID | CAN_DATA_TYPE, 0x114, 0x7ff);







	 CAN_Init(&hcan1, CAN_Motor_Call_Back);
    Uart_Init(&huart2, NULL, 0, NULL);
    CAN_Filter_Mask_Config(&hcan1, CAN_FILTER(13) | CAN_FIFO_1 | CAN_STDID | CAN_DATA_TYPE, 0x209, 0x7ff);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		Tx_Encoder = Rx_Encoder;
        Tx_Omega = Rx_Omega;
        Tx_Torque = Rx_Torque;
        Tx_Temperature = Rx_Temperature;
		count++;
		//const char msg[] = "loop\r\n";
		//HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
		
		
		
		
		uint8_t num=0x0A;
		if(direction==1){
		//if(count%2==0){current_value = +1000;HAL_Delay(100);}
		//if(count%2==1){current_value = +0;}
		current_value = +350;







		uint8_t canData[8] = {(uint8_t)((current_value >> 8) & 0xFF),(uint8_t)(current_value & 0xFF),num,num,num,num,num};
    CAN_SendMessage(canData);
		//CAN_Send_Data(&hcan1, 0x2fe, canData, 8);
    //HAL_Delay(100);  // ÿ�뷢��һ��
		}else if(direction==-1){
		current_value = -1000;
		//uint8_t num=0x0A; 
		uint8_t canData[8] = {(uint8_t)((current_value >> 8) & 0xFF),(uint8_t)(current_value & 0xFF),num,num,num,num,num};
    CAN_SendMessage(canData);
		//CAN_Send_Data(&hcan1, 0x2fe, canData, 8);
    //HAL_Delay(100);  // ÿ�뷢��һ��
		}else{
		current_value = +0;
		//uint8_t num=0x0A;
		uint8_t canData[8] = {(uint8_t)((current_value >> 8) & 0xFF),(uint8_t)(current_value & 0xFF),num,num,num,num,num};
    CAN_SendMessage(canData);
		//CAN_Send_Data(&hcan1, 0x2fe, canData, 8);
    //HAL_Delay(100);  // ÿ�뷢��һ��
		
		
		}
		HAL_Delay(100);
		
		
		
		
		/*
		int16_t current_value = +1000;
		uint8_t num=0x0A;
		uint8_t canData[8] = {(uint8_t)(current_value & 0xFF),(uint8_t)((current_value >> 8) & 0xFF),num,num,num,num,num};
    CAN_SendMessage(canData);
    HAL_Delay(100);  // ÿ�뷢��һ��
		*/
		
		
		
		
		
		
		
		
		
		
		
		
    /* ??????????????????????? */
		/*
		//HAL_UART_Transmit(&huart1, (uint8_t*)"Hello 32\r\n", sizeof("Hello 32\r\n")-1, 100);
		HAL_Delay(1000);
		int temperature = 25;
		char buffer[50];
		sprintf(buffer, "Temperature: %dC\r\n", temperature);
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
		*/
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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
