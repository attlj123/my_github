/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32h7xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
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
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */
/**************************************************
功能:ms延时，可以调用
输入:uint16 ms 延时时间
输出:无
***************************************************/
//void HAL_DelayMs(uint16_t ms)
//{
//    uint16_t i,j;

//    for(i=0;i<ms;i++)
//			for(j=0;j<8100;j++);
//}
/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
TIM_HandleTypeDef htim7;
void TIM7_Init(void)
{
	__HAL_RCC_TIM7_CLK_ENABLE();//使能TIM7 时钟

	htim7.Instance = TIM7;
	htim7.Init.Period = 9999;//设罗自动重装载值，计数期期为10000 个时钟周期
	htim7.Init.Prescaler = 2399;//设置预分频值，计数频率为100kHz
	htim7.Init.ClockDivision = 0;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.RepetitionCounter = 0;

	HAL_TIM_Base_Init(&htim7);

	HAL_NVIC_SetPriority(TIM7_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(TIM7_IRQn);

	HAL_TIM_Base_Start_IT(&htim7);
}

void TIM7_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim7);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM7) 
	{
		HAL_IncTick();
	}
}


void HAL_MspInit(void)
{
	/* USER CODE BEGIN MspInit 0 */
	GPIO_InitTypeDef GPIO_InitStructure = { 0 };
	/* USER CODE END MspInit 0 */

	__HAL_RCC_SYSCFG_CLK_ENABLE();

	/* System interrupt init*/
	
	//硬件超时定时器设置
	TIM7_Init();

	/* USER CODE BEGIN MspInit 1 */
	//	ETH_RESET-------------------------> PA3
	GPIO_InitStructure.Pin = GPIO_PIN_3; 
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;          //推挽复用
	GPIO_InitStructure.Pull = GPIO_NOPULL;              //不带上下拉
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;         //高速
	GPIO_InitStructure.Alternate = GPIO_AF11_ETH;       //复用为ETH功能
	HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);         //初始化
	//	ETH_RESET-------------------------> PC0
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
	HAL_Delay(5);

	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
  /* USER CODE END MspInit 1 */
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Supply configuration update enable
	*/
	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

	/** Configure the main internal regulator output voltage
	*/
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 5;
	RCC_OscInitStruct.PLL.PLLN = 192;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		__disable_irq();
		while (1);
	}

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
							  |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		__disable_irq();
		while (1);
	}

	/** Enables the Clock Security System
	*/
	HAL_RCC_EnableCSS();
}


///**
//* @brief ETH MSP Initialization
//* This function configures the hardware resources used in this example
//* @param heth: ETH handle pointer
//* @retval None
//*/
//void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
//{
//  GPIO_InitTypeDef GPIO_InitStruct = {0};
//  if(heth->Instance==ETH)
//  {
//		/* USER CODE BEGIN ETH_MspInit 0 */
//		/*网络引脚设置 RMII接口 
//		ETH_MDIO -------------------------> PA2
//		ETH_MDC --------------------------> PC1
//		ETH_RMII_REF_CLK------------------> PA1
//		ETH_RMII_CRS_DV ------------------> PA7
//		ETH_RMII_RXD0 --------------------> PC4
//		ETH_RMII_RXD1 --------------------> PC5
//		ETH_RMII_TX_EN -------------------> PB11
//		ETH_RMII_TXD0 --------------------> PB12
//		ETH_RMII_TXD1 --------------------> PB13
//		*/
//  /* USER CODE END ETH_MspInit 0 */
//    /* Peripheral clock enable */
//    __HAL_RCC_ETH1MAC_CLK_ENABLE();
//    __HAL_RCC_ETH1TX_CLK_ENABLE();
//    __HAL_RCC_ETH1RX_CLK_ENABLE();

//    __HAL_RCC_GPIOC_CLK_ENABLE();
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    /**ETH GPIO Configuration
//    PC1     ------> ETH_MDC
//    PA1     ------> ETH_REF_CLK
//    PA2     ------> ETH_MDIO
//    PA7     ------> ETH_CRS_DV
//    PC4     ------> ETH_RXD0
//    PC5     ------> ETH_RXD1
//    PB11     ------> ETH_TX_EN
//    PB12     ------> ETH_TXD0
//    PB13     ------> ETH_TXD1
//    */
//		
//    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
//    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

//    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//		
//	//	ETH_RESET-------------------------> PA3
//	GPIO_InitStruct.Pin = GPIO_PIN_3; 
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;          //推挽复用
//	GPIO_InitStruct.Pull = GPIO_NOPULL;              //不带上下拉
//	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;         //高速
//	GPIO_InitStruct.Alternate = GPIO_AF11_ETH;       //复用为ETH功能
//	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);         //初始化

//	//	ETH_RESET-------------------------> PC0
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
//	HAL_DelayMs(50);

//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
////		HAL_DelayMs(50);

////	HAL_NVIC_SetPriority( ETH_IRQn,	( uint32_t ) configMAX_SYSCALL_INTERRUPT_PRIORITY, 0 );
////	HAL_NVIC_EnableIRQ( ETH_IRQn );
//	/* USER CODE END ETH_MspInit 1 */
//  }
//}

///**
//* @brief ETH MSP De-Initialization
//* This function freeze the hardware resources used in this example
//* @param heth: ETH handle pointer
//* @retval None
//*/
//void HAL_ETH_MspDeInit(ETH_HandleTypeDef* heth)
//{
//  if(heth->Instance==ETH)
//  {
//  /* USER CODE BEGIN ETH_MspDeInit 0 */

//  /* USER CODE END ETH_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_ETH1MAC_CLK_DISABLE();
//    __HAL_RCC_ETH1TX_CLK_DISABLE();
//    __HAL_RCC_ETH1RX_CLK_DISABLE();

//    /**ETH GPIO Configuration
//    PC1     ------> ETH_MDC
//    PA1     ------> ETH_REF_CLK
//    PA2     ------> ETH_MDIO
//    PA7     ------> ETH_CRS_DV
//    PC4     ------> ETH_RXD0
//    PC5     ------> ETH_RXD1
//    PB11     ------> ETH_TX_EN
//    PB12     ------> ETH_TXD0
//    PB13     ------> ETH_TXD1
//    */
//    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);

//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7);

//    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13);

//  /* USER CODE BEGIN ETH_MspDeInit 1 */

//  /* USER CODE END ETH_MspDeInit 1 */
//  }

//}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
