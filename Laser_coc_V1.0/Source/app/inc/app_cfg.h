#ifndef _app_cfg_
#define _app_cfg_

#include "FreeRTOSConfig.h"

#define  RS485_RECEIVE_PRIO					configMAX_PRIORITIES-1					//通讯进程
#define  TRANS_RECEIVE_PRIO					configMAX_PRIORITIES-2					//通讯进程
#define  MSG_ANALYSE_PRIO					configMAX_PRIORITIES-3					//通讯进程
#define  SETUP_ALL_PRIO						configMAX_PRIORITIES-4			  	//全部家电优先级
#define  SCAN_ITH_PRIO						configMAX_PRIORITIES-5			  	//扫描VI优先级
#define  SAMP_DUT_PRIO						configMAX_PRIORITIES-6				//采样DUT进程
#define  SAMP_MONIT_PRIO					configMAX_PRIORITIES-7				//采样监控进程
#define  OTHER_ITEM_PRIO					configMAX_PRIORITIES-8				//其余功能
#define  LINK_LED_PRIO						configMAX_PRIORITIES-9			  	//错误优先级
#define  ERR_LED_PRIO						configMAX_PRIORITIES-10			  	//错误优先级
#define  RUN_LED_PRIO						configMAX_PRIORITIES-11			  	//通讯灯优先级
#define  FIRST_LED_PRIO						configMAX_PRIORITIES-12			  	//通讯灯优先级
#define  SECOND_LED_PRIO					configMAX_PRIORITIES-13			  	//通讯灯优先级
#define  THIRD_LED_PRIO						configMAX_PRIORITIES-14			  	//通讯灯优先级

#define  RS485_RECEIVE_SIZE					configMINIMAL_STACK_SIZE*5			//通讯进程
#define  TRANS_RECEIVE_SIZE					configMINIMAL_STACK_SIZE*5			//通讯进程
#define  MSG_ANALYSE_SIZE					configMINIMAL_STACK_SIZE*20			//通讯进程
#define  SETUP_ALL_SIZE						configMINIMAL_STACK_SIZE*5			//设置全部加电线程
#define  SCAN_ITH_SIZE						configMINIMAL_STACK_SIZE*5			//扫描VI数据
#define  SAMP_DUT_SIZE						configMINIMAL_STACK_SIZE*5			//采样DUT进程
#define  SAMP_MONIT_SIZE					configMINIMAL_STACK_SIZE*5			//采样监控进程
#define  OTHER_ITEM_SIZE					configMINIMAL_STACK_SIZE*5			//其余功能
#define  LINK_LED_SIZE						configMINIMAL_STACK_SIZE			//错误异常
#define  ERR_LED_SIZE						configMINIMAL_STACK_SIZE			//错误异常
#define  RUN_LED_SIZE						configMINIMAL_STACK_SIZE			//运行灯
#define  FIRST_LED_SIZE						configMINIMAL_STACK_SIZE			//运行灯1
#define  SECOND_LED_SIZE					configMINIMAL_STACK_SIZE			//运行灯2
#define  THIRD_LED_SIZE						configMINIMAL_STACK_SIZE			//运行灯3

#endif /* MAIN_H */

