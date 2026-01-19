#ifndef __APP_LINK_H__
#define __APP_LINK_H__

#include "TypesDef.h"

//定义帧头帧尾
#define FRAME_HEAD 	0xCD
#define FRAME_END 	0xDC
#define VERIFY_HEX  0xBB

//定义帧错误信息
#define FRAME_SUCCESS        0x00 //无错误
#define FRAME_HEAD_ERROR     0x01 //帧头错误
#define FRAME_END_ERROR      0x02 //帧尾错误
#define FRAME_KIND_ERROR     0x03 //帧Kind位错误
#define FRAME_INDEX_ERROR    0x04 //帧Index位错误
#define FRAME_CARDNUM_ERROR  0x05 //板卡号错误
#define FRAME_VERIFY_ERROR   0x06 //帧校验错误

//操作结果返回值
#define  RECEIVE_SUCCESS         0x00 //数据接收成功
#define  TIME_OUT                0x01
#define  FILE_SIZE_OVERLOAD      0x02
#define  DATA_FLASH_STORE_ERROR  0x03

#define  DATA_TRANS_NOT_OVER     0x01
#define  DATA_TRANS_OVER         0x00
#define  START_TRY_TIMES         0x0A

//#define  FLASH_PAGE_SIZE    			0x20000      //STM32F103 器件的FLASH PAGE是2K
#define  PROGRAM_BASE_ADDR  			0x08020000   //程序基址
#define  MAX_PAGE           			7        //STM32F107  最大的可用PAGE数是234/2=117

#define  ADDR_FLASH_SECTOR_0			0x08000000
#define  ADDR_FLASH_SECTOR_1			0x08020000
#define  ADDR_FLASH_SECTOR_2			0x08040000
#define  ADDR_FLASH_SECTOR_3			0x08060000
#define  ADDR_FLASH_SECTOR_4			0x08080000
#define  ADDR_FLASH_SECTOR_5			0x080A0000
#define  ADDR_FLASH_SECTOR_6			0x080C0000
#define  ADDR_FLASH_SECTOR_7			0x080E0000

#define  FLASH_WAITETIME   50000

#define  STM32_FLASH_BASE					0x08000000
/********************************************************************
功能:计算帧校验位
输入:DataBuffer 帧首地址
     Length 数据长度
输出:校验值
*********************************************************************/
uint8 App_Link_GetVerify(uint8* DataBuffer,uint16 Length);


/********************************************************************
功能:检查帧校验位是否正确
输入:DataBuffer 帧首地址
     Length 帧长度
     kind 存储帧kind位
     Index 存储帧Index位
输出:校验是否正确
*********************************************************************/
uint8 App_Link_CheckVerify(uint8* DataBuffer,uint16 Length,uint8 *kind,uint8 *Index);


/********************************************************************
功能:Can发送应答信息
输入:无
输出:无
*********************************************************************/
void App_Link_TxAckPacket(uint8 Kind,uint8 Index);


/********************************************************************
功能:获取文件大小信息
输入:TimeOut 超时时间
     FileSize 存储文件大小
输出:错误信息
*********************************************************************/
uint8 App_Link_ReadFileSize (uint32 TimeOut,uint32 *FileSize);


/********************************************************************
功能:获取文件数据
输入:TimeOut 超时时间
     IndexNeed 第几帧数据
     FileOver  存储文件结束信息
     sign 读取成功信息
     DataLength 存储文件长度
输出:文件数据
*********************************************************************/
uint8* App_Link_ReadData(uint32 TimeOut,uint8 IndexNeed,uint8 *FileOver,uint8 *sign,uint8 *DataLength);


/********************************************************************
功能:计算文件需要暂用多少页FLASH
输入:FileSize 文件大小
输出:文件数据
*********************************************************************/
uint32 App_Link_FLASH_PagesMask(uint32 FileSize);


/********************************************************************
功能:下载程序文件
输入:无
输出:执行错误信息
*********************************************************************/
uint8 App_Link_BinFileDownLoad(void);


#endif

