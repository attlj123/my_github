#ifndef _MessagePass_
#define _MessagePass_

#include "CommandFrame.h"

//通讯基础信息
#define MAX_COMMAND_NUM						100  			//最大的命令条数

#define RS485_DEVICE_INTERFACE				0x01
#define ETH_MQTT_INTERFACE                	0x02

#define ASCII_TRANS_REVEIVE					0x01
#define HEX_TRANS_REVEIVE            		0x02

//帧位数组成
#define STRING_START						2

#define MESSAGE_QUEUE_LENGTH 				1
#define MESSAGE_QUEUE_ITEM_SIZE 			sizeof(CommInterfaceInfo*)


void BuffClear(uint8_t *buffer,uint32_t length,uint8_t Data);
void MessagePassInit(void);

#endif

