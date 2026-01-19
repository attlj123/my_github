/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32h7xx_hal.h"
#include "string.h"
#include "stdio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#include "EthernetTCPIP.h"
#include "MessagePass.h"
#include "ControlParam.h"
#include "FunctionSet.h"
#include "ComHandle.h"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void MPU_Config( void );
void SystemClock_Config( void );

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

uint8_t g_HardAddress = 0;
//***************************************************
//函数名  : RS485控制I初始化
//输入说明: 无
//返回值：无
//*************************************************
void ControlHardAddress(uint8_t *addr)
{
	uint16_t ResultAdd = 0;
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
		
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
		
	//地址检测
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Pin = RS485_ADD0_PIN;
	HAL_GPIO_Init(RS485_ADD0_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = RS485_ADD1_PIN;
	HAL_GPIO_Init(RS485_ADD1_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = RS485_ADD2_PIN;
	HAL_GPIO_Init(RS485_ADD2_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = RS485_ADD3_PIN;
	HAL_GPIO_Init(RS485_ADD3_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = RS485_ADD4_PIN;
	HAL_GPIO_Init(RS485_ADD4_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = RS485_ADD5_PIN;
	HAL_GPIO_Init(RS485_ADD5_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = RS485_ADD6_PIN;
	HAL_GPIO_Init(RS485_ADD6_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = RS485_ADD7_PIN;
	HAL_GPIO_Init(RS485_ADD7_PORT, &GPIO_InitStruct);
	
	ResultAdd = ((uint16_t)HAL_GPIO_ReadPin(RS485_ADD7_PORT,RS485_ADD7_PIN)<<7|
				 (uint16_t)HAL_GPIO_ReadPin(RS485_ADD6_PORT,RS485_ADD6_PIN)<<6|
				 (uint16_t)HAL_GPIO_ReadPin(RS485_ADD5_PORT,RS485_ADD5_PIN)<<5|
				 (uint16_t)HAL_GPIO_ReadPin(RS485_ADD4_PORT,RS485_ADD4_PIN)<<4|
				 (uint16_t)HAL_GPIO_ReadPin(RS485_ADD3_PORT,RS485_ADD3_PIN)<<3|
				 (uint16_t)HAL_GPIO_ReadPin(RS485_ADD2_PORT,RS485_ADD2_PIN)<<2|
				 (uint16_t)HAL_GPIO_ReadPin(RS485_ADD1_PORT,RS485_ADD1_PIN)<<1|
				 (uint16_t)HAL_GPIO_ReadPin(RS485_ADD0_PORT,RS485_ADD0_PIN));

	 *addr = ResultAdd + 1;
}

int main(void)
{
	/* MPU Configuration--------------------------------------------------------*/
	MPU_Config();

//	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
//	SCB_EnableDCache();
	
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */
	uint32_t SYSCLK_Frequency;
	
	SystemCoreClockUpdate();
	
	SYSCLK_Frequency = HAL_RCC_GetSysClockFreq();
	
	if(SYSCLK_Frequency != (uint32_t)SystemCoreClock)
		return 0;
	
	/* USER CODE END SysInit */
	ControlHardAddress(&g_HardAddress);			//获取硬件地址
	
	HardParamInit();
	
	ControlParamInit();
	
	FunctionSetInit();

	EthernetTCPInit(g_HardAddress);
	
	ComHandleInit(g_HardAddress);
	
	MessagePassInit();
	
	vTaskStartScheduler();
	
	/* USER CODE END 2 */
	while (1);
	/* USER CODE END 3 */
}

/**
  * @brief ETH Initialization Function
  * @param None
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
