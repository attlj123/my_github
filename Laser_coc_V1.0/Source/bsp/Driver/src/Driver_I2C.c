#include "Driver_I2C.h"

//定义超时时间
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
//==========================================================
//功能:I2C初始化
//输入:I2Cx 取值I2C1, I2C2
//     OwnAddr 自己的地址，取值0~63
//     ClockSpeed I2C时钟 取值<400 000 400KHz
//输出:无
//==========================================================
void Driver_I2C_Init(I2C_TypeStruct I2CPara)
{   
    if(I2CPara.Name==I2C1)
    {
		HAL_I2C_DeInit(&hi2c1);

		hi2c1.Instance = I2CPara.Name;
		hi2c1.Init.Timing = I2CPara.ClockValue;
		hi2c1.Init.OwnAddress1 = I2CPara.Address;
		hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
		hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		hi2c1.Init.OwnAddress2 = 0;
		hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
		hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
		if (HAL_I2C_Init(&hi2c1) != HAL_OK)
		{
			I2CPara.ErrorMessage=I2C_ERR_INIT;
		}
		/** Configure Analogue filter
		*/
		if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
		{
			I2CPara.ErrorMessage=I2C_ERR_INIT;
		}
		/** Configure Digital filter
		*/
		if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
		{
			I2CPara.ErrorMessage=I2C_ERR_INIT;
		}
    }
	else if(I2CPara.Name==I2C2)
    {
		HAL_I2C_DeInit(&hi2c2);

		hi2c2.Instance = I2CPara.Name;
		hi2c2.Init.Timing = I2CPara.ClockValue;
		hi2c2.Init.OwnAddress1 = I2CPara.Address;
		hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
		hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		hi2c2.Init.OwnAddress2 = 0;
		hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
		hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
		if (HAL_I2C_Init(&hi2c2) != HAL_OK)
		{
			I2CPara.ErrorMessage=I2C_ERR_INIT;
		}
		/** Configure Analogue filter
		*/
		if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
		{
			I2CPara.ErrorMessage=I2C_ERR_INIT;
		}
		/** Configure Digital filter
		*/
		if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
		{
			I2CPara.ErrorMessage=I2C_ERR_INIT;
		}
    }
	else if(I2CPara.Name==I2C3)
    {
		HAL_I2C_DeInit(&hi2c3);

		hi2c3.Instance = I2CPara.Name;
		hi2c3.Init.Timing = I2CPara.ClockValue;
		hi2c3.Init.OwnAddress1 = I2CPara.Address;
		hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
		hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		hi2c3.Init.OwnAddress2 = 0;
		hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
		hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
		if (HAL_I2C_Init(&hi2c3) != HAL_OK)
		{
			I2CPara.ErrorMessage=I2C_ERR_INIT;
		}
		/** Configure Analogue filter
		*/
		if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
		{
			I2CPara.ErrorMessage=I2C_ERR_INIT;
		}
		/** Configure Digital filter
		*/
		if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
		{
			I2CPara.ErrorMessage=I2C_ERR_INIT;
		}
    }
		
	I2CPara.ErrorMessage=I2C_NO_ERR;
}


void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	
	if(i2cHandle->Instance==I2C1)
	{
		/* USER CODE BEGIN I2C1_MspInit 0 */
	 
		/* USER CODE END I2C1_MspInit 0 */
	 
			__HAL_RCC_GPIOB_CLK_ENABLE();
			/**I2C1 GPIO Configuration
			PB6     ------> I2C1_SCL
			PB7     ------> I2C1_SDA
			*/
			GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	 
			/* I2C1 clock enable */
			__HAL_RCC_I2C1_CLK_ENABLE();
	 
			/* I2C1 interrupt Init */
			HAL_NVIC_SetPriority(I2C1_ER_IRQn, 3, 0);
			HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
		/* USER CODE BEGIN I2C1_MspInit 1 */
	 
		/* USER CODE END I2C1_MspInit 1 */
	}
	else   if(i2cHandle->Instance==I2C2)
	{
		/* USER CODE BEGIN I2C2_MspInit 0 */
	 
		/* USER CODE END I2C2_MspInit 0 */
	 
			__HAL_RCC_GPIOB_CLK_ENABLE();
			/**I2C2 GPIO Configuration
			PB10     ------> I2C2_SCL
			PB11     ------> I2C2_SDA
			*/
			GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	 
			/* I2C2 clock enable */
			__HAL_RCC_I2C2_CLK_ENABLE();
	 
			/* I2C1 interrupt Init */
			HAL_NVIC_SetPriority(I2C2_ER_IRQn, 4, 0);
			HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
		/* USER CODE BEGIN I2C2_MspInit 1 */
	 
		/* USER CODE END I2C2_MspInit 1 */
	}
	else if(i2cHandle->Instance==I2C3)
	{
	/* USER CODE BEGIN I2C3_MspInit 0 */
 
	/* USER CODE END I2C3_MspInit 0 */
 
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**I2C3 GPIO Configuration
		PA8     ------> I2C3_SCL
		PC9     ------> I2C3_SDA
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		GPIO_InitStruct.Pin = GPIO_PIN_9;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
 
		/* I2C3 clock enable */
		__HAL_RCC_I2C3_CLK_ENABLE();
 
		/* I2C3 interrupt Init */
		HAL_NVIC_SetPriority(I2C3_ER_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);
	/* USER CODE BEGIN I2C3_MspInit 1 */
 
	/* USER CODE END I2C3_MspInit 1 */
	}
}

//===========================================================
//功能:主机向从机发送数据
//输入:SlaveAdd 从机地址
//     Command 发送第一个数据,模块EEPROM地址
//     data 待发送数据
//输出:错误信息
//===========================================================
uint8_t Driver_I2C_MasterWriteData(I2C_TypeStruct I2CPara,uint8_t SlaveAddr,uint8_t *Data,uint16_t ReadLength)
{
    if(I2CPara.Name==I2C1)
	{
		if(HAL_I2C_Master_Transmit(&hi2c1,SlaveAddr,Data,ReadLength,1)==HAL_OK)
		{

				I2CPara.ErrorMessage = I2C_NO_ERR;
		}
		else
		{
				Driver_I2C_Init(I2CPara);
				I2CPara.ErrorMessage = I2C_ERR_TXCHAR_TIMEOUT;
		}
	}
	else if(I2CPara.Name==I2C2)
	{
		if(HAL_I2C_Master_Transmit(&hi2c2,SlaveAddr,Data,ReadLength,1)==HAL_OK)
		{
				I2CPara.ErrorMessage = I2C_NO_ERR;
		}
		else
		{
				Driver_I2C_Init(I2CPara);
				I2CPara.ErrorMessage = I2C_ERR_TXCHAR_TIMEOUT;
		}
	}
	else if(I2CPara.Name==I2C3)
	{
		if(HAL_I2C_Master_Transmit(&hi2c3,SlaveAddr,Data,ReadLength,1)==HAL_OK)
		{
				I2CPara.ErrorMessage = I2C_NO_ERR;
		}
		else
		{
				Driver_I2C_Init(I2CPara);
				I2CPara.ErrorMessage = I2C_ERR_TXCHAR_TIMEOUT;
		}
	}

    return I2CPara.ErrorMessage;
}


//===========================================================
//功能:主机向从机发送数据
//输入:SlaveAdd 从机地址
//     Command 发送第一个数据,模块EEPROM地址
//     data 待发送数据
//输出:错误信息
//===========================================================
uint8_t Driver_I2C_MasterWriteToSlave(I2C_TypeStruct I2CPara,uint16_t SlaveAddr,uint16_t MemAddr,uint16_t MemLenght,uint8_t *Data,uint16_t ReadLength)
{
    if(I2CPara.Name==I2C1)
	{
		if(HAL_I2C_Mem_Write(&hi2c1,SlaveAddr,MemAddr,MemLenght,Data,ReadLength,1)==HAL_OK)
		{
				I2CPara.ErrorMessage = I2C_NO_ERR;
		}
		else
		{
				Driver_I2C_Init(I2CPara);
				I2CPara.ErrorMessage = I2C_ERR_TXCHAR_TIMEOUT;
		}
	}
	else if(I2CPara.Name==I2C2)
	{
		if(HAL_I2C_Mem_Write(&hi2c2,SlaveAddr,MemAddr,MemLenght,Data,ReadLength,1)==HAL_OK)
		{
				I2CPara.ErrorMessage = I2C_NO_ERR;
		}
		else
		{
				Driver_I2C_Init(I2CPara);
				I2CPara.ErrorMessage = I2C_ERR_TXCHAR_TIMEOUT;
		}
	}
	else if(I2CPara.Name==I2C3)
	{
		if(HAL_I2C_Mem_Write(&hi2c3,SlaveAddr,MemAddr,MemLenght,Data,ReadLength,1)==HAL_OK)
		{
				I2CPara.ErrorMessage = I2C_NO_ERR;
		}
		else
		{
				Driver_I2C_Init(I2CPara);
				I2CPara.ErrorMessage = I2C_ERR_TXCHAR_TIMEOUT;
		}
	}

    return I2CPara.ErrorMessage;
}

//===========================================================
//功能:主机向从机发送数据
//输入:SlaveAdd 从机地址
//     Command 发送第一个数据,模块EEPROM地址
//     data 待发送数据
//输出:错误信息
//===========================================================
uint8_t Driver_I2C_MasterWriteReadFromSlave(I2C_TypeStruct I2CPara,uint16_t SlaveAddr,uint16_t MemAddr,uint16_t MemLenght,uint8_t *Data,uint16_t ReadLength)
{

    if(I2CPara.Name==I2C1)
	{
		if(HAL_I2C_Mem_Read(&hi2c1,SlaveAddr,MemAddr,MemLenght,Data,ReadLength,1)==HAL_OK)
		{
				I2CPara.ErrorMessage = I2C_NO_ERR;
		}
		else
		{
				Driver_I2C_Init(I2CPara);
				I2CPara.ErrorMessage = I2C_ERR_RXCHAR_TIMEOUT;
		}
	}
	else if(I2CPara.Name==I2C2)
	{
		if(HAL_I2C_Mem_Read(&hi2c2,SlaveAddr,MemAddr,MemLenght,Data,ReadLength,1)==HAL_OK)
		{
				I2CPara.ErrorMessage = I2C_NO_ERR;
		}
		else
		{
				Driver_I2C_Init(I2CPara);
				I2CPara.ErrorMessage = I2C_ERR_RXCHAR_TIMEOUT;
		}
	}
	else if(I2CPara.Name==I2C3)
	{
		if(HAL_I2C_Mem_Read(&hi2c3,SlaveAddr,MemAddr,MemLenght,Data,ReadLength,1)==HAL_OK)
		{
				I2CPara.ErrorMessage = I2C_NO_ERR;
		}
		else
		{
				Driver_I2C_Init(I2CPara);
				I2CPara.ErrorMessage = I2C_ERR_RXCHAR_TIMEOUT;
		}
	}

    return I2CPara.ErrorMessage;

}

