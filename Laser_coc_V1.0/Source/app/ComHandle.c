#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "Driver_Uart.h"

#include "ComHandle.h"
#include "MessagePass.h"

#include "app_cfg.h"

//实例化485参数结构
RS485_TypeStruct g_s_Rs485ConfigInfo;	
ComTrans_TypeStruct g_s_ComTransimt;

QueueHandle_t Rs485Queue = NULL;
QueueHandle_t TransQueue = NULL;

extern uint8_t g_HardAddress;
extern uint8_t UsartRXTemp;
extern uint8_t TranRXTemp;
extern CommInterfaceInfo RS485Interface;
extern QueueHandle_t MessageQueue;

//***************************************************
//函数名  :  Rs485SendMessage
//功能说明： RS485发送信息
//输入说明： void
//***************************************************
void UartSendMessage (uint8_t *Message)
{
	xQueueSendToBack(MessageQueue,&Message,pdMS_TO_TICKS(0));
}


//***************************************************
//函数名  : RS485总线串口发送数据
//输入说明: 无
//返回值：无
//*************************************************
void RS485SendFrame(uint32_t Lenght,char* data)
{
	uint32_t i = 0;

	//先将RS485置于发送状态
	RS485_WRITE_HANDLE
	
	for(i=0;i<3000;i++)
		__nop();
	
	UartTxFrame((uint8_t *)data,(uint16_t)Lenght);
	
	for(i=0;i<3000;i++)
		__nop();

	RS485_READ_HANDLE
}


//***************************************************
//函数名  : RS485总线串口发送数据
//输入说明: 无
//返回值：无
//*************************************************
void TransSendFrame(uint32_t Lenght,char* data)
{
	uint32_t i = 0;

	//先将RS485置于发送状态
	TRANS_WRITE_HANDLE
	
	for(i=0;i<3000;i++)
		__nop();
	
	TranTxFrame((uint8_t *)data,(uint16_t)Lenght);
	
	for(i=0;i<3000;i++)
		__nop();

	TRANS_READ_HANDLE
}




//**************************************************
//函数名  :  MessageAnalyseEntry
//功能说明： 通讯传递初始化
//					串口 CAN 配置 以及解析平台配置
//输入说明： void
//***************************************************
void Rs485ReceiveEntry(void *p_arg)
{
	uint8_t Uart485Data;
	QueueHandle_t xQueue;

	xQueue = (QueueHandle_t)p_arg;
	
	for(;;)
	{
		if(pdFALSE != xQueueReceive(xQueue,&Uart485Data,portMAX_DELAY))
		{
			if((Uart485Data == FRAME_START)&&(g_s_Rs485ConfigInfo.RS485ReceiveStatus == FRAME_START_STATUS))									//开始帧
			{
				g_s_Rs485ConfigInfo.FrameNum++;																		//帧位数
				g_s_Rs485ConfigInfo.RS485ReceiveStatus = FRAME_ID_STATUS;
				g_s_Rs485ConfigInfo.Verify = g_s_Rs485ConfigInfo.Verify + Uart485Data;											//校验												
			}
			else if((Uart485Data == (uint8_t)g_s_Rs485ConfigInfo.Rs485Add)&&(g_s_Rs485ConfigInfo.RS485ReceiveStatus == FRAME_ID_STATUS))				//地址帧
			{
				g_s_Rs485ConfigInfo.FrameNum++;																		//帧位数
				g_s_Rs485ConfigInfo.RS485ReceiveStatus = FRAME_LENGHT_STATUS;
				g_s_Rs485ConfigInfo.Verify = g_s_Rs485ConfigInfo.Verify + Uart485Data;														
			}
			else if(g_s_Rs485ConfigInfo.RS485ReceiveStatus == FRAME_LENGHT_STATUS)						//数据量帧
			{
				g_s_Rs485ConfigInfo.FrameNum++;																		//帧位数
				g_s_Rs485ConfigInfo.Verify = g_s_Rs485ConfigInfo.Verify + Uart485Data;	
				if(FRAME_LENGHT_HIGH_NUM == g_s_Rs485ConfigInfo.FrameNum)
					g_s_Rs485ConfigInfo.DataLenght = Uart485Data;
				else if(FRAME_LENGHT_LOW_NUM == g_s_Rs485ConfigInfo.FrameNum)
				{
					g_s_Rs485ConfigInfo.DataLenght = g_s_Rs485ConfigInfo.DataLenght<<8|Uart485Data;
					g_s_Rs485ConfigInfo.RS485ReceiveStatus = FRAME_VERIFY_STATUS;
				}
			}
			else if(g_s_Rs485ConfigInfo.RS485ReceiveStatus == FRAME_VERIFY_STATUS)	
			{
				if((uint8_t)g_s_Rs485ConfigInfo.Verify == Uart485Data)
					g_s_Rs485ConfigInfo.RS485ReceiveStatus = FRAME_END_STATUS;
				else 
				{
					g_s_Rs485ConfigInfo.RS485ReceiveStatus = FRAME_START_STATUS;
					g_s_Rs485ConfigInfo.FrameNum = 0;
					g_s_Rs485ConfigInfo.Verify = 0;
					g_s_Rs485ConfigInfo.DataLenght = 0;
					g_s_Rs485ConfigInfo.CurrentLenght = 0;
				}
			}
			else if((g_s_Rs485ConfigInfo.RS485ReceiveStatus == FRAME_END_STATUS)&&(Uart485Data == FRAME_END))
					g_s_Rs485ConfigInfo.RS485ReceiveStatus = FRAME_DATA_STATUS;
			else if(g_s_Rs485ConfigInfo.RS485ReceiveStatus == FRAME_DATA_STATUS)
			{
				CommandByteReceiveProcess(&RS485Interface,Uart485Data,UartSendMessage);
				g_s_Rs485ConfigInfo.CurrentLenght++;
				if(g_s_Rs485ConfigInfo.CurrentLenght == g_s_Rs485ConfigInfo.DataLenght)
				{
						g_s_Rs485ConfigInfo.RS485ReceiveStatus = FRAME_START_STATUS;
						g_s_Rs485ConfigInfo.FrameNum = 0;
						g_s_Rs485ConfigInfo.Verify = 0;
						g_s_Rs485ConfigInfo.DataLenght = 0;
						g_s_Rs485ConfigInfo.CurrentLenght = 0;
				}
			}
			else 
			{
				g_s_Rs485ConfigInfo.RS485ReceiveStatus = FRAME_START_STATUS;
				g_s_Rs485ConfigInfo.FrameNum = 0;
				g_s_Rs485ConfigInfo.Verify = 0;
				g_s_Rs485ConfigInfo.DataLenght = 0;
				g_s_Rs485ConfigInfo.CurrentLenght = 0;
			}
		}
	}
}

char TranRxBuffer[30] = {0};
char TranHexString[30] = {0};
char TranHexLeng = 0;
char hex_str[4] = {0};

extern QueueHandle_t PublishQueue;
//**************************************************
//函数名  :  MessageAnalyseEntry
//功能说明： 通讯传递初始化
//					串口 CAN 配置 以及解析平台配置
//输入说明： void
//***************************************************
void TransReceiveEntry(void *p_arg)
{
	uint8_t TransmitData;
	char* p_tx_buff = NULL;
	QueueHandle_t xQueue;
	
	xQueue = (QueueHandle_t)p_arg;
	
	for(;;)
	{
		if(pdFALSE != xQueueReceive(xQueue,&TransmitData,portMAX_DELAY))
		{
			if(g_s_ComTransimt.Ascii_Hex==ASCII_TRANS_REVEIVE)
			{
				TranRxBuffer[g_s_ComTransimt.RxIndex] = TransmitData;
				
				// 处理接收到的数据，例如检查帧结束或长度字段
				if(TranRxBuffer[g_s_ComTransimt.RxIndex] == '\r' || TranRxBuffer[g_s_ComTransimt.RxIndex] == '\n' || g_s_ComTransimt.RxIndex >= sizeof(TranRxBuffer))
				{
					// 处理完整的数据包
					p_tx_buff = TranRxBuffer;
				
					if(ETH_MQTT_INTERFACE==g_s_ComTransimt.Com_Device)
						xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
					else if(RS485_DEVICE_INTERFACE==g_s_ComTransimt.Com_Device)
						RS485SendFrame(strlen((char*)TranRxBuffer),TranRxBuffer);
					
					g_s_ComTransimt.RxIndex = 0; // 重置索引以接收新的数据包
				} 
				else 
				{
					g_s_ComTransimt.RxIndex++;
				}
			
			}
			else if(g_s_ComTransimt.Ascii_Hex==HEX_TRANS_REVEIVE)
			{
				g_s_ComTransimt.RxIndex++;
				
				sprintf(hex_str, "%02X",TransmitData);
				strcat(TranHexString,hex_str);
				
				if( g_s_ComTransimt.RxIndex >= g_s_ComTransimt.Hex_Lenght)
				{
					// 处理完整的数据包
					p_tx_buff = TranHexString;
				
					if(ETH_MQTT_INTERFACE==g_s_ComTransimt.Com_Device)
						xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
					else if(RS485_DEVICE_INTERFACE==g_s_ComTransimt.Com_Device)
						RS485SendFrame(strlen((char*)TranHexString),TranHexString);
					
					g_s_ComTransimt.RxIndex = 0; // 重置索引以接收新的数据包
				}			
			}
		}
	}
}

//***************************************************
//函数名  : RS485控制I初始化
//输入说明: 无
//返回值：无
//*************************************************
void RS485ControlInit(RS485_TypeStruct* RS485Part,uint16_t addr)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	//RS485控制输出
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Pin = RS485_CONTROL_PIN;
	HAL_GPIO_Init(RS485_CONTROL_PORT, &GPIO_InitStruct);
	RS485_READ_HANDLE

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Pin = TRANS_CONTROL_PIN;
	HAL_GPIO_Init(TRANS_CONTROL_PORT, &GPIO_InitStruct);
	TRANS_READ_HANDLE	
	
	RS485Part->RS485ReceiveStatus = FRAME_START_STATUS;
	RS485Part->FrameNum = 0;
	RS485Part->Verify = 0;
	RS485Part->DataLenght = 0;
	RS485Part->CurrentLenght = 0;
	RS485Part->Rs485Add = addr;
}


//***************************************************
//函数名  :  MessagePassInit
//功能说明： 通讯传递初始化
//					串口 CAN 配置 以及解析平台配置
//输入说明： void
//***************************************************
void ComHandleInit(uint8_t addr)
{
	BaseType_t CreatStatus;
	UartConfigInfo Uart485Port;
	UartConfigInfo UartTranspond;

	//RS485初始化相关配置功能
	Uart485Port.BandSpeed = 115200;
	Uart485Port.RxInterruptEnable = UART_RX_INTERRUPT_EN;
	Uart485Port.RxPreInterruptPrioty = 6;
	Uart485Port.RxSubInterruptPrioty = 0;
	DriverUartInitial(Uart485Port,USART1);
	
	//转发串口
	UartTranspond.BandSpeed = 57600;
	UartTranspond.RxInterruptEnable = UART_RX_INTERRUPT_EN;
	UartTranspond.RxPreInterruptPrioty = 7;
	UartTranspond.RxSubInterruptPrioty = 0;
	DriverTranInitial(UartTranspond,USART3);

	//RS485控制、地址初始化
	RS485ControlInit(&g_s_Rs485ConfigInfo,addr);

	//创建任务堆栈
	Rs485Queue = xQueueCreate(RS485_QUEUE_LENGTH,RS485_QUEUE_ITEM_SIZE);
	if(Rs485Queue == NULL)
		return ;
						 
	CreatStatus = xTaskCreate(Rs485ReceiveEntry,"Rs485Receive",RS485_RECEIVE_SIZE,(void*)Rs485Queue,RS485_RECEIVE_PRIO,NULL);
	
	if(CreatStatus != pdPASS)
		return ;
	
	//通讯转发功能
	TransQueue = xQueueCreate(TRANS_QUEUE_LENGTH,TRANS_QUEUE_ITEM_SIZE);
	if(TransQueue == NULL)
		return ;
						 
	CreatStatus = xTaskCreate(TransReceiveEntry,"TransReceive",TRANS_RECEIVE_SIZE,(void*)TransQueue,TRANS_RECEIVE_PRIO,NULL);
	
	if(CreatStatus != pdPASS)
		return ;
	
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//	char* p_tx_buff = NULL;
	BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = pdFALSE;
	
	if (huart->Instance == USART1) 
	{		
		HAL_UART_Receive_IT(huart, &UsartRXTemp, sizeof(UsartRXTemp)); 
	
		xQueueSendToBackFromISR(Rs485Queue,&UsartRXTemp,&xHigherPriorityTaskWoken);
//		CommandByteReceiveProcess(&RS485Interface,UsartRXTemp,UartSendMessage);
		
		if( xHigherPriorityTaskWoken == pdTRUE )
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else if (huart->Instance == USART3) 
	{
		HAL_UART_Receive_IT(huart, &TranRXTemp, sizeof(TranRXTemp)); 
	
		xQueueSendToBackFromISR(TransQueue,&TranRXTemp,&xHigherPriorityTaskWoken);
//		CommandByteReceiveProcess(&RS485Interface,UsartRXTemp,UartSendMessage);
		
		if( xHigherPriorityTaskWoken == pdTRUE )
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
	
}


