#ifndef _ComHandle_
#define _ComHandle_

#include "CommandFrame.h"

//通讯基础信息
#define FRAME_START							0xDE
#define FRAME_END							0xED

//帧位数组成
#define FRAME_START_NUM						1
#define FRAME_ID_NUM						2
#define FRAME_LENGHT_HIGH_NUM				3
#define FRAME_LENGHT_LOW_NUM				4
#define FRAME_VERIFY_NUM			    	5
#define FRAME_END_NUM			        	6
	
#define RS485_QUEUE_LENGTH 					100
#define RS485_QUEUE_ITEM_SIZE 				sizeof(uint8_t)
	
#define TRANS_QUEUE_LENGTH 					100
#define TRANS_QUEUE_ITEM_SIZE 				sizeof(uint8_t)

//硬件地址
#define 	RS485_ADD0_PORT				GPIOC
#define 	RS485_ADD0_PIN				GPIO_PIN_12
#define   	RS485_ADD1_PORT				GPIOD
#define 	RS485_ADD1_PIN				GPIO_PIN_0
#define 	RS485_ADD2_PORT				GPIOD
#define 	RS485_ADD2_PIN				GPIO_PIN_1
#define   	RS485_ADD3_PORT				GPIOD
#define 	RS485_ADD3_PIN				GPIO_PIN_2
#define 	RS485_ADD4_PORT				GPIOD
#define 	RS485_ADD4_PIN				GPIO_PIN_3
#define 	RS485_ADD5_PORT				GPIOD
#define 	RS485_ADD5_PIN				GPIO_PIN_4
#define 	RS485_ADD6_PORT				GPIOD
#define 	RS485_ADD6_PIN				GPIO_PIN_5
#define 	RS485_ADD7_PORT				GPIOD
#define 	RS485_ADD7_PIN				GPIO_PIN_6


//RS485控制读写
#define 	RS485_CONTROL_PORT			GPIOC
#define 	RS485_CONTROL_PIN			GPIO_PIN_8
#define		RS485_WRITE_HANDLE			HAL_GPIO_WritePin(RS485_CONTROL_PORT,RS485_CONTROL_PIN,GPIO_PIN_SET);							//写操作
#define		RS485_READ_HANDLE		  	HAL_GPIO_WritePin(RS485_CONTROL_PORT,RS485_CONTROL_PIN,GPIO_PIN_RESET);						//读操作

#define 	TRANS_CONTROL_PORT			GPIOE
#define 	TRANS_CONTROL_PIN			GPIO_PIN_8
#define		TRANS_WRITE_HANDLE			HAL_GPIO_WritePin(TRANS_CONTROL_PORT,TRANS_CONTROL_PIN,GPIO_PIN_SET);							//写操作
#define		TRANS_READ_HANDLE		  	HAL_GPIO_WritePin(TRANS_CONTROL_PORT,TRANS_CONTROL_PIN,GPIO_PIN_RESET);						//读操作

//传输帧状态
typedef enum
{	
	FRAME_START_STATUS = 1,					//接受帧开始
	FRAME_ID_STATUS = 2,					//接受帧地址
	FRAME_LENGHT_STATUS = 3,				//接受帧长度
	FRAME_VERIFY_STATUS = 4,				//接受帧校验
	FRAME_END_STATUS = 5,					//接受帧结束
	FRAME_DATA_STATUS = 6					//接受帧数据
}RECEIVE_FRAME_STATUS;

typedef struct
{
	uint16_t Rs485Add;   						 //485器件地址
	RECEIVE_FRAME_STATUS RS485ReceiveStatus;    //包头所在的状态
	uint16_t DataLenght;
	uint16_t CurrentLenght;   					//包头后的数据包长度
	uint16_t Verify;   							//校验和
	uint16_t FrameNum;     						//包头帧个数
}RS485_TypeStruct;

//转发接口参数
typedef struct
{
	uint8_t Com_Device;
	uint8_t Ascii_Hex;
	uint8_t Hex_Lenght;
	uint8_t RxIndex;
}ComTrans_TypeStruct;

void RS485SendFrame(uint32_t Lenght,char* data);
void TransSendFrame(uint32_t Lenght,char* data);
void ComHandleInit(uint8_t addr);

#endif

