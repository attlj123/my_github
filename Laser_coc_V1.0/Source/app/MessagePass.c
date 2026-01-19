#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "Driver_I2C.h"
#include "Driver_Uart.h"
#include "ota.h"

#include "CommandFrame.h"

#include "MessagePass.h"
#include "FunctionSet.h"
#include "ControlParam.h"
#include "ComHandle.h"

#include "app_cfg.h"

__align(8) uint8_t g_p_FreeCommandMem[MAX_COMMAND_NUM*MAX_COMMAND_LEN] = {0};      //空闲消息内存块

//ETH MQTT接口的使用数据
uint8_t CommandBuffer[MAX_COMMAND_LEN+1];
uint8_t CommandBufferBackup[MAX_COMMAND_LEN+1];

//ETH MQTT接口
CommInterfaceInfo EthMqttInterface={0,
									0,
									CommandBuffer,
									CommandBufferBackup,
									MAX_COMMAND_LEN+1,
									INTERFACE_ACTIVE,
									ETH_MQTT_INTERFACE,
									NULL};

//RS485 接口
CommInterfaceInfo RS485Interface={0,
									0,
									CommandBuffer,
									CommandBufferBackup,
									MAX_COMMAND_LEN+1,
									INTERFACE_ACTIVE,
									RS485_DEVICE_INTERFACE,
									NULL};

//实例化485参数结构								
QueueHandle_t MessageQueue = NULL;
																									
char tx_buff[1300] = {0};									

extern char TranRxBuffer[30];
extern char TranHexString[30];
extern uint8_t g_HardAddress;
extern ComTrans_TypeStruct g_s_ComTransimt;
							
extern QueueHandle_t PublishQueue;

extern SemaphoreHandle_t SetupALLSemphr;    	//设置信号量
extern SemaphoreHandle_t ScanIthSemphr;    		//扫描信号量

extern SemaphoreHandle_t SampDutSemphr;			//采样DUT参数
extern SemaphoreHandle_t SampMonitSemphr;		//采样监控参数

extern SemaphoreHandle_t OtherItemSemphr;
extern SemaphoreHandle_t LinkLedSemphr;
extern SemaphoreHandle_t FirstLedSemphr;		
extern SemaphoreHandle_t SecondLedSemphr;
extern SemaphoreHandle_t ThirdLedSemphr;

extern DUT_TypeStruct 	 g_s_DutputInfor[DUT_MAX_NUM];
extern Driver_TypeStruct g_s_DriverInfor[DRIVER_NUM];
extern SampIn_TypeStruct g_s_SampInInfor[SAMP_MAX_NUM];
extern System_TypeStruct g_s_SystemInfor;

extern TaskHandle_t SampDutHandle;					//采样DUT线程
extern TaskHandle_t SampMonitHandle;				//采样监控现场
extern TaskHandle_t OtherEntryHandle;

extern TaskHandle_t FirstLedHandle;					//采样DUT线程
extern TaskHandle_t SecondLedHandle;					//采样监控现场
extern TaskHandle_t ThirdLedHandle;
																	
extern uint16_t IthVoltage[ITH_MAX_COUNT];
extern uint16_t IthCurrent[ITH_MAX_COUNT];
extern uint16_t IthPD[ITH_MAX_COUNT];	
//***************************************************
 //功能：将内存的值刷新为指定的值
 //参数：buffer:内存地址
 //                length:刷新长度
 //                Data:刷新的具体的内容
//****************************************************	
void BuffClear(uint8_t *buffer,uint32_t length,uint8_t Data)
{
	uint32_t i=0;
	for(i=0;i<length;i++)
		*(buffer+i)=Data;
}

//***************************************************
 //功能string to HEX
 //参数：buffer:内存地址
 //                length:刷新长度
 //                Data:刷新的具体的内容
//****************************************************	
int string2hex(char* str,char* hex)
{
    int i = 0;
    int j = 0;
    unsigned char temp = 0;
    int str_len = 0;
    char str_cpy[100] = {'0'};
    strcpy(str_cpy,str);
    str_len = strlen(str_cpy);
    if(str_len==0)
    {
        return 1;
    }
    while(i < str_len)
    {
        if(str_cpy[i]>='0' && str_cpy[i]<='F') 
        {
            if((str_cpy[i]>='0' && str_cpy[i]<='9'))
            {
                temp = (str_cpy[i] & 0x0f)<<4;
            }
            else if(str_cpy[i]>='A' && str_cpy[i]<='F')
            {
                temp = ((str_cpy[i] + 0x09) & 0x0f)<<4;
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }   
        i++;
        if(str_cpy[i]>='0' && str_cpy[i]<='F') 
        {
            if(str_cpy[i]>='0' && str_cpy[i]<='9')
            {
                temp |= (str_cpy[i] & 0x0f);
            }
            else if(str_cpy[i]>='A' && str_cpy[i]<='F')
            {
                temp |= ((str_cpy[i] + 0x09) & 0x0f);
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        } 
        i++;
        hex[j] = temp;
        
        j++;
    }

    return 0 ;
}

//**************************************************
//函数：SetReset
//功能：设置复位
//说明：
//**************************************************
void SetReset(uint8_t DeviceNum,uint8_t *Parameter)
{	
	NVIC_SystemReset();
}


//**************************************************
//函数：CallIDN
//功能：调用IDN编号
//说明：
//**************************************************
void CallIDN(uint8_t DeviceNum,uint8_t *Parameter)
{	
	char Version[] = "V1.0";
	char Model[] = "COC_BurnIn";
	char buf[] = "20260112888";
	char err = 0;
	char* p_tx_buff = NULL;
	uint16_t send_lengt = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	if(err == 0)
		sprintf(tx_buff+STRING_START, "%s_%d_%s_%s\n",Model,g_HardAddress,Version,buf);
	else
		sprintf(tx_buff+STRING_START, "err\n");
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);    //修改发生方式 增加mqtt指定长度发送
}

//***************************************************
//函数名  :ConfigLDBurnInMode
//功能说明：设置老化模式ACC或者APC
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ConfigLDBurnInMode(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
		
	if(0 == strcmp((char*)Parameter, "ACC"))
    {
		g_s_SystemInfor.BurnInMode = BURNIN_ACC_MODE;
		sprintf(tx_buff+STRING_START,"ok\n");
    }
	else if(0 == strcmp((char*)Parameter, "APC"))
    {
		g_s_SystemInfor.BurnInMode = BURNIN_APC_MODE;
		sprintf(tx_buff+STRING_START,"ok\n");
    }
	else 
	{
		sprintf(tx_buff+STRING_START,"err\n");
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :ConfigLDBurnInMode
//功能说明：设置老化模式ACC或者APC
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void CheckLDBurnInMode(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
    if(g_s_SystemInfor.BurnInMode == BURNIN_ACC_MODE)
		sprintf(tx_buff+STRING_START,"ACC\n");
    else if(g_s_SystemInfor.BurnInMode == BURNIN_APC_MODE)
		sprintf(tx_buff+STRING_START,"APC\n");

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :ConfigAPCModePara
//功能说明：设置APC模式下参数值
//输入说明: Parameter：PDValue,LDLimit,PDoffset
//返回值：无
//说明：外部调用
//*************************************************
void ConfigAPCModePara(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	char Para0[10] = {0};
	char Para1[10] = {0};
	char Para2[10] = {0};
	char Para3[10] = {0};
	
	double PDValue = 0;
	double LDLimit = 0;
	double PDoffset = 0;
    uint8_t PortData = 0;
	
	uint8_t i=0;

    ParaSeparate(4,9,Parameter,Para0,Para1,Para2,Para3);

	PDValue = strtod(Para0,NULL);
	LDLimit = strtod(Para1,NULL);
	PDoffset = strtod(Para2,NULL);
	PortData = atoi((char*)Para3);

	if((PDValue>=0)&&(LDLimit>=0)&&(PDoffset>=0))
	{
		if(PortData == 0)
		{
			for(i=0;i<SAMP_MAX_NUM;i++)
			{
				g_s_SampInInfor[i].APCModePDValue = PDValue;
				PortAPCPidInitial(i);
			}
		}
		else
		{
			g_s_SampInInfor[PortData-1].APCModePDValue = PDValue;
			PortAPCPidInitial(PortData-1);
		}
	
		g_s_SystemInfor.APCLimitCurrent = LDLimit;
		g_s_SystemInfor.ImPdOffSet = PDoffset;
		sprintf(tx_buff+STRING_START,"ok\n");
	}
	else 
	{
		sprintf(tx_buff+STRING_START,"err\n");
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :CheckAPCModePara
//功能说明：读取APC设置参数值 PDValue,LDLimit,PDOffSet
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void CheckAPCModePara(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
		
    uint8_t PortData = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));
		
	PortData = atoi((char*)Parameter);
		
	if(PortData > SAMP_MAX_NUM)
		PortData = SAMP_MAX_NUM;
	
	if(PortData < 1)
		PortData = 1;

	sprintf(tx_buff+STRING_START,"%.3f %.3f %.3f\n",g_s_SampInInfor[PortData-1].APCModePDValue,g_s_SystemInfor.APCLimitCurrent,g_s_SystemInfor.ImPdOffSet);

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :CheckAPCModePara
//功能说明：读取APC设置参数值 PDValue,LDLimit,PDOffSet
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ConfigPortSwitch(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	char port_str[10] = {0};
	char data_str[10] = {0};
		
    uint8_t port = 0;
	char Hex_char[2] = {0};

    ParaSeparate(2,9,Parameter,port_str,data_str);
	
	port = atoi((char*)port_str);
		
	if(port > DRIVER_NUM)
		port = DRIVER_NUM;
	
	if(port < 1)
		port = 1;
	
	string2hex((char*)data_str,Hex_char);
	
	g_s_DriverInfor[port-1].PortOutput = ((uint16_t)Hex_char[0])<<8|((uint16_t)Hex_char[1]);
	
	if(g_s_DriverInfor[port-1].BitStatus == DRIVER_BIT_VALID)
	{
		DriverOutTCA9535(port,g_s_DriverInfor[port-1].PortOutput);
		sprintf(tx_buff+STRING_START,"ok\n");
	}
	else 
	{
		sprintf(tx_buff+STRING_START,"err\n");
	}
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;

	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}


//***************************************************
//函数名  :CheckAPCModePara
//功能说明：读取APC设置参数值 PDValue,LDLimit,PDOffSet
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ReadDrvierType(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;

    uint8_t port = 0;

	port = atoi((char*)Parameter);
		
	if(port > DRIVER_NUM)
		port = DRIVER_NUM;
	
	if(port < 1)
		port = 1;

	sprintf(tx_buff+STRING_START,"%d %d\n",g_s_DriverInfor[port-1].BitStatus,g_s_DriverInfor[port-1].DriverType);
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;

	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}


//***************************************************
//函数名  :ConfigLDCurrentProcess
//功能说明： 配置LD的驱动电流的大小
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ConfigLDCurrentProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
    char CurrentBuffer[10] = {0};
    char PortBuffer[10] = {0};
    char DriverBuffer[10] = {0};

    uint8_t PortData = 0;
    uint8_t DriverData = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));

    ParaSeparate(3,6,Parameter,CurrentBuffer,PortBuffer,DriverBuffer);
		
	PortData = atoi((char*)PortBuffer);
	DriverData = atoi((char*)DriverBuffer);
	
	if(DriverData >= DRIVER_NUM)
		DriverData = DRIVER_NUM;
	
	if(DriverData < 1)
		DriverData = 1;

	g_s_SystemInfor.CurrDriver = DriverData;
		
    g_s_DriverInfor[g_s_SystemInfor.CurrDriver-1].SetValue = strtod(CurrentBuffer,NULL);

    if(PortData == 0)
	{
		g_s_SystemInfor.AllsetStatus = 1;
		
		if( xSemaphoreGive(SetupALLSemphr) != pdTRUE )
			sprintf(tx_buff+STRING_START, "err\n");
		else
			sprintf(tx_buff+STRING_START, "ok\n");
	}
    else
    {
		if(PortData >= DUT_MAX_NUM)
			PortData = DUT_MAX_NUM;

		LDCurrentSetBit_OneStep(&g_s_DriverInfor[g_s_SystemInfor.CurrDriver-1],&g_s_DutputInfor[PortData-1]);

		sprintf(tx_buff+STRING_START,"ok\n");
    }

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :CheckLDCurrentProcess
//功能说明：查询LD的驱动电流的大小
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void CheckLDCurrentProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
    uint8_t DriverData = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	DriverData = atoi((char*)Parameter);

	if(DriverData >= DRIVER_NUM)
		DriverData = DRIVER_NUM;
	
	if(DriverData < 1)
		DriverData = 1;

	sprintf(tx_buff+STRING_START,"%.3f\n",g_s_DriverInfor[DriverData-1].SetValue);

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :ReadLDScanStatusProcess
//功能说明： 查询扫描电流起点，步进，终点
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ReadSetupAllStatus(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	sprintf(tx_buff+STRING_START,"%d\n",g_s_SystemInfor.AllsetStatus);
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :ConfigPDVoltageProcess
//功能说明： 配置PD 工作反偏电压
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ConfigMPDVoltageProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	g_s_SystemInfor.SetVmpdValue = strtod((char*)Parameter,NULL);
		
	if((g_s_SystemInfor.SetVmpdValue<=5)&&(g_s_SystemInfor.SetVmpdValue>=0))
	{
		SetFuncValue(15,1,g_s_SystemInfor.SetVmpdValue*g_s_SystemInfor.MPDDACSetK+g_s_SystemInfor.MPDDACSetB);
		sprintf(tx_buff+STRING_START,"ok\n");
	}
	else 
	{
		g_s_SystemInfor.SetVmpdValue = 0;
		sprintf(tx_buff+STRING_START,"err\n");
	}
		
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :CheckPDVoltageProcess
//功能说明： 查询PD  反偏电压
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void CheckMPDVoltageProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	sprintf(tx_buff+STRING_START,"%.3f\n",g_s_SystemInfor.SetVmpdValue);

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}



//***************************************************
//函数名  :ConfigPDVoltageProcess
//功能说明： 配置PD 工作反偏电压
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ConfigPDVoltageProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	g_s_SystemInfor.SetVpdValue = strtod((char*)Parameter,NULL);
		
	if((g_s_SystemInfor.SetVpdValue<=5)&&(g_s_SystemInfor.SetVpdValue>=0))
	{
		SetFuncValue(15,1,g_s_SystemInfor.SetVpdValue*g_s_SystemInfor.VpdDACSetK+g_s_SystemInfor.VpdDACSetB);
		sprintf(tx_buff+STRING_START,"ok\n");
	}
	else 
	{
		g_s_SystemInfor.SetVpdValue = 0;
		sprintf(tx_buff+STRING_START,"err\n");
	}
		
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :CheckPDVoltageProcess
//功能说明： 查询PD  反偏电压
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void CheckPDVoltageProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	sprintf(tx_buff+STRING_START,"%.3f\n",g_s_SystemInfor.SetVpdValue);

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}




//***************************************************
//函数名  :SetLDVCCSwitchProcess
//功能说明：设置老化模式ACC或者APC
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void SetLDVCCSwitchProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
    char ch_str[10] = {0};
	uint8_t ch = 0;
	uint8_t i = 0;
	
    char status[10] = {0};
    char value[10] = {0};
	double val_date = 0;  
	double set_date = 0;

	memset(tx_buff, 0, sizeof(tx_buff));	
			
    ParaSeparate(3,6,Parameter,status,ch_str,value);
	
	ch = atoi(ch_str);
	
	if(ch<1)
		ch = 1;
	
	if(ch>DRIVER_NUM)
		ch = DRIVER_NUM;
		
	if(0 == strcmp((char*)status, "ON"))
    {
		if(g_s_DriverInfor[ch-1].DriverType == DRIVER_TYPE_CURR)
		{
			g_s_DriverInfor[ch-1].PosPWRValue = strtod(value,NULL);
			val_date = g_s_DriverInfor[ch-1].PosPWRValue*g_s_DriverInfor[ch-1].SetPWR_Pos_K+g_s_DriverInfor[ch-1].SetPWR_Pos_B;
			SetFuncValue(12,ch-1,val_date);
			MainTCA9535Bit(TCA9535_REG_OUT_0,ch-1,ENABLE);
			
			g_s_DriverInfor[ch-1].PWRStatus = SYSTEMS_ON;
			g_s_SystemInfor.PowerStatus = SYSTEMS_ON;
		}
		else if(g_s_DriverInfor[ch-1].DriverType == DRIVER_TYPE_VOLT)
		{
			g_s_DriverInfor[ch-1].NegPWRValue = strtod(value,NULL);
			val_date = g_s_DriverInfor[ch-1].NegPWRValue*g_s_DriverInfor[ch-1].SetPWR_Neg_K+g_s_DriverInfor[ch-1].SetPWR_Neg_B;
			MainTCA9535Bit(TCA9535_REG_OUT_0,ch-1,ENABLE);
			
			DriverDelayBlockMs(50);
			set_date = val_date*0.2;
			SetFuncValue(13,ch-1,set_date);
			DriverDelayBlockMs(50);
			set_date = val_date*0.4;
			SetFuncValue(13,ch-1,set_date);
			DriverDelayBlockMs(50);
			set_date = val_date*0.6;
			SetFuncValue(13,ch-1,set_date);
			DriverDelayBlockMs(50);
			set_date = val_date*0.8;
			SetFuncValue(13,ch-1,set_date);
			DriverDelayBlockMs(50);
			set_date = val_date;
			SetFuncValue(13,ch-1,set_date);
			
			g_s_DriverInfor[ch-1].PWRStatus = SYSTEMS_ON;
			g_s_SystemInfor.PowerStatus = SYSTEMS_ON;
		}
		else if(g_s_DriverInfor[ch-1].DriverType == DRIVER_TYPE_BOTH) 
		{
			g_s_DriverInfor[ch-1].PosPWRValue = fabs(strtod(value,NULL));		//BOTH型 设置value 为pos正和 neg负
			g_s_DriverInfor[ch-1].NegPWRValue = -fabs(strtod(value,NULL));
			val_date = g_s_DriverInfor[ch-1].PosPWRValue*g_s_DriverInfor[ch-1].SetPWR_Pos_K+g_s_DriverInfor[ch-1].SetPWR_Pos_B;
			SetFuncValue(12,ch-1,val_date);
			val_date = g_s_DriverInfor[ch-1].NegPWRValue*g_s_DriverInfor[ch-1].SetPWR_Neg_K+g_s_DriverInfor[ch-1].SetPWR_Neg_B;
			SetFuncValue(13,ch-1,val_date);
			MainTCA9535Bit(TCA9535_REG_OUT_0,ch-1,ENABLE);
			
			g_s_DriverInfor[ch-1].PWRStatus = SYSTEMS_ON;
			g_s_SystemInfor.PowerStatus = SYSTEMS_ON;
		}

		sprintf(tx_buff+STRING_START,"ok\n");
    }
	else if(0 == strcmp((char*)status, "OFF"))
    {
		g_s_SystemInfor.PowerStatus = SYSTEMS_OFF;
		
		for(i=0;i<DRIVER_NUM;i++)
		{
			g_s_DriverInfor[i].PWRStatus = SYSTEMS_OFF;
			MainTCA9535Bit(TCA9535_REG_OUT_0,i,DISABLE);
			
			DriverDelayBlockMs(50);
			
			if(g_s_DriverInfor[i].DriverType == DRIVER_TYPE_CURR)
			{
				SetFuncValue(12,i,val_date);
			}
			else if(g_s_DriverInfor[i].DriverType == DRIVER_TYPE_VOLT)
			{
				SetFuncValue(13,i,0);
			}
			else if(g_s_DriverInfor[i].DriverType == DRIVER_TYPE_BOTH) 
			{
				SetFuncValue(12,i,val_date);
				SetFuncValue(13,i,0);
			}
		}
		
		sprintf(tx_buff+STRING_START,"ok\n");
    }
	else 
	{
		sprintf(tx_buff+STRING_START,"err\n");
	}


	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :SetLDAllPowerDownProcess
//功能说明： LD工作电压老化数据查询
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void SetLDAllPowerDownProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	uint8_t i = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));	

	for(i=0;i<DRIVER_NUM;i++)
	{
		MainTCA9535Bit(TCA9535_REG_OUT_0,i,DISABLE);
		
		LDCurrentSetdown_OneStep(&g_s_DriverInfor[i],0);
		
		g_s_DriverInfor[i].SetValue = 0;
		
		g_s_DriverInfor[i].PWRStatus = SYSTEMS_OFF;
	}

	g_s_SystemInfor.PowerStatus = SYSTEMS_OFF;

	sprintf(tx_buff+STRING_START, "ok\n");

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ConfigTimeUp
//功能：配置恒流步进
//说明：
//**************************************************
void ConfigTimeUp(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	char *p_data = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));	
	
	p_data = strtok((char*)Parameter," ");
	if(p_data)
		g_s_SystemInfor.SetDelayNum = atoi((char*)p_data);
	else
		sprintf(tx_buff+STRING_START, "err\n");

	p_data = strtok(NULL," ");
	if(p_data)
		g_s_SystemInfor.SetStepNum = atoi((char*)p_data);
	else
		sprintf(tx_buff+STRING_START,  "err\n");
	
	sprintf(tx_buff+STRING_START,  "ok\n");

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：CheckTimeUp
//功能：查询恒流步进
//说明：
//**************************************************
void CheckTimeUp(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	sprintf(tx_buff+STRING_START,  "%d %d\n",g_s_SystemInfor.SetDelayNum,g_s_SystemInfor.SetStepNum);

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ConfigLDCurrentGear
//功能：配置电流档位
//说明：
//**************************************************
void ConfigLDCurrentGear(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
    uint8_t ChTemp = 0;
    uint8_t GearTemp = 0;
    char Chstr[10] = {0};
    char Gearstr[10] = {0};
	
	memset(tx_buff, 0, sizeof(tx_buff));

    ParaSeparate(2,6,Parameter,Chstr,Gearstr);
    GearTemp = atoi((char*)Gearstr);
    ChTemp = atoi((char*)Chstr);
	
	if(ChTemp<1)
		ChTemp = 1;
	
	if(ChTemp>DRIVER_NUM)
		ChTemp = DRIVER_NUM;
	
	if(GearTemp == 0)
	{
		g_s_DriverInfor[ChTemp-1].LevelGear = 0;
		MainTCA9535Bit(TCA9535_REG_OUT_1,ChTemp-1,DISABLE);
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else if(GearTemp == 1)
	{
		g_s_DriverInfor[ChTemp-1].LevelGear = 1;
		MainTCA9535Bit(TCA9535_REG_OUT_1,ChTemp-1,ENABLE);
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else 
	{
		sprintf(tx_buff+STRING_START, "err\n");
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：CheckLDCurrentGear
//功能：查询电流档位
//说明：
//**************************************************
void CheckLDCurrentGear(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
    uint8_t ChTemp = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
    ChTemp = atoi((char*)Parameter);
	
	if(ChTemp<1)
		ChTemp = 1;
	
	if(ChTemp>DRIVER_NUM)
		ChTemp = DRIVER_NUM;

	sprintf(tx_buff+STRING_START,"%d\n",g_s_DriverInfor[ChTemp-1].LevelGear);
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ConfigLDCurrentGear
//功能：配置电流档位
//说明：
//**************************************************
void ConfigPDCurrentLevel(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
    uint8_t GearTemp = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));

    GearTemp = atoi((char*)Parameter);

	if(GearTemp == 0)
	{
		MpdthTCA9535Bit(2,DISABLE);
		MpdthTCA9535Bit(3,DISABLE);
		g_s_SystemInfor.PDLevel = 0;
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else if(GearTemp == 1)
	{
		MpdthTCA9535Bit(2,ENABLE);
		MpdthTCA9535Bit(3,DISABLE);
		g_s_SystemInfor.PDLevel = 1;
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else if(GearTemp == 2)
	{
		MpdthTCA9535Bit(2,DISABLE);
		MpdthTCA9535Bit(3,ENABLE);
		g_s_SystemInfor.PDLevel = 2;
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else
	{
		sprintf(tx_buff+STRING_START, "err\n");
	}
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ConfigMPDCurrentLevel
//功能：mpd电流档位
//说明：
//**************************************************
void ConfigMPDCurrentLevel(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
    uint8_t GearTemp = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));

    GearTemp = atoi((char*)Parameter);

	if(GearTemp == 0)
	{
		MpdthTCA9535Bit(5,DISABLE);
		MpdthTCA9535Bit(6,DISABLE);
		g_s_SystemInfor.MPDLevel = 0;
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else if(GearTemp == 1)
	{
		MpdthTCA9535Bit(5,ENABLE);
		MpdthTCA9535Bit(6,DISABLE);
		g_s_SystemInfor.MPDLevel = 1;
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else if(GearTemp == 2)
	{
		MpdthTCA9535Bit(5,DISABLE);
		MpdthTCA9535Bit(6,ENABLE);
		g_s_SystemInfor.MPDLevel = 2;
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else
	{
		sprintf(tx_buff+STRING_START, "err\n");
	}
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ConfigSetLimit
//功能：查询电流档位
//说明：
//**************************************************
void ConfigPWRLimit(uint8_t DeviceNum,uint8_t *Parameter)
{
	double val_date = 0; 
	char val_str[8] = {0};
	char type_str[8] = {0};
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;

	memset(tx_buff, 0, sizeof(tx_buff));
	
    ParaSeparate(2,7,Parameter,type_str,val_str);

	val_date = strtod((char*)val_str,NULL);
	
	
	if(0 == strcmp((char*)type_str, "VP"))			//0:v_pos（both board used）
	{
		SetFuncValue(14,0,val_date);
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else if(0 == strcmp((char*)type_str, "CP")) 	 //1:c_pos（both board used）
	{
		SetFuncValue(14,1,val_date);
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else if(0 == strcmp((char*)type_str, "VN"))		//2:v_neg（both board used）
	{
		SetFuncValue(14,2,val_date);
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else if(0 == strcmp((char*)type_str, "CN"))     //3:c_neg（both board used）
	{
		SetFuncValue(14,3,val_date);
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else if(0 == strcmp((char*)type_str, "CL"))		//4:set_cl(voltage board used)
	{
		SetFuncValue(14,4,val_date);
		sprintf(tx_buff+STRING_START, "ok\n");
	}

	sprintf(tx_buff+STRING_START, "ok\n");
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：CheckLDCurrentGear
//功能：查询电流档位
//说明：
//**************************************************
void CheckPDCurrentLevel(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	sprintf(tx_buff+STRING_START,"%d\n",g_s_SystemInfor.PDLevel);
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START); 
}

//**************************************************
//函数：CheckLDCurrentGear
//功能：查询电流档位
//说明：
//**************************************************
void CheckMPDCurrentLevel(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	sprintf(tx_buff+STRING_START,"%d\n",g_s_SystemInfor.MPDLevel);
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START); 
}

//***************************************************
//函数名  :ReadPowerStatusProcess
//功能说明： 查询扫描电流起点，步进，终点
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ReadPowerStatusProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	sprintf(tx_buff+STRING_START,"%d\n",g_s_SystemInfor.PowerStatus);
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START); 
}


//***************************************************
//函数名  :ConfigLDScanCurrentProcess
//功能说明： 配置扫描电流起点，步进，终点
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ConfigLDScanCurrentProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
    char Curstart[8] = {0};
    char Curstep[8] = {0};
    char Curstop[8] = {0};
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

    ParaSeparate(3,7,Parameter,Curstart,Curstep,Curstop);
    g_s_SystemInfor.ScanStart = strtod(Curstart,NULL);
    g_s_SystemInfor.ScanStop = strtod(Curstop,NULL);
    g_s_SystemInfor.ScanStep = strtod(Curstep,NULL);
		
//    if((g_s_DriverInformation[0].CurrentGear !=1)||(g_s_DriverInformation[1].CurrentGear !=1))   //小电流档位最大电流25mA
//    {
//        if(g_s_SystemInfor.ScanStop > 20)
//            g_s_SystemInfor.ScanStop = 20;
//    }
//    else
//    {
//        if(g_s_SystemInfor.ScanStop > 200)
//            g_s_SystemInfor.ScanStop = 200;
//    }

    if(g_s_SystemInfor.ScanStep<0.01)
        g_s_SystemInfor.ScanStep = 0.01;
    if(g_s_SystemInfor.ScanStart>g_s_SystemInfor.ScanStop)
        g_s_SystemInfor.ScanStart = 0;
		
	g_s_SystemInfor.IthScanNum = (g_s_SystemInfor.ScanStop-g_s_SystemInfor.ScanStart)/g_s_SystemInfor.ScanStep+1;
	
	if(g_s_SystemInfor.IthScanNum>ITH_MAX_COUNT)
		g_s_SystemInfor.IthScanNum = ITH_MAX_COUNT;
		
	sprintf(tx_buff+STRING_START,"ok\n");
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START); 
}


//***************************************************
//函数名  :CheckLDScanCurrentProcess
//功能说明： 查询扫描电流起点，步进，终点
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void CheckLDScanCurrentProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	sprintf(tx_buff+STRING_START,"%.1f %.1f %.1f\n",g_s_SystemInfor.ScanStart, g_s_SystemInfor.ScanStep,g_s_SystemInfor.ScanStop);

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START); 
}


//***************************************************
//函数名  :StartLDScanTestProcess
//功能说明： 查询扫描电流起点，步进，终点
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void StartLDScanTestProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint8_t port_num = 0;
	uint8_t pd_num = 0;
	char portstr[8] = {0};
	char pdstr[8] = {0};
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	ParaSeparate(2,7,Parameter,portstr,pdstr);
	
	port_num = atoi((char*)portstr);
	pd_num = atoi((char*)pdstr);
	
	if(port_num>DUT_MAX_NUM)
		port_num = DUT_MAX_NUM;
	else if(port_num<1)
		port_num = 1;
	
	if(pd_num>SAMP_MAX_NUM)
		pd_num = SAMP_MAX_NUM;
	else if(pd_num<1)
		pd_num = 1;
	
	g_s_SystemInfor.ScanPort = port_num;
	g_s_SystemInfor.ScanPD = pd_num;
	
	if( xSemaphoreGive(ScanIthSemphr) != pdTRUE )
	{
		sprintf(tx_buff+STRING_START, "err\n");
		g_s_SystemInfor.IthStatus = 0;
	}
	else
	{
		sprintf(tx_buff+STRING_START, "ok\n");
		g_s_SystemInfor.IthStatus = 1;
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START); 
}

//***************************************************
//函数名  :ReadLDScanStatusProcess
//功能说明： 查询扫描电流起点，步进，终点
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ReadLDScanStatusProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	sprintf(tx_buff+STRING_START,"%d\n",g_s_SystemInfor.IthStatus);

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START); 
}

//***************************************************
//函数名  :ReadLDIthProcess
//功能说明： 读取阈值参数值
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void ReadLDIthProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
	uint16_t cnt = 0;
	uint16_t i = 0;
	uint16_t lengt	= 0;
	uint16_t total = 0;
	uint16_t send_lengt = 0;

	char cnt_str[8] = {0};
	char lengt_str[8] = {0};
//	char str_buff[15] = {0};

	char* p_tx_buff = NULL;
	memset(tx_buff, 0, sizeof(tx_buff));
	
	ParaSeparate(2,7,Parameter,cnt_str,lengt_str);
	
	cnt = atoi((char*)cnt_str);				//获取数据开始点
	lengt = atoi((char*)lengt_str);			//获取数据长度
	
	total = cnt+lengt;
	
	if((total>g_s_SystemInfor.IthScanNum)||(lengt>200)||(lengt==0))
	{
		sprintf(tx_buff+STRING_START, "err\n");
		
		p_tx_buff = tx_buff;
		
		send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
		
		if(ETH_MQTT_INTERFACE==DeviceNum)
			xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
		else	if(RS485_DEVICE_INTERFACE==DeviceNum)
			RS485SendFrame(send_lengt,tx_buff+STRING_START); 
	}
	else 
	{
		for(;cnt<=total;cnt++)
		{
			if(ETH_MQTT_INTERFACE==DeviceNum)
			{
				sprintf(tx_buff+STRING_START+i*6, "%c%c%c%c%c%c",(char)(IthVoltage[cnt]>>8),(char)IthVoltage[cnt],(char)(IthCurrent[cnt]>>8),(char)IthCurrent[cnt],(char)(IthPD[cnt]>>8),(char)IthPD[cnt]);
				i++;
			}
			else if(RS485_DEVICE_INTERFACE==DeviceNum)
			{
				sprintf(tx_buff, "%c%c%c%c%c%c",(char)(IthVoltage[cnt]>>8),(char)IthVoltage[cnt],(char)(IthCurrent[cnt]>>8),(char)IthCurrent[cnt],(char)(IthPD[cnt]>>8),(char)IthPD[cnt]);
				RS485SendFrame(6,tx_buff);
			}
		}
		
		if(ETH_MQTT_INTERFACE==DeviceNum)
		{
			tx_buff[0] = lengt*6>>8;
			tx_buff[1] = lengt*6;
			
			p_tx_buff = tx_buff;
			xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
		}
	}
}

//**************************************************
//函数：ReadLDCurrentProcess
//功能：读取单路LD电流值
//说明：
//**************************************************
void ReadLDCurrentProcess(uint8_t DeviceNum,uint8_t *Parameter)
{	
	int16_t vol_int16 = 0;
	int16_t cur_int16 = 0;
	int16_t data = 0;
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	if( xSemaphoreGive(SampDutSemphr) != pdTRUE )
		sprintf(tx_buff+STRING_START, "err\n");
	else
	{
		while(eTaskGetState(SampDutHandle) != eSuspended)
			vTaskDelay(pdMS_TO_TICKS(1));
		
		data = atoi((char*)Parameter);
			
		if((data>0)&&(data<=DUT_MAX_NUM))
		{
			g_s_SystemInfor.CurrPort = data;
			
			SwitchDUTSamp(g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].PortNum);	
			
			DriverDelayBlockMs(10);
			
			if(I2C_NO_ERR == ReadSampDUT(64,1,NULL,&cur_int16,&vol_int16))
			{
				if(g_s_DriverInfor[g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].DriverNum-1].LevelGear == 0)
					g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Curr_GetValue  = (double)cur_int16*g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_1st_K+g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_1st_B;
				else if(g_s_DriverInfor[g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].DriverNum-1].LevelGear == 1)
					g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Curr_GetValue  = (double)cur_int16*g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_2nd_K+g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_2nd_B;
			}
			
			sprintf(tx_buff+STRING_START, "%.3f\n",g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Curr_GetValue);
		}
		else 
		{
			sprintf(tx_buff+STRING_START, "err\n");
		}
		
		if(eTaskGetState(SampDutHandle) == eSuspended)
			vTaskResume(SampDutHandle);
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}


//**************************************************
//函数：ReadLDVoltageProcess
//功能：读取单路LD电压值
//说明：
//**************************************************
void ReadLDVoltageProcess(uint8_t DeviceNum,uint8_t *Parameter)
{	
	int16_t  vol_int16 = 0;
	int16_t  cur_int16 = 0;
	int16_t data = 0;
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	if( xSemaphoreGive(SampDutHandle) != pdTRUE )
		sprintf(tx_buff+STRING_START, "err\n");
	else
	{
		while(eTaskGetState(SampDutHandle) != eSuspended)
			vTaskDelay(pdMS_TO_TICKS(1));
		
		data = atoi((char*)Parameter);
			
		if((data>0)&&(data<=DUT_MAX_NUM))
		{
			g_s_SystemInfor.CurrPort = data;
			
			SwitchDUTSamp(g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].PortNum);	
			
			DriverDelayBlockMs(10);
			
			if(I2C_NO_ERR == ReadSampDUT(64,1,NULL,&cur_int16,&vol_int16))
			{
				g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Volt_GetValue = (double)vol_int16*g_s_SystemInfor.DutVoltGetK+g_s_SystemInfor.DutVoltGetB;
			}
			
			sprintf(tx_buff+STRING_START, "%.3f\n",g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Volt_GetValue);
		}
		else 
		{
			sprintf(tx_buff+STRING_START, "err\n");
		}
		
		if(eTaskGetState(SampDutHandle) == eSuspended)
			vTaskResume(SampDutHandle);
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}


//**************************************************
//函数：ReadPDCurrentProcess
//功能：读取单路mPD电流值
//说明：
//**************************************************
void ReadPDCurrentProcess(uint8_t DeviceNum,uint8_t *Parameter)
{	
	int16_t  pd_int16 = 0;
	int16_t  mpd_int16 = 0;

	int16_t data = 0;
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	if( xSemaphoreGive(SampMonitSemphr) != pdTRUE )
		sprintf(tx_buff+STRING_START, "err\n");
	else
	{
		while(eTaskGetState(SampMonitHandle) != eSuspended)
			vTaskDelay(pdMS_TO_TICKS(1));
		
		data = atoi((char*)Parameter);
			
		if((data>0)&&(data<=SAMP_MAX_NUM))
		{
			g_s_SystemInfor.CurrSamp = data;
			
			SwitchMPDTH(g_s_DutputInfor[g_s_SystemInfor.CurrSamp-1].PortNum);	
			
			DriverDelayBlockMs(10);
			
			if(I2C_NO_ERR == ReadPDandMPD(64,1,NULL,&pd_int16,&mpd_int16))
			{
				if(g_s_SystemInfor.PDLevel == 0)
					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue = (double)pd_int16*g_s_SystemInfor.PDGet_1st_K+g_s_SystemInfor.PDGet_1st_B;
				else if(g_s_SystemInfor.PDLevel == 1)
					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue = (double)pd_int16*g_s_SystemInfor.PDGet_2nd_K+g_s_SystemInfor.PDGet_2nd_B;
				else if(g_s_SystemInfor.PDLevel == 2)
					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue = (double)pd_int16*g_s_SystemInfor.PDGet_3rd_K+g_s_SystemInfor.PDGet_3rd_B;
				
//				g_s_DutputInfor[g_s_SystemInformation.CurrPort-1].PDCurrentGet = fabs(g_s_DutputInfor[g_s_SystemInformation.CurrPort-1].PDCurrentGet);		
			}
			
			sprintf(tx_buff+STRING_START, "%.3f\n",g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue);
		}
		else 
		{
			sprintf(tx_buff+STRING_START, "err\n");
		}
		
		if(eTaskGetState(SampMonitHandle) == eSuspended)
			vTaskResume(SampMonitHandle);
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ReadBitResult
//功能：查询单路结果
//说明：
//**************************************************
void ReadDutBitResult(uint8_t DeviceNum,uint8_t *Parameter)
{	
	int16_t  vol_int16 = 0;
	int16_t  cur_int16 = 0;
	int16_t	 data = 0;
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	if( xSemaphoreGive(SampDutSemphr) != pdTRUE )
	{
		sprintf(tx_buff+STRING_START, "err\n");
		
		p_tx_buff = tx_buff;
		
		send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
		
		if(ETH_MQTT_INTERFACE==DeviceNum)
			xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
		else	if(RS485_DEVICE_INTERFACE==DeviceNum)
			RS485SendFrame(send_lengt,tx_buff+STRING_START);
	}
	else
	{
		while(eTaskGetState(SampDutHandle) != eSuspended)
			vTaskDelay(pdMS_TO_TICKS(1));
		
		data = atoi((char*)Parameter);
			
		if((data>0)&&(data<=DUT_MAX_NUM))
		{
			g_s_SystemInfor.CurrPort = data;

			SwitchDUTSamp(g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].PortNum);
			
			DriverDelayBlockMs(10);
			
			if(I2C_NO_ERR == ReadSampDUT(64,1,NULL,&cur_int16,&vol_int16))
			{
				g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Volt_GetValue = (double)vol_int16*g_s_SystemInfor.DutVoltGetK+g_s_SystemInfor.DutVoltGetB;
					
				if(g_s_DriverInfor[g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].DriverNum-1].LevelGear == 0)
					g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Curr_GetValue  = (double)cur_int16*g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_1st_K+g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_1st_B;
				else if(g_s_DriverInfor[g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].DriverNum-1].LevelGear == 1)
					g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Curr_GetValue  = (double)cur_int16*g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_2nd_K+g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_2nd_B;				
			}
			
			vol_int16 = g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Volt_GetValue*100;				//单位值 10mV
			cur_int16 = g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Curr_GetValue*10;			//单位值 100uA
	
			sprintf(tx_buff+STRING_START, "%c%c%c%c",(char)(vol_int16>>8),(char)vol_int16,(char)(cur_int16>>8),(char)cur_int16);
			
			if(eTaskGetState(SampDutHandle) == eSuspended)
				vTaskResume(SampDutHandle);
			
			p_tx_buff = tx_buff;
			
			send_lengt = 4;tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
			
			if(ETH_MQTT_INTERFACE==DeviceNum)
				xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
			else	if(RS485_DEVICE_INTERFACE==DeviceNum)
				RS485SendFrame(4,tx_buff+STRING_START);
		}
		else 
		{
			sprintf(tx_buff+STRING_START, "err\n");
			
			if(eTaskGetState(SampDutHandle) == eSuspended)
				vTaskResume(SampDutHandle);
			
			p_tx_buff = tx_buff;
			
			send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
			
			if(ETH_MQTT_INTERFACE==DeviceNum)
				xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
			else	if(RS485_DEVICE_INTERFACE==DeviceNum)
				RS485SendFrame(send_lengt,tx_buff+STRING_START);
		}
	}
}

//**************************************************
//函数：ReadAllResult
//功能：查询单路电流值
//说明：
//**************************************************
void ReadDutAllResult(uint8_t DeviceNum,uint8_t *Parameter)
{	
	char cnt = 0;
	int16_t volt_int = 0;
	int16_t curr_int = 0;
	uint16_t lengt	= DUT_MAX_NUM;

	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
		
	for(cnt=0;cnt<DUT_MAX_NUM;cnt++)
	{		
		volt_int = (int16_t)(g_s_DutputInfor[cnt].Volt_GetValue*100);			//单位值 10mV
		curr_int = (int16_t)(g_s_DutputInfor[cnt].Curr_GetValue*10);			//单位值 100uA

		if(ETH_MQTT_INTERFACE==DeviceNum)
		{
			sprintf(tx_buff+STRING_START+cnt*4, "%c%c%c%c",(char)(volt_int>>8),(char)volt_int,(char)(curr_int>>8),(char)curr_int);
		}
		else if(RS485_DEVICE_INTERFACE==DeviceNum)
		{
			sprintf(tx_buff, "%c%c%c%c",(char)(volt_int>>8),(char)volt_int,(char)(curr_int>>8),(char)curr_int);
			RS485SendFrame(4,tx_buff);
		}
	}
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
	{
		tx_buff[0] = lengt*4>>8;
		tx_buff[1] = lengt*4;
		
		p_tx_buff = tx_buff;
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	}
}


//**************************************************
//函数：ReadBitResult
//功能：查询单路结果
//说明：
//**************************************************
void ReadMonitBitResult(uint8_t DeviceNum,uint8_t *Parameter)
{	
	int16_t  pd_int16 = 0;
	int16_t  mpd_int16 = 0;
	int16_t	 data = 0;
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	if( xSemaphoreGive(SampMonitSemphr) != pdTRUE )
	{
		sprintf(tx_buff+STRING_START, "err\n");
		
		p_tx_buff = tx_buff;
		
		send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
		
		if(ETH_MQTT_INTERFACE==DeviceNum)
			xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
		else	if(RS485_DEVICE_INTERFACE==DeviceNum)
			RS485SendFrame(send_lengt,tx_buff+STRING_START);
	}
	else
	{
		while(eTaskGetState(SampMonitHandle) != eSuspended)
			vTaskDelay(pdMS_TO_TICKS(1));
		
		data = atoi((char*)Parameter);
			
		if((data>0)&&(data<=SAMP_MAX_NUM))
		{
			g_s_SystemInfor.CurrSamp = data;

			SwitchMPDTH(g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].SampNum);
			
			DriverDelayBlockMs(10);
			
			if(I2C_NO_ERR == ReadPDandMPD(64,1,NULL,&pd_int16,&mpd_int16))
			{
				if(g_s_SystemInfor.PDLevel == 0)
					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue = (double)pd_int16*g_s_SystemInfor.PDGet_1st_K+g_s_SystemInfor.PDGet_1st_B;
				else if(g_s_SystemInfor.PDLevel == 1)
					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue = (double)pd_int16*g_s_SystemInfor.PDGet_2nd_K+g_s_SystemInfor.PDGet_2nd_B;
				else if(g_s_SystemInfor.PDLevel == 2)
					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue = (double)pd_int16*g_s_SystemInfor.PDGet_3rd_K+g_s_SystemInfor.PDGet_3rd_B;
				
				if(g_s_SystemInfor.MPDLevel == 0)
					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].MPD_GetValue = (double)mpd_int16*g_s_SystemInfor.MPDGet_1st_K+g_s_SystemInfor.MPDGet_1st_B;
				else if(g_s_SystemInfor.MPDLevel == 1)
					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].MPD_GetValue = (double)mpd_int16*g_s_SystemInfor.MPDGet_2nd_K+g_s_SystemInfor.MPDGet_2nd_B;
				else if(g_s_SystemInfor.MPDLevel == 2)
					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].MPD_GetValue = (double)mpd_int16*g_s_SystemInfor.MPDGet_3rd_K+g_s_SystemInfor.MPDGet_3rd_B;	
			}
			
			pd_int16 = g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue*10;				//单位值 0.1uA
			mpd_int16 = g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].MPD_GetValue*10;			//单位值 0.1uA
	
			sprintf(tx_buff+STRING_START, "%c%c%c%c",(char)(pd_int16>>8),(char)pd_int16,(char)(mpd_int16>>8),(char)mpd_int16);
			
			if(eTaskGetState(SampMonitHandle) == eSuspended)
				vTaskResume(SampMonitHandle);
			
			p_tx_buff = tx_buff;
			
			send_lengt = 4;tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
			
			if(ETH_MQTT_INTERFACE==DeviceNum)
				xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
			else	if(RS485_DEVICE_INTERFACE==DeviceNum)
				RS485SendFrame(4,tx_buff+STRING_START);
		}
		else 
		{
			sprintf(tx_buff+STRING_START, "err\n");
			
			if(eTaskGetState(SampMonitHandle) == eSuspended)
				vTaskResume(SampMonitHandle);
			
			p_tx_buff = tx_buff;
			
			send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
			
			if(ETH_MQTT_INTERFACE==DeviceNum)
				xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
			else	if(RS485_DEVICE_INTERFACE==DeviceNum)
				RS485SendFrame(send_lengt,tx_buff+STRING_START);
		}
	}
}

//**************************************************
//函数：ReadAllResult
//功能：查询单路电流值
//说明：
//**************************************************
void ReadMonitAllResult(uint8_t DeviceNum,uint8_t *Parameter)
{	
	char cnt = 0;
	int16_t pd_int = 0;
	int16_t mpd_int = 0;
	uint16_t lengt	= SAMP_MAX_NUM;

	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
		
	for(cnt=0;cnt<SAMP_MAX_NUM;cnt++)
	{		
		pd_int = (int16_t)(g_s_SampInInfor[cnt].PD_GetValue*10);				//单位值 0.1uA
		mpd_int = (int16_t)(g_s_SampInInfor[cnt].MPD_GetValue*10);				//单位值 0.1uA					

		if(ETH_MQTT_INTERFACE==DeviceNum)
		{
			sprintf(tx_buff+STRING_START+cnt*4,"%c%c%c%c",(char)(pd_int>>8),(char)pd_int,(char)(mpd_int>>8),(char)mpd_int);
		}
		else if(RS485_DEVICE_INTERFACE==DeviceNum)
		{
			sprintf(tx_buff+STRING_START,"%c%c%c%c",(char)(pd_int>>8),(char)pd_int,(char)(mpd_int>>8),(char)mpd_int);
			RS485SendFrame(4,tx_buff);
		}
	}
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
	{
		tx_buff[0] = lengt*4>>8;
		tx_buff[1] = lengt*4;
		
		p_tx_buff = tx_buff;
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	}
}


//**************************************************
//函数：WriteBurnInBoard
//功能：设置IDN
//说明：	
//**************************************************
void WriteBurnInBoard(uint8_t DeviceNum,uint8_t *Parameter)
{
    char at24add[11] = {0};
    char at24len[11] = {0};
    uint8_t at24str[11] = {0};
	uint8_t addint = 0;
	uint8_t lenint = 0;
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

    ParaSeparate(3,10,Parameter,at24add,at24len,at24str);
		
    addint = atoi(at24add);
    lenint = atoi(at24len);
	
	if(I2C_CHIP_ERR_NONE == WriteE2promInfor(7,addint,lenint,at24str))
		sprintf(tx_buff+STRING_START, "ok\n");
	else
		sprintf(tx_buff+STRING_START, "err\n");
		
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ReadBurnInBoard
//功能：调用IDN编号
//说明：
//**************************************************
void ReadBurnInBoard(uint8_t DeviceNum,uint8_t *Parameter)
{	
    char at24add[11] = {0};
    char at24len[11] = {0};
    uint8_t at24str[11] = {0};
	uint8_t addint = 0;
	uint8_t lenint = 0;
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

    ParaSeparate(2,10,Parameter,at24add,at24len);
		
    addint = atoi(at24add);
    lenint = atoi(at24len);
	
	if(I2C_CHIP_ERR_NONE == ReadE2promInfor(7,addint,lenint,at24str))
		sprintf(tx_buff+STRING_START, "%s\n",at24str);
	else
		sprintf(tx_buff+STRING_START, "err\n");
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ReadBurnInBoard
//功能：调用IDN编号
//说明：
//**************************************************
void ReadTempInf(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	ReadTMP112Temp(MAIN_I2C_CHN,&g_s_SystemInfor.Temp_1st,&g_s_SystemInfor.Temp_2nd);
	ReadTMP112Temp(g_s_DriverInfor[0].DriverNum,&g_s_DriverInfor[0].Temp_1st,&g_s_DriverInfor[0].Temp_2nd);
	ReadTMP112Temp(g_s_DriverInfor[1].DriverNum,&g_s_DriverInfor[1].Temp_1st,&g_s_DriverInfor[1].Temp_2nd);
	ReadTMP112Temp(g_s_DriverInfor[2].DriverNum,&g_s_DriverInfor[2].Temp_1st,&g_s_DriverInfor[2].Temp_2nd);
	ReadTMP112Temp(g_s_DriverInfor[3].DriverNum,&g_s_DriverInfor[3].Temp_1st,&g_s_DriverInfor[3].Temp_2nd);
	ReadTMP112Temp(g_s_DriverInfor[4].DriverNum,&g_s_DriverInfor[4].Temp_1st,&g_s_DriverInfor[4].Temp_2nd);
	ReadTMP112Temp(g_s_DriverInfor[5].DriverNum,&g_s_DriverInfor[5].Temp_1st,&g_s_DriverInfor[5].Temp_2nd);

	sprintf(tx_buff+STRING_START, "%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", \
	g_s_SystemInfor.Temp_1st, \
	g_s_SystemInfor.Temp_2nd, \
	g_s_DriverInfor[0].Temp_1st, \
	g_s_DriverInfor[0].Temp_2nd, \
	g_s_DriverInfor[1].Temp_1st, \
	g_s_DriverInfor[1].Temp_2nd, \
	g_s_DriverInfor[2].Temp_1st, \
	g_s_DriverInfor[2].Temp_2nd, \
	g_s_DriverInfor[3].Temp_1st, \
	g_s_DriverInfor[3].Temp_2nd, \
	g_s_DriverInfor[4].Temp_1st, \
	g_s_DriverInfor[4].Temp_2nd, \
	g_s_DriverInfor[5].Temp_1st, \
	g_s_DriverInfor[5].Temp_2nd);

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ReadBoardLineInf
//功能：调用IDN编号
//说明：
//**************************************************
void ReadBoardLineInf(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	sprintf(tx_buff+STRING_START, "%d\n",g_s_SystemInfor.SystemErrType);

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ReadBurnInBoard
//功能：调用IDN编号
//说明：
//**************************************************
void ReadIICBoardInf(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	sprintf(tx_buff+STRING_START, "%d\n",g_s_SystemInfor.I2cSlaveStatus);

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ConfigLDCurrentGear
//功能：配置电流档位
//说明：
//**************************************************
void SetAutoOutage(uint8_t DeviceNum,uint8_t *Parameter)
{
    char status = 0;
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

    status = atoi((char*)Parameter);

	if(status == 0)
	{
		g_s_SystemInfor.SystemAutoOutage = SYSTEMS_OFF;
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else if(status == 1)
	{
		g_s_SystemInfor.SystemAutoOutage = SYSTEMS_ON;
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else
	{
		sprintf(tx_buff+STRING_START, "err\n");
	}
	
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：ConfigLDCurrentGear
//功能：配置电流档位
//说明：
//**************************************************
void SetOnChipFlag(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

//		if(GetStrLength((int8_t*)(Parameter)) == 8)
//		{
			g_s_SystemInfor.OnChipFlag = ((uint64_t)Parameter[7])<<56|((uint64_t)Parameter[6])<<48| \
																				 ((uint64_t)Parameter[5])<<40|((uint64_t)Parameter[4])<<32| \
																				 ((uint64_t)Parameter[3])<<24|((uint64_t)Parameter[2])<<16| \
																				 ((uint64_t)Parameter[1])<<8|((uint64_t)Parameter[0]);
			
			sprintf(tx_buff+STRING_START, "ok\n");
//		}
//		else
//		{
//			sprintf(send, "err\n");
//		}

		
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}


//**************************************************
//函数：ControlLedStatus
//功能：配置LED输出状态
//说明：
//**************************************************
void ControlLedStatus(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
    char ch_str[10] = {0};
    char status[10] = {0};
	
	memset(tx_buff, 0, sizeof(tx_buff));

    ParaSeparate(2,7,Parameter,status,ch_str);
	
	sprintf(tx_buff+STRING_START, "ok\n");
		
	if(0 == strcmp((char*)status, "ON"))
    {
		if(0 == strcmp((char*)ch_str, "LED1"))
		{
			if(eTaskGetState(FirstLedHandle) != eSuspended)
				xSemaphoreGive(FirstLedSemphr);
			
			while(eTaskGetState(FirstLedHandle) != eSuspended)
				vTaskDelay(pdMS_TO_TICKS(1));
			
			SET_LED_1ST
		}
		else if(0 == strcmp((char*)ch_str, "LED2"))
		{
			if(eTaskGetState(SecondLedHandle) != eSuspended)
				xSemaphoreGive(SecondLedSemphr);
			
			while(eTaskGetState(SecondLedHandle) != eSuspended)
				vTaskDelay(pdMS_TO_TICKS(1));
			
			SET_LED_2ND
		}
		else if(0 == strcmp((char*)ch_str, "LED3"))
		{
			if(eTaskGetState(ThirdLedHandle) != eSuspended)
				xSemaphoreGive(ThirdLedSemphr);
			
			while(eTaskGetState(ThirdLedHandle) != eSuspended)
				vTaskDelay(pdMS_TO_TICKS(1));
			
			SET_LED_3RD
		}
	}
	else if(0 == strcmp((char*)status, "OFF"))
	{
		if(0 == strcmp((char*)ch_str, "LED1"))
		{
			if(eTaskGetState(FirstLedHandle) != eSuspended)
				xSemaphoreGive(FirstLedSemphr);
			
			while(eTaskGetState(FirstLedHandle) != eSuspended)
				vTaskDelay(pdMS_TO_TICKS(1));
			
			RESET_LED_1ST
		}
		else if(0 == strcmp((char*)ch_str, "LED2"))
		{
			if(eTaskGetState(SecondLedHandle) != eSuspended)
				xSemaphoreGive(SecondLedSemphr);
			
			while(eTaskGetState(SecondLedHandle) != eSuspended)
				vTaskDelay(pdMS_TO_TICKS(1));
			
			RESET_LED_2ND
		}
		else if(0 == strcmp((char*)ch_str, "LED3"))
		{
			if(eTaskGetState(ThirdLedHandle) != eSuspended)
				xSemaphoreGive(ThirdLedSemphr);
			
			while(eTaskGetState(ThirdLedHandle) != eSuspended)
				vTaskDelay(pdMS_TO_TICKS(1));
			
			RESET_LED_3RD
		}
	}
	else if(0 == strcmp((char*)status, "BLINK"))
	{
		if(0 == strcmp((char*)ch_str, "LED1"))
		{
			if(eTaskGetState(FirstLedHandle) == eSuspended)
				vTaskResume(FirstLedHandle);
		}
		else if(0 == strcmp((char*)ch_str, "LED2"))
		{
			if(eTaskGetState(SecondLedHandle) == eSuspended)
				vTaskResume(SecondLedHandle);
		}
		else if(0 == strcmp((char*)ch_str, "LED3"))
		{
			if(eTaskGetState(ThirdLedHandle) == eSuspended)
				vTaskResume(ThirdLedHandle);
		}
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}


//**************************************************
//函数：TranspondAscii
//功能：转发ascii
//说明：
//**************************************************
void TransBandSpeed(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	uint16_t bandspeed = 0;
	UartConfigInfo UartTranspond;
	
	bandspeed = atoi((char*)Parameter);
	
	memset(tx_buff, 0, sizeof(tx_buff));

	//转发串口
	UartTranspond.BandSpeed = bandspeed;
	UartTranspond.RxInterruptEnable = UART_RX_INTERRUPT_EN;
	UartTranspond.RxPreInterruptPrioty = 7;
	UartTranspond.RxSubInterruptPrioty = 0;
	DriverTranInitial(UartTranspond,USART3);

	sprintf(tx_buff+STRING_START, "ok\n");

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}


//**************************************************
//函数：TranspondAscii
//功能：转发ascii
//说明：
//**************************************************
void TranspondAscii(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	memset(TranRxBuffer, 0, sizeof(TranRxBuffer));

	g_s_ComTransimt.Com_Device = DeviceNum;
	g_s_ComTransimt.Ascii_Hex = ASCII_TRANS_REVEIVE;
	g_s_ComTransimt.RxIndex = 0;

	sprintf(tx_buff+STRING_START, "ok\n");

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
	
	send_lengt = (uint16_t)strlen((char*)Parameter);
	
	Parameter[send_lengt-1] = 0x0D;   //温控器最后以/r结尾 0x0D
	
	TransSendFrame(send_lengt,(char*)Parameter);
}


//**************************************************
//函数：TranspondAscii
//功能：转发ascii
//说明：
//**************************************************
void TranspondHex(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
    char leng_str[11] = {0};
    char tran_str[51] = {0};
    char tran_hex[30] = {0};

	uint16_t lenint = 0;
	
    ParaSeparate(2,50,Parameter,leng_str,tran_str);
	
	lenint = atoi(leng_str);
	
	memset(tx_buff, 0, sizeof(tx_buff));
	memset(TranRxBuffer, 0, sizeof(TranRxBuffer));
	memset(TranHexString, 0, sizeof(TranHexString));
	
	string2hex((char*)tran_str,tran_hex);

	g_s_ComTransimt.Com_Device = DeviceNum;
    g_s_ComTransimt.Ascii_Hex = HEX_TRANS_REVEIVE;
	g_s_ComTransimt.Hex_Lenght = lenint;
	g_s_ComTransimt.RxIndex = 0;

	sprintf(tx_buff+STRING_START, "ok\n");

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
	
	send_lengt = (uint16_t)strlen(tran_str)/2;
	
	TransSendFrame(send_lengt,tran_hex);
}


//***************************************************
//函数名  :SetCalDataProcess
//功能说明： 设置相关校准参数
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void SetCalDataProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
    char PortData = 0;
    char Para0[21] = {0};        //校准系数的类型
    char Para1[21] = {0};        //校准通道
    char Para2[21] = {0};        //系数K
    char Para3[21] = {0};        //系数B
	
	uint8_t reg_flag = 0;
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

    ParaSeparate(4,20,Parameter,Para0,Para1,Para2,Para3);

    PortData = atoi(Para1);
	
	if(PortData > DUT_MAX_NUM)
		PortData = DUT_MAX_NUM;
	
	if(PortData < 1)
		PortData = 1;
	
	sprintf(tx_buff+STRING_START,"ok\n");

	if(0 == strcmp((char*)Para0, "LDCurrentSet"))
    {
		if(g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].BitStatus == DRIVER_BIT_VALID)
		{
			g_s_DutputInfor[PortData-1].SetValue_2nd_K = strtod(Para2,NULL);
			g_s_DutputInfor[PortData-1].SetValue_2nd_B = strtod(Para3,NULL);
			reg_flag = DRI_FLAG_VALUE;
				
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, g_s_DutputInfor[PortData-1].DriverSon*DUT_ADDR_SIZE+16*SET_2ND_ADDR,8,(uint8_t *)&g_s_DutputInfor[PortData-1].SetValue_2nd_K);
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, g_s_DutputInfor[PortData-1].DriverSon*DUT_ADDR_SIZE+16*SET_2ND_ADDR+8,8,(uint8_t *)&g_s_DutputInfor[PortData-1].SetValue_2nd_B);
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, DRI_FLAG_ADDR,1,&reg_flag);
		}
		else 
		{
			sprintf(tx_buff+STRING_START,"err\n");
		}
    }
	else if(0 == strcmp((char*)Para0, "LDCurrentGet"))
    {
		if(g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].BitStatus == DRIVER_BIT_VALID)
		{
			g_s_DutputInfor[PortData-1].GetValue_2nd_K = strtod(Para2,NULL);
			g_s_DutputInfor[PortData-1].GetValue_2nd_B = strtod(Para3,NULL);
			reg_flag = DRI_FLAG_VALUE;
			
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, g_s_DutputInfor[PortData-1].DriverSon*DUT_ADDR_SIZE+16*GET_2ND_ADDR,8,(uint8_t *)&g_s_DutputInfor[PortData-1].GetValue_2nd_K);
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, g_s_DutputInfor[PortData-1].DriverSon*DUT_ADDR_SIZE+16*GET_2ND_ADDR+8,8,(uint8_t *)&g_s_DutputInfor[PortData-1].GetValue_2nd_B);
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, DRI_FLAG_ADDR,1,&reg_flag);
		}
		else 
		{
			sprintf(tx_buff+STRING_START,"err\n");
		}
    }
	else if(0 == strcmp((char*)Para0, "LDCurrentMinSet"))
    {
		if(g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].BitStatus == DRIVER_BIT_VALID)
		{
			g_s_DutputInfor[PortData-1].SetValue_1st_K = strtod(Para2,NULL);
			g_s_DutputInfor[PortData-1].SetValue_1st_B = strtod(Para3,NULL);
			reg_flag = DRI_FLAG_VALUE;
				
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, g_s_DutputInfor[PortData-1].DriverSon*DUT_ADDR_SIZE+16*SET_1ST_ADDR,8,(uint8_t *)&g_s_DutputInfor[PortData-1].SetValue_1st_K);
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, g_s_DutputInfor[PortData-1].DriverSon*DUT_ADDR_SIZE+16*SET_1ST_ADDR+8,8,(uint8_t *)&g_s_DutputInfor[PortData-1].SetValue_1st_B);
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, DRI_FLAG_ADDR,1,&reg_flag);
		}
		else 
		{
			sprintf(tx_buff+STRING_START,"err\n");
		}
    }
	else if(0 == strcmp((char*)Para0, "LDCurrentMinGet"))
    {
		if(g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].BitStatus == DRIVER_BIT_VALID)
		{
			g_s_DutputInfor[PortData-1].GetValue_1st_K = strtod(Para2,NULL);
			g_s_DutputInfor[PortData-1].GetValue_1st_B = strtod(Para3,NULL);
			reg_flag = DRI_FLAG_VALUE;
				
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, g_s_DutputInfor[PortData-1].DriverSon*DUT_ADDR_SIZE+16*GET_1ST_ADDR,8,(uint8_t *)&g_s_DutputInfor[PortData-1].GetValue_1st_K);
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, g_s_DutputInfor[PortData-1].DriverSon*DUT_ADDR_SIZE+16*GET_1ST_ADDR+8,8,(uint8_t *)&g_s_DutputInfor[PortData-1].GetValue_1st_B);
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, DRI_FLAG_ADDR,1,&reg_flag);
		}
		else 
		{
			sprintf(tx_buff+STRING_START,"err\n");
		}
    }
	else if(0 == strcmp((char*)Para0, "PosPWRVoltage"))
    {
		if(g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].BitStatus == DRIVER_BIT_VALID)
		{
			g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Pos_K = strtod(Para2,NULL);
			g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Pos_B = strtod(Para3,NULL);
			reg_flag = DRI_FLAG_VALUE;
			
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum,PWR_CAL_ADDR,8,(uint8_t*)(&g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Pos_K));
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum,PWR_CAL_ADDR+8,8,(uint8_t*)(&g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Pos_B));
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, DRI_FLAG_ADDR,1,&reg_flag);
		}
		else 
		{
			sprintf(tx_buff+STRING_START,"err\n");
		}
    }
	else if(0 == strcmp((char*)Para0, "NegPWRVoltage"))
    {
		if(g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].BitStatus == DRIVER_BIT_VALID)
		{
			g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Neg_K = strtod(Para2,NULL);
			g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Neg_B = strtod(Para3,NULL);
			reg_flag = DRI_FLAG_VALUE;
			
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum,PWR_CAL_ADDR+16,8,(uint8_t*)(&g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Neg_K));
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum,PWR_CAL_ADDR+16+8,8,(uint8_t*)(&g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Neg_B));
			vTaskDelay(pdMS_TO_TICKS(5));
			WriteE2promInfor(g_s_DutputInfor[PortData-1].DriverNum, DRI_FLAG_ADDR,1,&reg_flag);
		}
		else 
		{
			sprintf(tx_buff+STRING_START,"err\n");
		}
    }
	else if(0 == strcmp((char*)Para0, "PDCurrentGet1"))
    {
        g_s_SystemInfor.PDGet_1st_K = strtod(Para2,NULL);
        g_s_SystemInfor.PDGet_1st_B = strtod(Para3,NULL);
		reg_flag = MAIN_FLAG_VALUE;
			
		WriteE2promInfor(MAIN_I2C_CHN, PD_1ST_ADDR*16,8, (uint8_t *)& g_s_SystemInfor.PDGet_1st_K);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, PD_1ST_ADDR*16+8,8, (uint8_t *)&g_s_SystemInfor.PDGet_1st_B);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MAIN_FLAG_ADDR,1,&reg_flag);
    }
	else if(0 == strcmp((char*)Para0, "PDCurrentGet2"))
    {
        g_s_SystemInfor.PDGet_2nd_K = strtod(Para2,NULL);
        g_s_SystemInfor.PDGet_2nd_B = strtod(Para3,NULL);
		reg_flag = MAIN_FLAG_VALUE;
			
		WriteE2promInfor(MAIN_I2C_CHN, PD_2ND_ADDR*16,8, (uint8_t *)& g_s_SystemInfor.PDGet_2nd_K);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, PD_2ND_ADDR*16+8,8, (uint8_t *)&g_s_SystemInfor.PDGet_2nd_B);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MAIN_FLAG_ADDR,1,&reg_flag);
    }
	else if(0 == strcmp((char*)Para0, "PDCurrentGet3"))
    {
        g_s_SystemInfor.PDGet_3rd_K = strtod(Para2,NULL);
        g_s_SystemInfor.PDGet_3rd_B = strtod(Para3,NULL);
		reg_flag = MAIN_FLAG_VALUE;
			
		WriteE2promInfor(MAIN_I2C_CHN, PD_3RD_ADDR*16,8, (uint8_t *)& g_s_SystemInfor.PDGet_3rd_K);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, PD_3RD_ADDR*16+8,8, (uint8_t *)&g_s_SystemInfor.PDGet_3rd_B);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MAIN_FLAG_ADDR,1,&reg_flag);
    }
	else if(0 == strcmp((char*)Para0, "GetLDVoltage"))
    {
        g_s_SystemInfor.DutVoltGetK = strtod(Para2,NULL);
        g_s_SystemInfor.DutVoltGetB = strtod(Para3,NULL);
		reg_flag = MAIN_FLAG_VALUE;
			
		WriteE2promInfor(MAIN_I2C_CHN, DUT_VOLT_ADDR*16,8, (uint8_t *)& g_s_SystemInfor.DutVoltGetK);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, DUT_VOLT_ADDR*16+8,8, (uint8_t *)&g_s_SystemInfor.DutVoltGetB);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MAIN_FLAG_ADDR,1,&reg_flag);
    }
	else if(0 == strcmp((char*)Para0, "SetPDVolt"))
    {
        g_s_SystemInfor.VpdDACSetK = strtod(Para2,NULL);
        g_s_SystemInfor.VpdDACSetB = strtod(Para3,NULL);
		reg_flag = MAIN_FLAG_VALUE;
			
		WriteE2promInfor(MAIN_I2C_CHN, PD_SET_ADDR*16,8, (uint8_t *)& g_s_SystemInfor.VpdDACSetK);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, PD_SET_ADDR*16+8,8, (uint8_t *)& g_s_SystemInfor.VpdDACSetB);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MAIN_FLAG_ADDR,1,&reg_flag);
    }
	else if(0 == strcmp((char*)Para0, "MPDCurrentGet1"))
    {
        g_s_SystemInfor.MPDGet_1st_K = strtod(Para2,NULL);
        g_s_SystemInfor.MPDGet_1st_B = strtod(Para3,NULL);
		reg_flag = MAIN_FLAG_VALUE;
			
		WriteE2promInfor(MAIN_I2C_CHN, MPD_1ST_ADDR*16,8, (uint8_t *)& g_s_SystemInfor.MPDGet_1st_K);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MPD_1ST_ADDR*16+8,8, (uint8_t *)&g_s_SystemInfor.MPDGet_1st_B);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MAIN_FLAG_ADDR,1,&reg_flag);
    }
	else if(0 == strcmp((char*)Para0, "MPDCurrentGet2"))
    {
        g_s_SystemInfor.MPDGet_2nd_K = strtod(Para2,NULL);
        g_s_SystemInfor.MPDGet_2nd_B = strtod(Para3,NULL);
		reg_flag = MAIN_FLAG_VALUE;
			
		WriteE2promInfor(MAIN_I2C_CHN, MPD_2ND_ADDR*16,8, (uint8_t *)& g_s_SystemInfor.MPDGet_2nd_K);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MPD_2ND_ADDR*16+8,8, (uint8_t *)&g_s_SystemInfor.MPDGet_2nd_B);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MAIN_FLAG_ADDR,1,&reg_flag);
    }
	else if(0 == strcmp((char*)Para0, "MPDCurrentGet3"))
    {
        g_s_SystemInfor.MPDGet_3rd_K = strtod(Para2,NULL);
        g_s_SystemInfor.MPDGet_3rd_B = strtod(Para3,NULL);
		reg_flag = MAIN_FLAG_VALUE;
			
		WriteE2promInfor(MAIN_I2C_CHN, MPD_3RD_ADDR*16,8, (uint8_t *)& g_s_SystemInfor.MPDGet_3rd_K);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MPD_3RD_ADDR*16+8,8, (uint8_t *)&g_s_SystemInfor.MPDGet_3rd_B);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MAIN_FLAG_ADDR,1,&reg_flag);
    }
	else if(0 == strcmp((char*)Para0, "SetMPDVolt"))
    {
        g_s_SystemInfor.MPDDACSetK = strtod(Para2,NULL);
        g_s_SystemInfor.MPDDACSetB = strtod(Para3,NULL);
		reg_flag = MAIN_FLAG_VALUE;
			
		WriteE2promInfor(MAIN_I2C_CHN, MPD_SET_ADDR*16,8, (uint8_t *)& g_s_SystemInfor.MPDDACSetK);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MPD_SET_ADDR*16+8,8, (uint8_t *)& g_s_SystemInfor.MPDDACSetB);
		vTaskDelay(pdMS_TO_TICKS(5));
		WriteE2promInfor(MAIN_I2C_CHN, MAIN_FLAG_ADDR,1,&reg_flag);
    }

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}


//***************************************************
//函数名  :GetCalDataProcess
//功能说明：查询相关校准参数
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void GetCalDataProcess(uint8_t DeviceNum,uint8_t *Parameter)
{
    char PortData = 0;
    char Para0[21] = {0};        //校准系数的类型
    char Para1[21] = {0};        //校准通道
		
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

    ParaSeparate(2,20,Parameter,Para0,Para1);

    PortData = atoi(Para1);
	
	if(PortData > DUT_MAX_NUM)
		PortData = DUT_MAX_NUM;
	
	if(PortData < 1)
		PortData = 1;
		
	if(0 == strcmp((char*)Para0, "LDCurrentSet"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_DutputInfor[PortData-1].SetValue_2nd_K,g_s_DutputInfor[PortData-1].SetValue_2nd_B);
	else if(0 == strcmp((char*)Para0, "LDCurrentGet"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_DutputInfor[PortData-1].GetValue_2nd_K,g_s_DutputInfor[PortData-1].GetValue_2nd_B);
	else if(0 == strcmp((char*)Para0, "LDCurrentMinSet"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_DutputInfor[PortData-1].SetValue_1st_K,g_s_DutputInfor[PortData-1].SetValue_1st_B);
	else if(0 == strcmp((char*)Para0, "LDCurrentMinGet"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_DutputInfor[PortData-1].GetValue_1st_K,g_s_DutputInfor[PortData-1].GetValue_1st_B);
	else if(0 == strcmp((char*)Para0, "PDCurrentGet1"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_SystemInfor.PDGet_1st_K,g_s_SystemInfor.PDGet_1st_B);
	else if(0 == strcmp((char*)Para0, "PDCurrentGet2"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_SystemInfor.PDGet_2nd_K,g_s_SystemInfor.PDGet_2nd_B);
	else if(0 == strcmp((char*)Para0, "PDCurrentGet3"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_SystemInfor.PDGet_3rd_K,g_s_SystemInfor.PDGet_3rd_B);
	else if(0 == strcmp((char*)Para0, "GetLDVoltage"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_SystemInfor.DutVoltGetK,g_s_SystemInfor.DutVoltGetB);
	else if(0 == strcmp((char*)Para0, "PosPWRVoltage"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Pos_K,g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Pos_B);
	else if(0 == strcmp((char*)Para0, "NegPWRVoltage"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Neg_K,g_s_DriverInfor[g_s_DutputInfor[PortData-1].DriverNum-1].SetPWR_Neg_B);
	else if(0 == strcmp((char*)Para0, "SetPDVolt"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_SystemInfor.VpdDACSetK,g_s_SystemInfor.VpdDACSetB);
	else if(0 == strcmp((char*)Para0, "MPDCurrentGet1"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_SystemInfor.MPDGet_1st_K,g_s_SystemInfor.MPDGet_1st_B);
	else if(0 == strcmp((char*)Para0, "MPDCurrentGet2"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_SystemInfor.MPDGet_2nd_K,g_s_SystemInfor.MPDGet_2nd_B);
	else if(0 == strcmp((char*)Para0, "MPDCurrentGet3"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_SystemInfor.MPDGet_3rd_K,g_s_SystemInfor.MPDGet_3rd_B);
	else if(0 == strcmp((char*)Para0, "SetMPDVolt"))
		sprintf(tx_buff+STRING_START,"%.5e %.5e\n",g_s_SystemInfor.MPDDACSetK,g_s_SystemInfor.MPDDACSetB);
	else 
		sprintf(tx_buff+STRING_START,"err\n");
		
	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//***************************************************
//函数名  :SetAPCModelPID
//功能说明：设置APC模式下 PID控制参数
//输入说明: Parameter：命令参数指针
//返回值：无
//说明：外部调用
//*************************************************
void SetAPCModelPID(uint8_t DeviceNum,uint8_t *Parameter)
{
    char PortData = 0;
    char Para0[21] = {0};        //校准系数的类型
    char Para1[21] = {0};        //通道
    char Para2[21] = {0};        //系数K
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

    ParaSeparate(3,20,Parameter,Para0,Para1,Para2);
	
    PortData = atoi(Para1);
	
	if(PortData > SAMP_MAX_NUM)
		PortData = SAMP_MAX_NUM;
	
	if(PortData < 1)
		PortData = 1;
		
	if(0 == strcmp((char*)Para0, "KP"))
    {
        g_s_SampInInfor[PortData-1].ApcPID_Kp = strtod(Para2,NULL);
    }
	else if(0 == strcmp((char*)Para0, "KI"))
    {
        g_s_SampInInfor[PortData-1].ApcPID_Ki = strtod(Para2,NULL);
    }
	else if(0 == strcmp((char*)Para0, "KD"))
    {
        g_s_SampInInfor[PortData-1].ApcPID_Kd = strtod(Para2,NULL);
    }
		
	PortAPCPidInitial(PortData-1);
		
	sprintf(tx_buff+STRING_START,"ok\n");

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}


//**************************************************
//函数：ConfigChannelProcess
//功能：切换通道
//说明：
//**************************************************
void ConfigChannelProcess(uint8_t DeviceNum,uint8_t *Parameter)
{	
	int16_t data = 0;
	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	data = atoi((char*)Parameter);
		
	if((data>0)&&(data<=SAMP_MAX_NUM))
	{
		g_s_SystemInfor.CurrSamp = data;
		
		SwitchMPDTH(g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].SampNum);
		
		DriverDelayBlockMs(10);
		
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else 
	{
		sprintf(tx_buff+STRING_START, "err\n");
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：TaskSampSuspend
//功能：挂起采样线程
//说明：
//**************************************************
void TaskSampSuspend(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	if( xSemaphoreGive(SampMonitSemphr) != pdTRUE )
	{
		sprintf(tx_buff+STRING_START, "err\n");
	}
	else
	{
		while(eTaskGetState(SampMonitHandle) != eSuspended)
			vTaskDelay(pdMS_TO_TICKS(1));
		
		sprintf(tx_buff+STRING_START, "ok\n");
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}
//**************************************************
//函数：TaskSampResume
//功能：恢复采样线程
//说明：
//**************************************************
void TaskSampResume(uint8_t DeviceNum,uint8_t *Parameter)
{	
	uint16_t send_lengt = 0;
	char* p_tx_buff = NULL;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	if(eTaskGetState(SampMonitHandle) == eSuspended)
	{
		vTaskResume(SampMonitHandle);
		sprintf(tx_buff+STRING_START, "ok\n");
	}
	else
	{
		sprintf(tx_buff+STRING_START, "err\n");
	}

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}


//**************************************************
//函数：OtaStart
//功能：开始进行OTA在线升级并传输文件参数
//说明：
//**************************************************
void OtaStart(uint8_t DeviceNum,uint8_t *Parameter)
{	
	char* p_tx_buff = NULL;
	uint16_t send_lengt = 0;

	uint32_t tolat = 0;
	uint16_t block = 0;
	uint16_t block_size = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	tolat = ((uint32_t)Parameter[0])<<24|((uint32_t)Parameter[1])<<16|((uint32_t)Parameter[2])<<8|(uint32_t)Parameter[3];
	block = ((uint16_t)Parameter[4])<<8|(uint16_t)Parameter[5];
	block_size = ((uint16_t)Parameter[6])<<8|(uint16_t)Parameter[7];

	if(start_ota(tolat,block,block_size) == 0)
		sprintf(tx_buff+STRING_START, "ota:start ok\n");
	else 
		sprintf(tx_buff+STRING_START, "ota:err\n");

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：OtaCommit
//功能：升级bin文件接收并写入备份存储空间
//说明：
//**************************************************
void OtaCommit(uint8_t DeviceNum,uint8_t *Parameter)
{	
	char* p_tx_buff = NULL;
	uint16_t send_lengt = 0;
	uint16_t block_num = 0;
	uint16_t data_lengt = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));
	
	block_num = ((uint16_t)Parameter[0])<<8|((uint16_t)Parameter[1]);
	data_lengt = ((uint16_t)Parameter[2])<<8|((uint16_t)Parameter[3]);
	
	if(commit_ota(block_num,(uint8_t *)(Parameter+4),data_lengt) == 0)
		sprintf(tx_buff+STRING_START, "ota:commit ok %d\n",block_num);
	else 
		sprintf(tx_buff+STRING_START, "ota:err\n");

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else	if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

//**************************************************
//函数：OtaVerfyMark
//功能：OTA升级功能完成升级后进行校验标记
//说明：
//**************************************************
void OtaVerfyMark(uint8_t DeviceNum,uint8_t *Parameter)
{	
	char* p_tx_buff = NULL;
	uint16_t send_lengt = 0;
	
	memset(tx_buff, 0, sizeof(tx_buff));

	if(verfy_ota_and_mark() == 0)
		sprintf(tx_buff+STRING_START, "ota:verfymark ok\n");
	else
		sprintf(tx_buff+STRING_START, "ota:err\n");

	p_tx_buff = tx_buff;
	
	send_lengt = (uint16_t)strlen(tx_buff+STRING_START);tx_buff[0] = send_lengt>>8;tx_buff[1] = send_lengt;
	
	if(ETH_MQTT_INTERFACE==DeviceNum)
		xQueueSendToBack(PublishQueue,(void*)&p_tx_buff,pdMS_TO_TICKS(0));
	else  if(RS485_DEVICE_INTERFACE==DeviceNum)
		RS485SendFrame(send_lengt,tx_buff+STRING_START);
}

														
//**************************************************
//函数名  :  MessageAnalyseEntry
//功能说明： 通讯传递初始化
//					串口 CAN 配置 以及解析平台配置
//输入说明： void
//***************************************************
void MessageAnalyseEntry(void *p_arg)
{
	uint8_t *ReceivereCommand = NULL;
	QueueHandle_t xQueue;

	xQueue = (QueueHandle_t)p_arg;
	
	for(;;)
	{
		if(pdFALSE != xQueueReceive(xQueue,(void*)&ReceivereCommand,portMAX_DELAY))
		{
			CommandAnalyse(ReceivereCommand);
			BuffClear(((CommInterfaceInfo*)ReceivereCommand)->DataBufferBackUp,GetStrLength((int8_t*)((CommInterfaceInfo*)ReceivereCommand)->DataBufferBackUp),'\0');
			xSemaphoreGive(LinkLedSemphr);
		}
	}
}


//***************************************************
//函数名  :  MessagePassInit
//功能说明： 通讯传递初始化
//					串口 CAN 配置 以及解析平台配置
//输入说明： void
//***************************************************
void MessagePassInit(void)
{
	BaseType_t CreatStatus;

	//初始化命令解析器
	CommandSysInit(g_p_FreeCommandMem,MAX_COMMAND_NUM*MAX_COMMAND_LEN);
	
	RegisterCommand((uint8_t*)"*RST",SetReset);			
	RegisterCommand((uint8_t*)"*IDN?",CallIDN);	

	//通讯指令
	RegisterCommand((uint8_t*)"CFG:BurnInMode",ConfigLDBurnInMode);								//设置模式
	RegisterCommand((uint8_t*)"CFG:BurnInMode?",CheckLDBurnInMode);								//读取模式
	RegisterCommand((uint8_t*)"CFG:APCStatus",ConfigAPCModePara);								//设置APC模式背光值、允许偏差值、驱动电流限制值
	RegisterCommand((uint8_t*)"CFG:APCStatus?",CheckAPCModePara);								//读取APC模式下参数值
	RegisterCommand((uint8_t*)"CFG:PortSwitch",ConfigPortSwitch);								//设置输出switch开关
	RegisterCommand((uint8_t*)"Read:DriverStatus?",ReadDrvierType);								//获取驱动板类型状态
	RegisterCommand((uint8_t*)"CFG:LDCurrent",ConfigLDCurrentProcess);
	RegisterCommand((uint8_t*)"CFG:LDCurrent?",CheckLDCurrentProcess);
	RegisterCommand((uint8_t*)"Read:SetUpStatus?",ReadSetupAllStatus);							//读取加电状态
	RegisterCommand((uint8_t*)"CFG:MPDVoltage",ConfigMPDVoltageProcess);      					//调用该函数前要先设置驱动电流
	RegisterCommand((uint8_t*)"CFG:MPDVoltage?",CheckMPDVoltageProcess);      					//调用该函数前要先设置驱动电流	
	RegisterCommand((uint8_t*)"CFG:PDVoltage",ConfigPDVoltageProcess);      					//调用该函数前要先设置驱动电流
	RegisterCommand((uint8_t*)"CFG:PDVoltage?",CheckPDVoltageProcess);      					//调用该函数前要先设置驱动电流	
	RegisterCommand((uint8_t*)"CFG:LDVCCSwitch",SetLDVCCSwitchProcess);  						//LDVCC电压开关
	RegisterCommand((uint8_t*)"Read:PowerStatus?",ReadPowerStatusProcess);  					//查询LDIth测试是否结束
	RegisterCommand((uint8_t*)"CFG:AllPowerDown",SetLDAllPowerDownProcess);  					//LDVCC电压开关
	RegisterCommand((uint8_t*)"CFG:CurrentSetNum",ConfigTimeUp);								//设置加电次数
	RegisterCommand((uint8_t*)"CFG:CurrentSetNum?",CheckTimeUp);								//查询加电时间设置
	RegisterCommand((uint8_t*)"CFG:LDGear",ConfigLDCurrentGear);								//设置LD器件类型
	RegisterCommand((uint8_t*)"CFG:LDGear?",CheckLDCurrentGear);								//查询LD
	RegisterCommand((uint8_t*)"CFG:MPDLevel",ConfigMPDCurrentLevel);							//设置LD器件类型
	RegisterCommand((uint8_t*)"CFG:MPDLevel?",CheckMPDCurrentLevel);							//查询LD器件类型	
	RegisterCommand((uint8_t*)"CFG:PDLevel",ConfigPDCurrentLevel);								//设置LD器件类型
	RegisterCommand((uint8_t*)"CFG:PDLevel?",CheckPDCurrentLevel);								//查询LD器件类型	
	
	RegisterCommand((uint8_t*)"CFG:PWRLimit",ConfigPWRLimit);									//配置电压源电流限制
	
	RegisterCommand((uint8_t*)"CFG:CurrChannel",ConfigChannelProcess);				//查询LD器件类型	
	RegisterCommand((uint8_t*)"TASK:SampSuspend",TaskSampSuspend);					//配置当前通道
	RegisterCommand((uint8_t*)"TASK:SampResume",TaskSampResume);					//配置当前通道	
	
	RegisterCommand((uint8_t*)"CFG:LDScanCurrent",ConfigLDScanCurrentProcess);		//配置扫描起点、步进、终点
	RegisterCommand((uint8_t*)"CFG:LDScanCurrent?",CheckLDScanCurrentProcess);  	//查询扫描起点、步进、终点
	RegisterCommand((uint8_t*)"CFG:StartLDScan",StartLDScanTestProcess); 			//其中LDIth测试
	RegisterCommand((uint8_t*)"Read:LDScanStatus?",ReadLDScanStatusProcess);  		//查询LDIth测试是否结束
	RegisterCommand((uint8_t*)"Read:LDIthData?",ReadLDIthProcess); 					//读取Ith测试结果
	
	RegisterCommand((uint8_t*)"Read:LDCurrent?",ReadLDCurrentProcess);
	RegisterCommand((uint8_t*)"Read:LDVoltage?",ReadLDVoltageProcess);
	RegisterCommand((uint8_t*)"Read:PDCurrent?",ReadPDCurrentProcess);
	RegisterCommand((uint8_t*)"Read:DutBit?",ReadDutBitResult);  					//读取DUT port 电压、电流
	RegisterCommand((uint8_t*)"Read:DutAll?",ReadDutAllResult); 				 	//读取DUT全部电压、电流
	RegisterCommand((uint8_t*)"Read:MonitBit?",ReadMonitBitResult);  				//读取Monit port 电压、电流
	RegisterCommand((uint8_t*)"Read:MonitAll?",ReadMonitAllResult); 				//读取Monit全部电压、电流
	
	RegisterCommand((uint8_t*)"INF:WriteAT24C02",WriteBurnInBoard);
	RegisterCommand((uint8_t*)"INF:ReadAT24C02",ReadBurnInBoard); 
	RegisterCommand((uint8_t*)"INF:TempMux?",ReadTempInf); 		
	RegisterCommand((uint8_t*)"INF:BoardLine",ReadBoardLineInf); 
	RegisterCommand((uint8_t*)"INF:IICBoard",ReadIICBoardInf); 
	RegisterCommand((uint8_t*)"SET:AutoOutage",SetAutoOutage); 
	
	RegisterCommand((uint8_t*)"SET:OnChipFlag",SetOnChipFlag); 
	RegisterCommand((uint8_t*)"LED:Control",ControlLedStatus); 						//控制LED输出状态
	
	RegisterCommand((uint8_t*)"TRAN:BandSpeed",TransBandSpeed);
	RegisterCommand((uint8_t*)"TRAN:Ascii",TranspondAscii);
	RegisterCommand((uint8_t*)"TRAN:Hex",TranspondHex);
	
	//内部校准参数
	RegisterCommand((uint8_t*)"!Set:CalData",SetCalDataProcess);
	RegisterCommand((uint8_t*)"!Get:CalData",GetCalDataProcess); 
	RegisterCommand((uint8_t*)"!Set:PIDPara",SetAPCModelPID); 	

	RegisterCommand((uint8_t*)"OTA:START",OtaStart);	
	RegisterCommand((uint8_t*)"OTA:COMMIT",OtaCommit);
	RegisterCommand((uint8_t*)"OTA:MARK",OtaVerfyMark);	

	MessageQueue = xQueueCreate(MESSAGE_QUEUE_LENGTH,MESSAGE_QUEUE_ITEM_SIZE);
	if(MessageQueue == NULL)
		return ;
						 
	CreatStatus = xTaskCreate(MessageAnalyseEntry,"MessageAnalyse",MSG_ANALYSE_SIZE,(void*)MessageQueue,MSG_ANALYSE_PRIO,NULL);
	
	if(CreatStatus != pdPASS)
		return ;
}



