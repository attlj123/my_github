#include "stm32h7xx_hal.h"

#include "SGM5348.h"

#include "FreeRTOS.h"
#include "semphr.h"

//extern SemaphoreHandle_t g_Sgm5348Mutex;

#define SPI_TIME_OUT1  40 
#define SPI_TIME_OUT  400
#define HARD_SPI

SPI_HandleTypeDef hspi1;

extern void Error_Handler(void);
//**************************************************
//函数：DriverI2S2Config
//功能：对SGM5348外部DAC的I2S2接口进行相关配置
//说明：
//**************************************************
void DriverSPIConfig(void)
{
	/* USER CODE BEGIN SPIl Init 0 */
	/* USER CODE END SPIl Init 0 */
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	
	if(HAL_SPI_Init(&hspi1)!= HAL_OK)
	{
		Error_Handler();
	}
}


void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* UART8 clock enable */
	__HAL_RCC_SPI1_CLK_ENABLE();

	__HAL_RCC_GPIOB_CLK_ENABLE();
	/**UART8 GPIO Configuration
	PB3     ------> SPI1 SCK
	PB5     ------> SPI1 MOSI
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
}


//**************************************************
//函数：DriverI2S2RegWrite
//功能：对SGM5348外部DAC的I2S2接口进行写入操作
//说明：
//**************************************************
void DriverSPIRegWrite(uint16_t byte)
{
    uint32_t StartTimeOut=0;
	
	#ifdef HARD_SPI		/* 硬件SPI */
	
		//等待互斥信号量
//		if( xSemaphoreTake(g_Sgm5348Mutex, 0 ) == pdPASS )
//		{
//			while(RESET == spi_i2s_flag_get(SPIx, SPI_FLAG_TBE));
			HAL_SPI_Transmit(&hspi1,(uint8_t*)&byte,1,2);
//			spi_i2s_data_transmit(SPIx,0xaaaa);
			for(StartTimeOut=0;StartTimeOut<SPI_TIME_OUT;StartTimeOut++)
			{
				__nop();
			}
			
//			xSemaphoreGive(g_Sgm5348Mutex);
//		}
		
  #endif	

  #ifdef SOFT_SPI		/* 软件SPI */	
	uint8_t i,j;
	uint16_t read_dat;
	uint8_t write_dat[2];
	
	gpio_bit_reset(GPIOx,GPIO_Pin);
	gpio_bit_set(GPIOB,GPIO_PIN_3);
	for(StartTimeOut=0;StartTimeOut<SPI_TIME_OUT;StartTimeOut++)
	{
		__nop();
	}
	
	write_dat[0] = byte>>8;
	write_dat[1] = byte;
//	write_dat[1] = byte>>8;
//	write_dat[0] = byte;
	for(j=0;j<2;j++)
	{
		for(i = 0; i < 8; i++)
		{
			if (write_dat[j] & 0x80)
			{
				gpio_bit_set(GPIOB,GPIO_PIN_5); //若最到位为高，则输出高
			}
			else
			{
				gpio_bit_reset(GPIOB,GPIO_PIN_5);//若最到位为低，则输出低
			}	
			write_dat[j] <<= 1;  // 低一位移位到最高位
//			DriverDelayBlockUs(1);
//			__nop();
//			__nop();
//			__nop();
			for(StartTimeOut=0;StartTimeOut<SPI_TIME_OUT1;StartTimeOut++)
			{
				__nop();
			}
			
			gpio_bit_reset(GPIOB,GPIO_PIN_3);  //拉低时钟
			
			//数据左移
			read_dat <<=1;
			
			if(gpio_input_bit_get(GPIOB,GPIO_PIN_4))
			{
				read_dat++;  //若从从机接收到高电平，数据自加一
			}
//			DriverDelayBlockUs(1);
//			__nop();
//			__nop();
//			__nop();
			for(StartTimeOut=0;StartTimeOut<SPI_TIME_OUT1;StartTimeOut++)
			{
				__nop();
			}
			gpio_bit_set(GPIOB,GPIO_PIN_3);
		}
}
	
	
		for(StartTimeOut=0;StartTimeOut<SPI_TIME_OUT;StartTimeOut++)
		{
			__nop();
		}
	
		gpio_bit_set(GPIOx,GPIO_Pin);
//	return(read_dat); //返回数据

#endif		
}

//**************************************************
//函数：DriverAD5318Init
//功能：对AD5318控制引脚进行配置
//说明：
//**************************************************
void DriverSGM5348Init(void)
{	
	//config chip
	DriverSPIRegWrite(0x9000);		//WTM model 写入寄存器改变输出
	DriverSPIRegWrite(0xE0FF);		//输出 100k对地 outputs
		
	//默认状态WRM,将通道寄存器数据清除
	DriverSPIRegWrite(0x0000);		//WRM model 写入寄存器清零输出
	DriverSPIRegWrite(0x1000);		//WRM model 写入寄存器清零输出
	DriverSPIRegWrite(0x2000);		//WRM model 写入寄存器清零输出
	DriverSPIRegWrite(0x3000);		//WRM model 写入寄存器清零输出
	DriverSPIRegWrite(0x4000);		//WRM model 写入寄存器清零输出
	DriverSPIRegWrite(0x5000);		//WRM model 写入寄存器清零输出
	DriverSPIRegWrite(0x6000);		//WRM model 写入寄存器清零输出
	DriverSPIRegWrite(0x7000);		//WRM model 写入寄存器清零输出
}


//**************************************************
//函数：DriverSGM5348SetDac
//功能：设置对应Chip,对应Channel控制,8 BIT DAC
//说明：
//**************************************************
void DriverSGM5348SetDac(uint8_t channel,uint16_t Dac_Value)
{
	switch(channel)
	{
		case 0:
			DriverSPIRegWrite(0x0000|Dac_Value);		//设置DAC
			break;
		case 1:
			DriverSPIRegWrite(0x1000|Dac_Value);		//设置DAC
			break;
		case 2:
			DriverSPIRegWrite(0x2000|Dac_Value);		//设置DAC
			break;
		case 3:
			DriverSPIRegWrite(0x3000|Dac_Value);		//设置DAC
			break;
		case 4:
			DriverSPIRegWrite(0x4000|Dac_Value);		//设置DAC
			break;
		case 5:
			DriverSPIRegWrite(0x5000|Dac_Value);		//设置DAC
			break;
		case 6:
			DriverSPIRegWrite(0x6000|Dac_Value);		//设置DAC
			break;
		case 7:
			DriverSPIRegWrite(0x7000|Dac_Value);		//设置DAC
			break;
		default:break;
	}
}

