#ifndef _COMMANDFRAMECONFIG_
#define _COMMANDFRAMECONFIG_


//错误操作值
#define NO_ERROR                   0x00
#define NO_RESOURCE                0x01 //没有足够的资源
#define NO_COMMANDPROCESSFUNCTION  0x02 //命令找到，可是没有命令处理函数
#define NO_COMMAND                 0x03  //没有找到命令
#define NO_COMMANDSYSINFOPROCESS   0x04  //获取系统信息函数没有注册

//具体配置值
#define MAX_COMMAND_LEN  			400  //命令最大的字符数，包括结尾符

//帧类型定义
#define CONFIG_MESSAGE        0x01
#define COMMUNICATION_MESSAGE 0x02

//内存接口长度
#define COMMAND_CELL_LENGTH  0x10  //用户不能修改

#endif

