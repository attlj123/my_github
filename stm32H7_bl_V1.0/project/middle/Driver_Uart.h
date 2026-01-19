#ifndef _Driver_Uart_
#define _Driver_Uart_

#include "TypesDef.h"
#include "stm32h7xx_hal.h"

//***************************************************
//函数名  :  UartInitial
//功能说明： 初始化通信串口
//输入说明： USART_TypeDef* USARTx:具体的COM口
//           uint32 baudspeed:串口波特率
//           uint8 preInterruptPrioty:串口接收中断先占优先级
//返回值  ：无
//作者/时间/版本号:     /20140402/v1.0
//***************************************************
void UartInitial(uint32 baudspeed, uint8 preInterruptPrioty);

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
void UartTxFrame(uint8 *buffer,uint16 DataLength);


#endif
