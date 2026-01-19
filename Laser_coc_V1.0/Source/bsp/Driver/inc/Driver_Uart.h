#ifndef _Driver_Uart_
#define _Driver_Uart_

#include <stdint.h>
#include <stddef.h>
#include "stm32h7xx_hal.h"

//定义中断的开启
#define UART_RX_INTERRUPT_EN  0x01
#define UART_RX_INTERRUPT_DIS 0x00


//UART配置结构体
typedef struct UartInfomation
{
	uint32_t BandSpeed;            //波特率
	uint8_t  RxInterruptEnable;    //是否开启中断
	uint8_t  RxPreInterruptPrioty; //抢占优先级
	uint8_t  RxSubInterruptPrioty; //非抢占优先级
} UartConfigInfo;

//***************************************************
//函数名  :  McUartInitial
//功能说明： 初始化通信串口
//输入说明： Config:配置串口的相关信息
//           Com:具体的COM口
//***************************************************
void DriverUartInitial(UartConfigInfo Config,USART_TypeDef *Com);
void DriverTranInitial(UartConfigInfo Config,USART_TypeDef *Com);
//***************************************************
//函数名  :  UartTxChar
//功能说明： UART发送一个字符
//输入说明： Com：哪一个UART
//           Txchar:待发送的字符
//***************************************************
void UartTxFrame(uint8_t *buffer,uint16_t DataLength);
void TranTxFrame(uint8_t *buffer,uint16_t DataLength);
void TranRxLenght(uint16_t rxlenght);

#endif
