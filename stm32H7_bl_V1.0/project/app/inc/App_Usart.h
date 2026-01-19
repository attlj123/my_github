#ifndef __APP_USART_H__
#define __APP_USART_H__


#include "TypesDef.h"
#include "Driver_Uart.h"

#include "stm32h7xx_hal.h"

//RS485通讯地址
#define 	A0_GPIO_Port		GPIOC
#define 	A0_Pin				GPIO_PIN_12
#define   	A1_GPIO_Port		GPIOD
#define 	A1_Pin				GPIO_PIN_0
#define 	A2_GPIO_Port		GPIOD
#define 	A2_Pin				GPIO_PIN_1
#define   	A3_GPIO_Port		GPIOD
#define 	A3_Pin				GPIO_PIN_2
#define 	A4_GPIO_Port		GPIOD
#define 	A4_Pin				GPIO_PIN_3
#define 	A5_GPIO_Port		GPIOD
#define 	A5_Pin				GPIO_PIN_4
#define 	A6_GPIO_Port		GPIOD
#define 	A6_Pin				GPIO_PIN_5
#define 	A7_GPIO_Port		GPIOD
#define 	A7_Pin				GPIO_PIN_6

//RS485控制读写
#define 	RS485_CONTROL_PORT		GPIOC
#define 	RS485_CONTROL_PIN		GPIO_PIN_8
#define		RS485_WRITE_HANDLE		HAL_GPIO_WritePin(RS485_CONTROL_PORT,RS485_CONTROL_PIN,GPIO_PIN_SET);						//写操作
#define		RS485_READ_HANDLE		HAL_GPIO_WritePin(RS485_CONTROL_PORT,RS485_CONTROL_PIN,GPIO_PIN_RESET);						//读操作

#define USARTTX_MAXLENGTH 50
#define USARTRX_MAXLENGTH 200

//接收状态预定义
#define GET_FRAME_NOT_OVER   0x00
#define GET_FRAME_OVER       0x01

typedef struct USARTSTRUCT
{
    uint32 UsartBaudspeed;
    uint8 UsartPrioty;
    uint8 UsartLink;
    uint8 UsartTxBuffer[USARTTX_MAXLENGTH];
    uint8 UsartRxBuffer[USARTRX_MAXLENGTH];
    uint16 UsartRxFrameLength;
    uint16 UsartRxDataLength;
}USARTSTRUCT;


/**************************************************
功能:需要调用.初始化USART
输入:USARTx:串口号
     baudspeed:串口波特率
     preInterruptPrioty:串口接收中断先占优先级
输出:无
***************************************************/
void App_Usart_Init(uint32 baudspeed,uint8 preInterruptPrioty);


/**************************************************
功能:可以调用.USART发送一个字节
输入:uint8 Data 待发送数据8bits
输出:无
***************************************************/
void App_Usart_SentChar(uint8 Data);


/**************************************************
功能:可以调用.USART发送一串数据
输入:uint8 *Data 待发送数据首地址
     uint16 DataLength 待发送数据长度
输出:无
***************************************************/
void App_Usart_TxFrame(uint8 *Data,uint16 DataLength);


/**************************************************
功能:串口中断数据处理
输入:IrqData 接收中断数据
输出:无
***************************************************/
void  App_Usart_IrqDataHandle(uint8 IrqData);


/**************************************************
功能:查询数据接收是否完成
输入:无
输出:接收状态
***************************************************/
uint8 App_Usart_GetFrame(void);


/**************************************************
功能:数据清零
输入:无
输出:无
***************************************************/
void App_Usart_ClearFrame(void);


/**************************************************
功能:串口复位 防止APP函数没有中断服务程序
输入:无
输出:无
***************************************************/
void App_Usart_Reset(void);


extern USARTSTRUCT g_UsartStruct;


#endif

