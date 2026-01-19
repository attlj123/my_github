#include "Driver_Uart.h"

#include "stm32h7xx_hal.h"

UART_HandleTypeDef huart;
uint8_t UsartRXTemp = 0;

extern void Error_Handler(void);
extern void  App_Usart_IrqDataHandle(uint8 IrqData);
//***************************************************
//函数名  :  UartInitial
//功能说明： 初始化通信串口
//输入说明： USART_TypeDef* USARTx:具体的COM口
//           uint32 baudspeed:串口波特率
//           uint8 preInterruptPrioty:串口接收中断先占优先级
//返回值  ：无
//***************************************************
void UartInitial(uint32 baudspeed, uint8 preInterruptPrioty)
{ 	
		//配置串口参数
		huart.Instance = USART1;
		huart.Init.BaudRate = baudspeed;
		huart.Init.WordLength = UART_WORDLENGTH_8B;
		huart.Init.StopBits = UART_STOPBITS_1;
		huart.Init.Parity = UART_PARITY_NONE;
		huart.Init.Mode = UART_MODE_TX_RX;
		huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart.Init.OverSampling = UART_OVERSAMPLING_16;
		huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
		huart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
		huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
		
		if (HAL_UART_Init(&huart) != HAL_OK)
		{
			Error_Handler();
		}
		if (HAL_UARTEx_SetTxFifoThreshold(&huart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
		{
			Error_Handler();
		}
		if (HAL_UARTEx_SetRxFifoThreshold(&huart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
		{
			Error_Handler();
		}
		if (HAL_UARTEx_DisableFifoMode(&huart) != HAL_OK)
		{
			Error_Handler();
		}
		

		HAL_UART_Receive_IT(&huart,&UsartRXTemp,1);

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART16;
		PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
		{
			Error_Handler();
		}

		/* UART8 clock enable */
		__HAL_RCC_USART1_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**UART8 GPIO Configuration
		PE0     ------> UART8_RX
		PE1     ------> UART8_TX
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* UART8 interrupt Init */
		HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
		/* USER CODE END UART8_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART1_CLK_DISABLE();

		/**UART8 GPIO Configuration
		PE0     ------> UART8_RX
		PE1     ------> UART8_TX
		*/
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10|GPIO_PIN_9);

		/* UART8 interrupt Deinit */
		HAL_NVIC_DisableIRQ(USART1_IRQn);
		/* USER CODE BEGIN UART8_MspDeInit 1 */

		/* USER CODE END UART8_MspDeInit 1 */
}

//***************************************************
//函数名  :  UartTxFrame
//功能说明： UART发送一个数组
//输入说明： USART_TypeDef* USARTx：哪一个UART
//           uint8 *buffer:待发送数组
//           uint16 DataLength:待发送数组的长度
//返回值  ：无
//作者/时间/版本号:     /20140402/v1.0
//修改记录: 无
//***************************************************
void UartTxFrame(uint8 *buffer,uint16 DataLength)
{
		HAL_UART_Transmit(&huart,buffer,DataLength,100);
}

/**************************************************
功能:不需调用.USART1接收中断
输入:无
输出:无
***************************************************/
void USART1_IRQHandler(void)
{
		HAL_UART_IRQHandler(&huart);
}




