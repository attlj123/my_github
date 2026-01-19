#include "App_Usart.h"
#include "Driver_Sleep.h"
#include "App_Link.h"


USARTSTRUCT g_UsartStruct;
extern uint8_t UsartRXTemp;

/**************************************************
功能:需要调用.初始化USART
输入:USARTx:串口号
     baudspeed:串口波特率
     preInterruptPrioty:串口接收中断先占优先级
输出:无
***************************************************/
void App_Usart_Init(uint32 baudspeed,uint8 preInterruptPrioty)
{
    g_UsartStruct.UsartBaudspeed=baudspeed;
    g_UsartStruct.UsartPrioty=preInterruptPrioty;
    g_UsartStruct.UsartLink=0;
    g_UsartStruct.UsartRxFrameLength=0;
    g_UsartStruct.UsartRxDataLength=0;
    UartInitial(g_UsartStruct.UsartBaudspeed,g_UsartStruct.UsartPrioty);
}



/**************************************************
功能:可以调用.USART发送一串数据
输入:uint8 *Data 待发送数据首地址
     uint16 DataLength 待发送数据长度
输出:无
***************************************************/
void App_Usart_TxFrame(uint8 *Data,uint16 DataLength)
{
	uint32_t i = 0;

	RS485_WRITE_HANDLE
	
	for(i=0;i<3000;i++)
		__nop();
	
	UartTxFrame(Data,DataLength);
	
	for(i=0;i<3000;i++)
		__nop();
	
	RS485_READ_HANDLE 
}


/**************************************************
功能:串口中断数据处理
输入:IrqData 接收中断数据
输出:无
***************************************************/
void  App_Usart_IrqDataHandle(uint8 IrqData)
{
    if(0==g_UsartStruct.UsartLink)
    {
        if( (0==g_UsartStruct.UsartRxFrameLength)&&(IrqData==FRAME_HEAD))
        {//接收帧头
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=IrqData;
            g_UsartStruct.UsartRxFrameLength++;
        }
        else if( (g_UsartStruct.UsartRxFrameLength>=1)&&(g_UsartStruct.UsartRxFrameLength<=2) )
        {//kind和index直接接收
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=IrqData;
            g_UsartStruct.UsartRxFrameLength++;
        }
        else if( g_UsartStruct.UsartRxFrameLength==3 )
        {//接收数据长度
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=IrqData;
            g_UsartStruct.UsartRxDataLength=IrqData;
            g_UsartStruct.UsartRxFrameLength++;
        }
        else if( (g_UsartStruct.UsartRxFrameLength>=4)&&(g_UsartStruct.UsartRxFrameLength<=g_UsartStruct.UsartRxDataLength+4-1) )
        {//接收数据
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=IrqData;
            g_UsartStruct.UsartRxFrameLength++;
        }
        else if( g_UsartStruct.UsartRxFrameLength==g_UsartStruct.UsartRxDataLength+4 )
        {//接收校验位
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=IrqData;
            g_UsartStruct.UsartRxFrameLength++;
        }
        else if((g_UsartStruct.UsartRxFrameLength==g_UsartStruct.UsartRxDataLength+5)&&(IrqData==FRAME_END))
        {//接收帧尾
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=IrqData;
            
            g_UsartStruct.UsartRxFrameLength=g_UsartStruct.UsartRxFrameLength+1;
            g_UsartStruct.UsartRxDataLength=0;
            g_UsartStruct.UsartLink=1;
        }
        else 
        {
            g_UsartStruct.UsartRxDataLength=0;
            g_UsartStruct.UsartRxFrameLength=0;
            g_UsartStruct.UsartLink=0;
        }
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) 
  {
    if(0==g_UsartStruct.UsartLink)
    {
        if( (0==g_UsartStruct.UsartRxFrameLength)&&(UsartRXTemp==FRAME_HEAD))
        {//接收帧头
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=UsartRXTemp;
            g_UsartStruct.UsartRxFrameLength++;
        }
        else if( (g_UsartStruct.UsartRxFrameLength>=1)&&(g_UsartStruct.UsartRxFrameLength<=2) )
        {//kind和index直接接收
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=UsartRXTemp;
            g_UsartStruct.UsartRxFrameLength++;
        }
        else if( g_UsartStruct.UsartRxFrameLength==3 )
        {//接收数据长度
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=UsartRXTemp;
            g_UsartStruct.UsartRxDataLength=UsartRXTemp;
            g_UsartStruct.UsartRxFrameLength++;
        }
        else if( (g_UsartStruct.UsartRxFrameLength>=4)&&(g_UsartStruct.UsartRxFrameLength<=g_UsartStruct.UsartRxDataLength+4-1) )
        {//接收数据
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=UsartRXTemp;
            g_UsartStruct.UsartRxFrameLength++;
        }
        else if( g_UsartStruct.UsartRxFrameLength==g_UsartStruct.UsartRxDataLength+4 )
        {//接收校验位
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=UsartRXTemp;
            g_UsartStruct.UsartRxFrameLength++;
        }
        else if((g_UsartStruct.UsartRxFrameLength==g_UsartStruct.UsartRxDataLength+5)&&(UsartRXTemp==FRAME_END))
        {//接收帧尾
            g_UsartStruct.UsartRxBuffer[g_UsartStruct.UsartRxFrameLength]=UsartRXTemp;
            
            g_UsartStruct.UsartRxFrameLength=g_UsartStruct.UsartRxFrameLength+1;
            g_UsartStruct.UsartRxDataLength=0;
            g_UsartStruct.UsartLink=1;
        }
        else 
        {
            g_UsartStruct.UsartRxDataLength=0;
            g_UsartStruct.UsartRxFrameLength=0;
            g_UsartStruct.UsartLink=0;
        }
    }
		
    HAL_UART_Receive_IT(huart, &UsartRXTemp, sizeof(UsartRXTemp)); 
  }
}

/**************************************************
功能:查询数据接收是否完成
输入:无
输出:接收状态
***************************************************/
uint8 App_Usart_GetFrame(void)
{
    if(0==g_UsartStruct.UsartLink)
        return GET_FRAME_NOT_OVER;
    else 
        return GET_FRAME_OVER;
}


/**************************************************
功能:数据清零
输入:无
输出:无
***************************************************/
void App_Usart_ClearFrame(void)
{
    g_UsartStruct.UsartRxFrameLength=0;
    
    g_UsartStruct.UsartLink=0;
}


/**************************************************
功能:串口复位 防止APP函数没有中断服务程序
输入:无
输出:无
***************************************************/
extern UART_HandleTypeDef huart;
void App_Usart_Reset(void)
{
    SleepUs(500);
	
	HAL_UART_MspDeInit(&huart);
//    usart_interrupt_disable(g_UsartStruct.UsartName, USART_INT_RBNE);
}

