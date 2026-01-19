#include <stdarg.h>  
#include <stdlib.h>
#include "CommandFrame.h"
#include "CommandFrame_1.h"
#include "CommandFrameConfig.h"

//版本信息
#define COMMANDVERSION  0.1 //中间件的版本号

//定义帧结束符
#define COMMAND_END_FRAME_0 '\r'  //定义帧结尾付
#define COMMAND_END_FRAME_1 '\n'  //定义帧结尾付

static void (*g_CommandSearchOverProcess) (uint8_t DeviceNum,uint8_t *Command,uint8_t *Parameter)=NULL;    //命令收索成功后的处理函数(命令处理函数已经执行后）
static void (*g_CommandSearchFailedProcess) (uint8_t DeviceNum,uint8_t *Command,uint8_t *Parameter)=NULL;  //命令收索失败后的处理函数
static void (*g_CommandSysInfoProcess) (uint8_t DeviceNum,uint8_t *Command)=NULL;                        //全局指令信息处理函数的注册

//***************************************************
 //功能：初始化串口接收字节处理函数
 //参数：ReceiveByte:接收到的字节
 //      OsSendMessage：操作系统发送消息函数，若是单线程写PNULL
 //说明：对外调用使用
//****************************************************
void CommandByteReceiveProcess(CommInterfaceInfo *InterfaceInfo,uint8_t ReceiveByte,void (*OsSendMessage)(uint8_t *Message))
{
	uint32_t i=0;
	//若是接口不是能的情况下，直接退出
	if(InterfaceInfo->InterfaceActive==INTERFACE_NO_ACTIVE)
		return;
//防止开始的非法字符
	if(0x00==InterfaceInfo->ReceiveDataLength)
	{
		if(!((ReceiveByte>=' ')&&(ReceiveByte<='~')))
			return ;
	}
	
	if((ReceiveByte!='\n')&&(ReceiveByte!='\r')&&(ReceiveByte!='\b')&&(ReceiveByte!='\t')) //若不是回车或是退格
	{
		if(InterfaceInfo->ReceiveDataLength<InterfaceInfo->BufferLength-1)
		{
			*(InterfaceInfo->DataBuffer+InterfaceInfo->ReceiveDataLength)=ReceiveByte;
			InterfaceInfo->ReceiveDataLength++;
		}
	}
	else if((ReceiveByte==COMMAND_END_FRAME_0)||(ReceiveByte==COMMAND_END_FRAME_1))
	{
		*(InterfaceInfo->DataBuffer+InterfaceInfo->ReceiveDataLength)='\0'; //添加字符串结尾符
		InterfaceInfo->BackUpReceiveDataLength=InterfaceInfo->ReceiveDataLength;     //保存帧长度

		for(i=0;i<=InterfaceInfo->ReceiveDataLength;i++)
			*(InterfaceInfo->DataBufferBackUp+i)=*(InterfaceInfo->DataBuffer+i); //命令也保存到备份缓存中
		InterfaceInfo->ReceiveDataLength=0;            //重新回到起始位置接收命令
		//发送消息
		if(OsSendMessage!=NULL)
			OsSendMessage((uint8_t*)InterfaceInfo);
	}
	else if(ReceiveByte=='\b')   //若是退格
	{
		if(InterfaceInfo->ReceiveDataLength>0)
		{
			InterfaceInfo->ReceiveDataLength--;
			*(InterfaceInfo->DataBuffer+InterfaceInfo->ReceiveDataLength)='\0';
		}
	}
}



//***************************************************
 //功能：初始化命令集的数据结构
 //参数：Buffer：模块需要使用的内存空间
 //      BufferLength:内存空间的大小
 //说明：对外调用使用
//****************************************************
void CommandSysInit(uint8_t *Buffer,uint32_t BufferLength)
{
	CommandAllStructInitial(Buffer,BufferLength);
	LinkAllFreeCommandInfoCell(Buffer,BufferLength);
}


//***************************************************
 //功能：注册命令集具体函数
 //参数：CommandStr:命令字符串指针
 //      CammandAnalyze:命令解析函数
 //说明：对外调用使用
//****************************************************
uint8_t RegisterCommand(uint8_t *CommandStr,void (*CammandAnalyze) (uint8_t DeviceNum,uint8_t *Parameter))
{
	uint8_t TempErrorInfo=0;
	CommandInfo *p_Temp=NULL;

  //先获取一个命令块
	p_Temp=GetIdleCommandInfoCell(&TempErrorInfo);
	CommandCellClear(p_Temp);  //清楚命令块的信息
  
	if(TempErrorInfo!=NO_ERROR)
		return TempErrorInfo; //若是获取失败
	 
  //注册相应的函数与参数
	p_Temp->CommandString=CommandStr;
	p_Temp->CammandAnalyze=CammandAnalyze;
  
  //将这个命令块添加到已经使用的控制块中
	AddToUseCommandInfoCell(p_Temp);
  
	return NO_ERROR;  
}

//***************************************************
 //功能：搜寻相关的指令并且执行相应的注册函数
 //参数：CommandStr:命令字符串指针
 //      Parameter: 命令的参数
 //      BigAndSmall:是否区分大小写 参考MCLib.h中的定义
 //说明：对外调用使用
//****************************************************
uint8_t CommandSearch(uint8_t DeviceNum,uint8_t *CommandStr,uint8_t *Parameter,uint8_t BigAndSmall)
{
	CommandInfo *p_Temp=NULL;
	CommandInfo *p_TempAfter=NULL;
	void (*TempCammandAnalyze) (uint8_t DeviceNum,uint8_t *Parameter);  //零时定义的命令解析的程序指针
	uint8_t CommandFind=PARA_NOT_SAME;
	uint8_t sysCommandFind=PARA_NOT_SAME;
   //收索已经使用的链表
	p_Temp=g_p_UsedCommandInfo;
	if(p_Temp==NULL) //若是不存在命令
		return NO_RESOURCE;

	while((PARA_NOT_SAME==CommandFind)&&(p_Temp!=NULL)&&(PARA_NOT_SAME==sysCommandFind))
	{
		CommandFind=StrCompara(CommandStr,p_Temp->CommandString,MAX_COMMAND_LEN,BigAndSmall);
		sysCommandFind=StrCompara(CommandStr,(uint8_t*)"*help",MAX_COMMAND_LEN,BigAndSmall);
		p_TempAfter=p_Temp;
		p_Temp=p_Temp->InfoNext; //指向下一个
	} 
	 //若是收索到了用户命令
	if(CommandFind==PARA_SAME)
	{
		TempCammandAnalyze= p_TempAfter-> CammandAnalyze; 
		if(TempCammandAnalyze==NULL)
			return NO_COMMANDPROCESSFUNCTION;
		TempCammandAnalyze(DeviceNum,Parameter);  //调用处理函数
			return NO_ERROR; 
	}
		
     //若是收索到了系统命令
	if(sysCommandFind==PARA_SAME)
	{
		//收索已经使用的链表
		p_Temp=g_p_UsedCommandInfo;
			while((p_Temp!=NULL))
			{
				CommandSysInfoProcess(DeviceNum,p_Temp->CommandString);
				p_Temp=p_Temp->InfoNext; //指向下一个
			}
		return NO_ERROR;
	}
	return NO_COMMAND;
}


//***************************************************
 //功能：搜寻失败处理函数的注册
 //参数：CommandStr:命令字符串指针
 //      Parameter: 命令的参数
 //说明：对外调用使用
//****************************************************
void CommandSearchFailedProcessRegister(void (*CommandSearchFailedProcess) (uint8_t DeviceNum,uint8_t *Command,uint8_t *Parameter))
{
	g_CommandSearchFailedProcess = CommandSearchFailedProcess;
}

void CommandFailedProcess(uint8_t DeviceNum,uint8_t *Command,uint8_t *Parameter)
{
	if(g_CommandSearchFailedProcess!=NULL)
		g_CommandSearchFailedProcess(DeviceNum,Command,Parameter);
}


//***************************************************
 //功能：搜寻失败处理函数的注册
 //参数：CommandStr:命令字符串指针
 //      Parameter: 命令的参数
 //说明：对外调用使用
//****************************************************
void CommandSearchOverProcessRegister(void (*CommandSearchOverProcess) (uint8_t DeviceNum,uint8_t *Command,uint8_t *Parameter))
{
	g_CommandSearchOverProcess = CommandSearchOverProcess;
}

void CommandSearchOverProcess(uint8_t DeviceNum,uint8_t *Command,uint8_t *Parameter)
{	
	if(g_CommandSearchOverProcess!=NULL)
		 g_CommandSearchOverProcess(DeviceNum,Command,Parameter);
}

//***************************************************
 //功能：搜寻全局指令信息处理函数的注册
 //参数：CommandStr:命令字符串指针
 //      Parameter: 命令的参数
 //说明：对外调用使用
//****************************************************
void CommandSysInfoProcessRegister(void (*CommandSysInfoProcess) (uint8_t DeviceNum,uint8_t *Command))
{
	g_CommandSysInfoProcess = CommandSysInfoProcess;
}

void CommandSysInfoProcess(uint8_t DeviceNum,uint8_t *Command)
{
	if(g_CommandSysInfoProcess!=NULL)
		g_CommandSysInfoProcess(DeviceNum,Command);
}

//***************************************************
 //功能：解析接收到的字符串
 //参数：RecieveredCommand:接收到的字符串buffer
 //说明：对外调用使用
//****************************************************
void CommandAnalyse(uint8_t *CommandInfo)
{
	uint8_t *ParaStr=NULL;
	uint32_t Length=0;
	uint32_t Position=0;
	uint32_t i=0;
	uint8_t Error=0;
	uint8_t DeviceNum;
	void (*CommandRetransmitTemp) (uint8_t *Command)=((CommInterfaceInfo*)CommandInfo)->CommandRetransmit; //获取转发处理函数
	uint8_t* RecieveredCommand=((CommInterfaceInfo*)CommandInfo)->DataBufferBackUp; //取命令信息
	DeviceNum=((CommInterfaceInfo*)CommandInfo)->DeviceNum;

	//获取领命长度
	Length=GetStrLength((int8_t *)RecieveredCommand);

   //找到命令分界点
	while((i<Length)&&(*(RecieveredCommand+i)!=' '))
		i++;
	Position=i;     //在第一个空格处添加'\0'作为命令的结束分割,记录此点
	
   //获取参数部分指针
	while((i<Length)&&(*(RecieveredCommand+i)==' '))    //去除中间部分的空格
		{i++;}
	ParaStr=RecieveredCommand+i;  //获取到Parameter的指针
	
	*(RecieveredCommand+Position)='\0' ; //添加领命结束符
	
	//启动命令收索中间件 
	Error=CommandSearch(DeviceNum,RecieveredCommand,ParaStr,BIGORSMALL); 

	//还原领命本身
	if(Position<Length)
		*(RecieveredCommand+Position)=' ' ; //还原空格
	if((Error!=NO_ERROR)&&(CommandRetransmitTemp!=NULL))
		CommandRetransmitTemp(RecieveredCommand); //若是命令不存在，转发领命，用于支持本地与转发操作
}


//***************************************************
 //功能：分离命令的参数
 //参数：*Source：原始参数指针
//        ParaCount：分离参数的个数
//        MaxLen   :参数的最长长度
 //      *xx，后面接的是分离出的参数的字符串
 //返回值：实际分离的参数的个数
 //说明：对外调用使用
//****************************************************
uint8_t  ParaSeparate(uint8_t ParaCount,uint32_t MaxLen,uint8_t *RecieveredPara,...)
{
	uint8_t RealSeparateCount=0;
	uint8_t ReturnBackRealNum=0;
	uint8_t *p_PointTemp=NULL;
	uint8_t *p_SourcePoint=NULL;
	uint32_t RealLength=0;

	va_list ap;
	va_start(ap, RecieveredPara);  //获取可变参数的首地址

	p_SourcePoint=RecieveredPara; //先获取参数首地址
	 
	//开始收索参数
	while(RealSeparateCount<ParaCount)
	{  
		RealLength=0;
		//先除掉前面可能存在的空格
		while(*p_SourcePoint==' ')
			p_SourcePoint++;
	
		//若是有结束符，者提前结束
		if(*p_SourcePoint	=='\0')
			RealSeparateCount=ParaCount+1; 
		else
		{ 
			//Copy参数
			p_PointTemp=va_arg(ap, uint8_t *); //获取参数
			while((*p_SourcePoint!=' ')&&(*p_SourcePoint!='\0'))   //若不是空格和结束符
			{
				if(RealLength<MaxLen) //超过保护长度不操作
				{
					*p_PointTemp=*p_SourcePoint;
					p_PointTemp++;
				}
				p_SourcePoint++;
				RealLength++;
			}
			*p_PointTemp='\0';
			RealSeparateCount++;
			ReturnBackRealNum=RealSeparateCount;
		}
	}
	
	va_end(ap);

	return ReturnBackRealNum;
}

//***************************************************
 //功能：设置接口是否处于活动状态
 //参数：无
 //返回值：版本信息
 //说明：对外调用使用
//****************************************************
void McSetInterfaceActive(CommInterfaceInfo *InterfaceInfo,uint8_t ActiveState)
{
	if(InterfaceInfo->InterfaceActive==ActiveState)
		return;
	InterfaceInfo->ReceiveDataLength=0x00;
	InterfaceInfo->BackUpReceiveDataLength=0x00;
	InterfaceInfo->InterfaceActive=ActiveState;
}



//***************************************************
 //功能：获取系统的版本信息
 //参数：无
 //返回值：版本信息
 //说明：对外调用使用
//****************************************************
float GetCommandFrameVersion(void)
{
	return COMMANDVERSION;
}


//***************************************************
 //功能：字符串比较
 //参数：SourceStr:源字符串指针
 //                DistinationStr:源字串指针
 //                MaxLen:最长的比较长度
 //                BigAndSmall:是否区分大小写BIGORSMALL:不区分，BIGANDSMALL区分大小写
 //返回值:是否一致PARA_NOT_SAME、PARA_SAME
//****************************************************	
uint8_t StrCompara(uint8_t *SourceStr,uint8_t *DistinationStr,uint32_t MaxLen,uint8_t BigAndSmall)
{ 
	uint32_t TempPosition=0;
	uint8_t ComparaNotSame=0;
	if(GetStrLength((int8_t *)SourceStr)!=GetStrLength((int8_t *)DistinationStr))
		return PARA_NOT_SAME; //长度不一致
	if(BigAndSmall==BIGANDSMALL)  //区分大小写
	{
		while(((TempPosition<MaxLen)&&(ComparaNotSame>0))||(*(SourceStr+TempPosition)!='\0'))
		{
			if(*(SourceStr+TempPosition)!=*(DistinationStr+TempPosition))
			ComparaNotSame=1;  
			TempPosition++;
		}
	}
	else  //若果不区分大小写
	{
		while(((TempPosition<MaxLen)&&(ComparaNotSame>0))||(*(SourceStr+TempPosition)!='\0'))
		{
			if((*(SourceStr+TempPosition)>='A')&&(*(SourceStr+TempPosition)<='Z')) //如是大写字母
			{
				if((*(SourceStr+TempPosition)!=*(DistinationStr+TempPosition))&&(*(SourceStr+TempPosition)+0x20!=*(DistinationStr+TempPosition)))
					ComparaNotSame=1;  
				TempPosition++;
			}
			else if((*(SourceStr+TempPosition)>='a')&&(*(SourceStr+TempPosition)<='z')) //如是大写字母
			{
				if((*(SourceStr+TempPosition)!=*(DistinationStr+TempPosition))&&(*(SourceStr+TempPosition)-0x20!=*(DistinationStr+TempPosition)))
					ComparaNotSame=1;  
				TempPosition++;
			}
			else
			{
				if(*(SourceStr+TempPosition)!=*(DistinationStr+TempPosition))  //如果不是英文字母
					ComparaNotSame=1;  
				TempPosition++;
			}
		}
	}
	 
	if((TempPosition>=MaxLen)||(ComparaNotSame==0x01))
		return PARA_NOT_SAME;
		
	return PARA_SAME;
}

//***************************************************
 //功能：获取字符串的长度，不包括结尾符
 //参数：Disnation:字符串首指针       
 //返回值:字符串的长度，为MAXOPERATEMAX时，表示唱超过规定的最大长度
//***************************************************
uint32_t GetStrLength(int8_t *Disnation)
{
	uint32_t i=0;
	while((*(Disnation+i)!='\0')&&(i<MAXOPERATEMAX))
		i++;
	return i;
}

