#ifndef _COMMANDFRAME_1_
#define _COMMANDFRAME_1_

#include <stdint.h>
#include "CommandFrameConfig.h"

//定义显示单元数据结构
typedef struct Command
{  
	struct Command *InfoNext;                    //指向下一个命令块的
	uint8_t *CommandString;                       //命令的具体值的指针
	uint8_t *Parameters;                          //指令具体携带的参数
	void (*CammandAnalyze) (uint8_t DeviceNum,uint8_t *Parameter);  //命令解析
}CommandInfo;

extern CommandInfo  *g_p_FreeCommandInfo;  //定义没有使用的控制
extern CommandInfo  *g_p_UsedCommandInfo;  //已经使用的控制块

//***************************************************
 //功能：清零基本的命令块的数据结构
 //参数：无
 //说明：内部调用函数，不对外使用
//****************************************************
void CommandCellClear(CommandInfo *);

//***************************************************
 //功能：清零说有的数据结构
 //参数：Buffer：模块需要使用的内存空间
 //      BufferLength:内存空间的大小
 //说明：内部调用函数，不对外使用
//****************************************************
void CommandAllStructInitial(uint8_t *Buffer,uint32_t BufferLength);
 
//***************************************************
 //功能：清零说有的数据结构
 //参数：Buffer：模块需要使用的内存空间
 //      BufferLength:内存空间的大小
 //说明：内部调用函数，不对外使用
//****************************************************
void LinkAllFreeCommandInfoCell(uint8_t *Buffer,uint32_t BufferLen);
 
//***************************************************
 //功能：从空闲列表中拿出一个cell
 //参数： ErrorInfo: 错误信息
 //说明：内部调用函数，不对外使用
//****************************************************
CommandInfo*  GetIdleCommandInfoCell(uint8_t* ErrorInfo);

//***************************************************
 //功能：添加一个命令块到已使用的命令块中
 //参数：*UsedCommandInfo：要添加的命令块的指针
 //说明：内部调用函数，不对外使用
//****************************************************
void AddToUseCommandInfoCell(CommandInfo *UsedCommandInfo);
 
#endif
 
 
