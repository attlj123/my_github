#include "CommandFrame_1.h"
#include "CommandFrameConfig.h"
#include <stdlib.h>

CommandInfo  *g_p_FreeCommandInfo=NULL;  //定义没有使用的控制
CommandInfo  *g_p_UsedCommandInfo=NULL;  //已经使用的控制块

//***************************************************
 //功能：清零基本的命令块的数据结构
 //参数：无
 //说明：内部调用函数，不对外使用
//****************************************************
void CommandCellClear(CommandInfo *p_TempCell)
{
		p_TempCell->InfoNext=NULL;
		p_TempCell->CommandString=NULL;
		p_TempCell->Parameters=NULL;
		p_TempCell->CammandAnalyze=NULL;	
}



//***************************************************
 //功能：清零说有的数据结构
 //参数：Buffer：模块需要使用的内存空间
 //      BufferLength:内存空间的大小
 //说明：内部调用函数，不对外使用
//****************************************************
void CommandAllStructInitial(uint8_t *Buffer,uint32_t BufferLength)
{
	CommandInfo* p_Temp=NULL;
	uint32_t i=0;
	for(i=0;i<BufferLength/COMMAND_CELL_LENGTH;i++)
	{
		p_Temp=(CommandInfo *)(Buffer+i*COMMAND_CELL_LENGTH);
		CommandCellClear(p_Temp);
	}
}	
 
//***************************************************
 //功能：清零说有的数据结构
 //参数：Buffer：模块需要使用的内存空间
 //      BufferLength:内存空间的大小
 //说明：内部调用函数，不对外使用
//****************************************************
void LinkAllFreeCommandInfoCell(uint8_t *Buffer,uint32_t BufferLength)
{
	uint32_t i=0;
	CommandInfo* p_Temp=NULL;
	g_p_FreeCommandInfo =(CommandInfo *)Buffer;
	p_Temp=g_p_FreeCommandInfo;
	for(i=1;i<BufferLength/COMMAND_CELL_LENGTH;i++)
	{
		p_Temp->InfoNext=(CommandInfo *)(Buffer+i*COMMAND_CELL_LENGTH);
		p_Temp=p_Temp->InfoNext;
	}
}
 
//***************************************************
 //功能：从空闲列表中拿出一个cell
 //参数： ErrorInfo: 错误信息
 //说明：内部调用函数，不对外使用
//****************************************************
CommandInfo*  GetIdleCommandInfoCell(uint8_t* ErrorInfo)
{
	CommandInfo *p_FirstPoint=NULL;
	p_FirstPoint=g_p_FreeCommandInfo; //取空闲的命令块的链表首地址
//判断是否是空的链表
	if(p_FirstPoint ==NULL)
	{
		*ErrorInfo = NO_RESOURCE;//链表已经取空了
		return NULL;
	}
//若是有资源，取一个命令块的首地址
	g_p_FreeCommandInfo = p_FirstPoint->InfoNext; //周首地址指向下一个
	return p_FirstPoint;
}


//***************************************************
 //功能：添加一个命令块到已使用的命令块中
 //参数：*UsedCommandInfo：要添加的命令块的指针
 //说明：内部调用函数，不对外使用
//****************************************************
void AddToUseCommandInfoCell(CommandInfo *UsedCommandInfo)
{
	CommandInfo* p_Temp=g_p_UsedCommandInfo;
	if(g_p_UsedCommandInfo==NULL)    //确认是否目前还没有使用的命令块
	{
		g_p_UsedCommandInfo=UsedCommandInfo;
		return;
	}
	while(p_Temp->InfoNext!=NULL)  //找到链表的尾部
	{
		p_Temp = p_Temp->InfoNext;
	}
	p_Temp->InfoNext=UsedCommandInfo;
}

