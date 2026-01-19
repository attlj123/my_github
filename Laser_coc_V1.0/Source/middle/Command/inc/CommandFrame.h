#ifndef _CommandFrame_
#define _CommandFrame_

#include <stdint.h>
#include "CommandFrame_1.h"
#include "CommandFrame.h"
#include "CommandFrameConfig.h"

#define MAXOPERATEMAX 2000  //字符串处理最长的长度

#define PARANUMERROR  1
#define PARANUMSUCESS 0

//是否接收到消息，用在单线程
#define MESSAGEEXIST 0x01
#define NOMESSAGE    0x00

//定义接口活动状态
#define INTERFACE_ACTIVE    0x01
#define INTERFACE_NO_ACTIVE 0x00

//字符串比较方式
#define BIGANDSMALL 0x00 //区分大小写
#define BIGORSMALL  0x01  //不区分大小写

#define PARA_NOT_SAME     0x00
#define PARA_SAME         0x01

//定义接口信号接收结构体
typedef struct
{
	uint32_t ReceiveDataLength;
	uint32_t BackUpReceiveDataLength;
	uint8_t* DataBuffer;
	uint8_t* DataBufferBackUp;
	uint32_t BufferLength;
	uint8_t  InterfaceActive;
	uint8_t  DeviceNum;
	void (*CommandRetransmit) (uint8_t *Command); //领命转发处理
}CommInterfaceInfo;

//***************************************************
 //功能：获取字符串的长度，不包括结尾符
 //参数：Disnation:字符串首指针       
 //返回值:字符串的长度，为MAXOPERATEMAX时，表示唱超过规定的最大长度
//***************************************************
uint32_t GetStrLength(int8_t *Disnation);

//***************************************************
 //功能：字符串比较
 //参数：SourceStr:源字符串指针
 //                DistinationStr:源字串指针
 //                MaxLen:最长的比较长度
 //                BigAndSmall:是否区分大小写BIGORSMALL:不区分，BIGANDSMALL区分大小写
 //返回值:是否一致PARA_NOT_SAME、PARA_SAME
//****************************************************	
uint8_t StrCompara(uint8_t *SourceStr,uint8_t *DistinationStr,uint32_t MaxLen,uint8_t BigAndSmall);

//***************************************************
 //功能：初始化串口接收字节处理函数
 //参数：ReceiveByte:接收到的字节
 //      OsSendMessage：操作系统发送消息函数，若是单线程写PNULL
 //说明：对外调用使用
//****************************************************
void CommandByteReceiveProcess(CommInterfaceInfo *InterfaceInfo,uint8_t ReceiveByte,void (*OsSendMessage)(uint8_t *Message));

//***************************************************
 //功能：初始化命令集的数据结构
 //参数：Buffer：模块需要使用的内存空间
 //      BufferLength:内存空间的大小
 //说明：对外调用使用
//****************************************************
void CommandSysInit(uint8_t *Buffer,uint32_t BufferLength);


//***************************************************
 //功能：注册命令集具体函数
 //参数：CommandStr:命令字符串指针
 //      CammandAnalyze:命令解析函数
 //返回值：是否生产错误
 //说明：对外调用使用
//****************************************************
uint8_t RegisterCommand(uint8_t *CommandStr,void (*CammandAnalyze) (uint8_t DeviceNum,uint8_t *Parameter));

//***************************************************
 //功能：搜寻相关的指令并且执行相应的注册函数
 //参数：CommandStr:命令字符串指针
 //      Parameter: 命令的参数
 //      BigAndSmall:是否区分大小写 参考MCLib.h中的定义
 //返回值：是否生产错误
 //说明：对外调用使用
//****************************************************
uint8_t CommandSearch(uint8_t DeviceNum,uint8_t *CommandStr,uint8_t *Parameter,uint8_t BigAndSmall);


//***************************************************
 //功能：搜寻失败处理函数的注册
 //参数：CommandStr:命令字符串指针
 //      Parameter: 命令的参数
 //说明：对外调用使用
//****************************************************
void CommandSearchFailedProcessRegister(void (*CommandSearchFailedProcess) (uint8_t DeviceNum,uint8_t *Command,uint8_t *Parameter));
void CommandFailedProcess(uint8_t DeviceNum,uint8_t *Command,uint8_t *Parameter);

//***************************************************
 //功能：搜寻失败处理函数的注册
 //参数：CommandStr:命令字符串指针
 //      Parameter: 命令的参数
 //说明：对外调用使用
//****************************************************
void CommandSearchOverProcessRegister(void (*CommandSearchOverProcess) (uint8_t DeviceNum,uint8_t *Command,uint8_t *Parameter));
void CommandSearchOverProcess(uint8_t DeviceNum,uint8_t *Command,uint8_t *Parameter);

//***************************************************
 //功能：搜寻全局指令信息处理函数的注册
 //参数：CommandStr:命令字符串指针
 //      Parameter: 命令的参数
 //说明：对外调用使用
//****************************************************
void CommandSysInfoProcessRegister(void (*CommandSysInfoProcess) (uint8_t DeviceNum,uint8_t *Command));
void CommandSysInfoProcess(uint8_t DeviceNum,uint8_t *Command);

//***************************************************
 //功能：解析接收到的字符串
 //参数：RecieveredCommand:接收到的字符串buffer
 //说明：对外调用使用
//****************************************************
void CommandAnalyse(uint8_t *RecieveredCommand);

//***************************************************
 //功能：分离命令的参数
 //参数：*Source：原始参数指针
//        ParaCount：分离参数的个数
//        MaxLen:参数最长的长度保护
 //      *xx，后面接的是分离出的参数的字符串
 //说明：对外调用使用
//****************************************************
uint8_t ParaSeparate(uint8_t ParaCount,uint32_t MaxLen,uint8_t *RecieveredPara,...);

//***************************************************
 //功能：设置接口是否处于活动状态
 //参数：无
 //返回值：版本信息
 //说明：对外调用使用
//****************************************************
void McSetInterfaceActive(CommInterfaceInfo *InterfaceInfo,uint8_t ActiveState);

//***************************************************
 //功能：获取系统的版本信息
 //参数：无
 //返回值：版本信息
 //说明：对外调用使用
//****************************************************
float GetCommandFrameVersion(void);

#endif
