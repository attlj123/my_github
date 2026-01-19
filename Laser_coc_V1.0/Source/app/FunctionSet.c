#include <math.h>

#include "Driver_I2C.h"
#include "SGM5348.h"
#include "PID.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "FunctionSet.h"
#include "MessagePass.h"
#include "ControlParam.h"
#include "app_cfg.h"

TaskHandle_t SampDutHandle;					//采样DUT线程
TaskHandle_t SampMonitHandle;				//采样监控现场
TaskHandle_t OtherEntryHandle;

SemaphoreHandle_t SetupALLSemphr = NULL;    	//设置信号量
SemaphoreHandle_t ScanIthSemphr = NULL;    		//扫描信号量

SemaphoreHandle_t SampDutSemphr = NULL;			//采样DUT参数
SemaphoreHandle_t SampMonitSemphr = NULL;		//采样监控参数

SemaphoreHandle_t OtherItemSemphr = NULL;

DUT_TypeStruct 		g_s_DutputInfor[DUT_MAX_NUM];
Driver_TypeStruct   g_s_DriverInfor[DRIVER_NUM];
SampIn_TypeStruct   g_s_SampInInfor[SAMP_MAX_NUM];
System_TypeStruct   g_s_SystemInfor;

uint16_t IthSetVaule[ITH_MAX_COUNT] = {0};
uint16_t IthVoltage[ITH_MAX_COUNT] = {0};
uint16_t IthCurrent[ITH_MAX_COUNT] = {0};
uint16_t IthPD[ITH_MAX_COUNT] = {0};
uint16_t IthMPD[ITH_MAX_COUNT] = {0};

//PID参数
PID_Ratio APC_PidRatio[SAMP_MAX_NUM];
//***************************************************
 //函数名  ：DriverDelayBlockUs
 //功能说明：us单位延时，尽管CPU时钟改变了，只需更改宏定义CPU_CLK 
 //输入、输出说明：TimeSet：设置时间  
//***************************************************
void DriverDelayBlockUs(uint16_t SetCnt)
{
	uint16_t i = 0;
	uint16_t j = 0;
 
	for(i=0; i<SetCnt; i++)
		for(j=0; j<DELAY_US_CNT; j++);
}

//***************************************************
 //函数名  DriverDelayBlockMs
 //功能说明：us单位延时，尽管CPU时钟改变了，只需更改宏定义CPU_CLK 
 //输入、输出说明：TimeSet：设置时间  
//***************************************************
void DriverDelayBlockMs(uint16_t SetCnt)
{
	vTaskDelay(pdMS_TO_TICKS(SetCnt));
}

//***************************************************
//函数名  :  工作电压设置
//功能说明： 功能设置任务
//输入说明： ldstatus 激光器状态
//返回值  ：
//作者/时间/版本号:  liuwei/201507014
//***************************************************
void SetPortDutvalue(DUT_TypeStruct* port,double dac)
{
	uint16_t dac_bit = 0;
	dac_bit = dac/SGM_DAC_VREF*SGM_DAC_BIT;
	
	if(dac_bit>SGM_DAC_BIT)
		dac_bit = SGM_DAC_BIT;

	ConfigSGM5348Value(port->DacGroupNum,port->DacSonNum,dac_bit);
	
	DriverDelayBlockUs(15);
}

//***************************************************
//函数名  :  SetFuncValue
//功能说明： 配置监控功能电压值
//输入说明：
//pwr_csp:   type：12   chn：0~5 driver
//pwr_csn:   type：13   chn：0~5 driver
//cvlimit:   type：14   chn：0:v_pos,1:c_pos,2:v_neg,3:c_neg（both board used）,4:set_cl(voltage board used)
//Monit:     type：15   chn：0:PD,1:MPD,2:TH
//返回值  ： 
//作者/时间/版本号:  liuwei/201507014
//***************************************************
void SetFuncValue(uint8_t type,uint8_t chn,double dac)
{
	uint16_t dac_bit = 0;
	dac_bit = dac/SGM_DAC_VREF*SGM_DAC_BIT;
	
	if(dac_bit>SGM_DAC_BIT)
		dac_bit = SGM_DAC_BIT;

	ConfigSGM5348Value(type,chn,dac_bit);
	
	DriverDelayBlockUs(15);
}

//***************************************************
//函数名  :  LDCurrentSetdown
//功能说明： LD驱动电流设置
//输入说明： LD需要设置驱动电流
//返回值  ：
//***************************************************
void LDCurrentSetdown_OneStep(Driver_TypeStruct *driver,double curr)
{
	uint8_t Port = 0;
	double VoltDAC = 0;
	
	driver->SetValue = curr;
	
	driver->NowSetValue = driver->SetValue;
	
	if(driver->NowSetValue == 0)
	{
		for(Port=0;Port<DUT_MAX_NUM;Port++)
		{
			if(driver->DriverNum == g_s_DutputInfor[Port].DriverNum)
			{
				if(driver->DriverType == DRIVER_TYPE_CURR)
					SetPortDutvalue(&g_s_DutputInfor[Port],0);
				else if(driver->DriverType == DRIVER_TYPE_VOLT)
					SetPortDutvalue(&g_s_DutputInfor[Port],0);
				else if(driver->DriverType == DRIVER_TYPE_BOTH)
					SetPortDutvalue(&g_s_DutputInfor[Port],0);
			}

		}
	}
	else
	{
		for(Port=0;Port<DUT_MAX_NUM;Port++)
		{
			if(driver->DriverNum == g_s_DutputInfor[Port].DriverNum)
			{
				if(driver->LevelGear == 1)
					VoltDAC = driver->NowSetValue*g_s_DutputInfor[Port].SetValue_2nd_K+g_s_DutputInfor[Port].SetValue_2nd_B;
				else
					VoltDAC = driver->NowSetValue*g_s_DutputInfor[Port].SetValue_1st_K+g_s_DutputInfor[Port].SetValue_1st_B;
			
				SetPortDutvalue(&g_s_DutputInfor[Port],VoltDAC);
			}
		}
	}
}

//***************************************************
//函数名  :  LDCurrentSetdown
//功能说明： LD驱动电流设置
//输入说明： LD需要设置驱动电流
//返回值  ：
//***************************************************
void LDCurrentSetdown(Driver_TypeStruct *driver,double curr)
{
	uint8_t i = 0;
	uint8_t Port = 0;
	double VoltDAC = 0;
	double SetData = 0;

	if(g_s_SystemInfor.SetStepNum == 0)
		return ;
	
	driver->SetValue = curr;
	
	SetData = (driver->SetValue - driver->NowSetValue)/g_s_SystemInfor.SetStepNum;
	
	for(i=1;i<g_s_SystemInfor.SetStepNum;i++)
	{
		driver->NowSetValue = driver->NowSetValue + SetData;

		for(Port=0;Port<DUT_MAX_NUM;Port++)
		{
			if(g_s_DutputInfor[Port].DriverNum == driver->DriverNum)
			{
				if(driver->LevelGear == 1)
					VoltDAC = driver->NowSetValue * g_s_DutputInfor[Port].SetValue_2nd_K + g_s_DutputInfor[Port].SetValue_2nd_B;
				else
					VoltDAC = driver->NowSetValue * g_s_DutputInfor[Port].SetValue_1st_K + g_s_DutputInfor[Port].SetValue_1st_B;
				
				SetPortDutvalue(&g_s_DutputInfor[Port],VoltDAC);
			}
		}
		
		vTaskDelay(pdMS_TO_TICKS(g_s_SystemInfor.SetDelayNum));
	}
	
	driver->NowSetValue = driver->SetValue;
	
	if(driver->NowSetValue == 0)
	{
		for(Port=0;Port<DUT_MAX_NUM;Port++)
		{
			if(g_s_DutputInfor[Port].DriverNum == driver->DriverNum)
			{
				if(driver->DriverType == DRIVER_TYPE_CURR)			//根据板类型进行 设置 0 参数
					SetPortDutvalue(&g_s_DutputInfor[Port],0);
				else if(driver->DriverType == DRIVER_TYPE_VOLT)
					SetPortDutvalue(&g_s_DutputInfor[Port],0);
				else if(driver->DriverType == DRIVER_TYPE_BOTH)
					SetPortDutvalue(&g_s_DutputInfor[Port],0);
			}
		}
	}
	else
	{
		for(Port=0;Port<DUT_MAX_NUM;Port++)
		{
			if(g_s_DutputInfor[Port].DriverNum == driver->DriverNum)
			{
				if(driver->LevelGear == 1)
					VoltDAC = driver->NowSetValue * g_s_DutputInfor[Port].SetValue_2nd_K + g_s_DutputInfor[Port].SetValue_2nd_B;
				else
					VoltDAC = driver->NowSetValue * g_s_DutputInfor[Port].SetValue_1st_K + g_s_DutputInfor[Port].SetValue_1st_B;

				SetPortDutvalue(&g_s_DutputInfor[Port],VoltDAC);
			}
		}
	}
}

//***************************************************
//函数名  :  LDCurrentSetBit
//功能说明： 单路LD驱动电流设置,单步设置，不需缓加电
//输入说明： LD需要设置驱动电流
//返回值  ：
//***************************************************
void LDCurrentSetBit_OneStep(Driver_TypeStruct *driver,DUT_TypeStruct* port)
{
    double VoltDAC = 0;

    driver->NowSetValue = driver->SetValue;
    if(driver->NowSetValue == 0)
    {
		if(driver->DriverType == DRIVER_TYPE_CURR)			//根据板类型进行 设置 0 参数
			SetPortDutvalue(port,0);
		else if(driver->DriverType == DRIVER_TYPE_VOLT)
			SetPortDutvalue(port,0);
		else if(driver->DriverType == DRIVER_TYPE_BOTH)
			SetPortDutvalue(port,0);
    }
    else
    {
        if(driver->LevelGear == 1)
            VoltDAC = driver->NowSetValue * port->SetValue_2nd_K + port->SetValue_2nd_B;
        else
            VoltDAC = driver->NowSetValue * port->SetValue_1st_K + port->SetValue_1st_B;
				
		SetPortDutvalue(port,VoltDAC);
    }

	vTaskDelay(pdMS_TO_TICKS(1));
}

//***************************************************
//函数名  :  ScanCurrentOutPut
//功能说明： 扫描LD恒流输出
//输入说明： 
//***************************************************
void ScanCurrentOutPut(DUT_TypeStruct* port,double vol)
{
	double VoltDAC = 0;

	if(vol == 0)
	{
		if(g_s_DriverInfor[port->DriverNum].DriverType == DRIVER_TYPE_CURR)
			SetPortDutvalue(port,0);
		else if(g_s_DriverInfor[port->DriverNum].DriverType == DRIVER_TYPE_VOLT)
			SetPortDutvalue(port,0);
		else if(g_s_DriverInfor[port->DriverNum].DriverType == DRIVER_TYPE_BOTH)
			SetPortDutvalue(port,0);
	}
	else
	{
		if(g_s_DriverInfor[port->DriverNum].LevelGear == 1)
			VoltDAC = vol * port->SetValue_2nd_K + port->SetValue_2nd_B;
		else
			VoltDAC = vol * port->SetValue_1st_K + port->SetValue_1st_B;
		
		SetPortDutvalue(port,VoltDAC);
	}
}

//***************************************************
//函数名  :  ScanLDIthEntry
//功能说明： 扫描激光器ITH曲线
//输入说明： void
//返回值  ：
//***************************************************
void ScanLDIthEntry(void *p_arg)
{
	SemaphoreHandle_t xSemaphore;
	uint16_t i = 0;
	int16_t vol_int16 = 0;
	int16_t cur_int16 = 0;
	int16_t pd_int16 = 0;
	
	double vol_double = 0;
	double cur_double = 0;
	double pd_double = 0;
	
	xSemaphore = (SemaphoreHandle_t)p_arg;
	
	for(;;)
	{		
		if( xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdPASS )
		{
			if( xSemaphoreGive(SampDutSemphr) == pdTRUE )
			{
				while(eTaskGetState(SampDutHandle) != eSuspended)
					vTaskDelay(pdMS_TO_TICKS(1));
			}	
				
			if( xSemaphoreGive(SampMonitSemphr) == pdTRUE )
			{
				while(eTaskGetState(SampMonitHandle) != eSuspended)
					vTaskDelay(pdMS_TO_TICKS(1));
			}
			
			SwitchDUTSamp(g_s_SystemInfor.ScanPort);
			SwitchMPDTH(g_s_SystemInfor.ScanPD);
			
			ScanCurrentOutPut(&g_s_DutputInfor[g_s_SystemInfor.ScanPort-1],g_s_SystemInfor.ScanStart);
			
			DriverDelayBlockMs(40);
			
			for(i=0;i<g_s_SystemInfor.IthScanNum;i++)
			{
				ScanCurrentOutPut(&g_s_DutputInfor[g_s_SystemInfor.ScanPort-1],g_s_SystemInfor.ScanStart+i*g_s_SystemInfor.ScanStep);
				IthSetVaule[i] = (g_s_SystemInfor.ScanStart+i*g_s_SystemInfor.ScanStep)*10;
				
				DriverDelayBlockMs(2);

				if(I2C_NO_ERR == ReadScanPIV(&vol_int16,&cur_int16,&pd_int16))
				{
					vol_double = (double)vol_int16*g_s_SystemInfor.DutVoltGetK+g_s_SystemInfor.DutVoltGetB;
					
					if(g_s_DriverInfor[g_s_DutputInfor[g_s_SystemInfor.ScanPort-1].DriverNum-1].LevelGear == 0)
						cur_double = (double)cur_int16*g_s_DutputInfor[g_s_SystemInfor.ScanPort-1].SetValue_1st_K+g_s_DutputInfor[g_s_SystemInfor.ScanPort-1].SetValue_1st_B;
					else if(g_s_DriverInfor[g_s_DutputInfor[g_s_SystemInfor.ScanPort-1].DriverNum-1].LevelGear == 1)
						cur_double = (double)cur_int16*g_s_DutputInfor[g_s_SystemInfor.ScanPort-1].SetValue_2nd_K+g_s_DutputInfor[g_s_SystemInfor.ScanPort-1].SetValue_2nd_B;
					
					if(g_s_SystemInfor.PDLevel == 0)
						pd_double = (double)pd_int16*g_s_SystemInfor.PDGet_1st_K+g_s_SystemInfor.PDGet_1st_B;
					else if(g_s_SystemInfor.PDLevel == 1)
						pd_double = (double)pd_int16*g_s_SystemInfor.PDGet_2nd_K+g_s_SystemInfor.PDGet_2nd_B;
					else if(g_s_SystemInfor.PDLevel == 2)
						pd_double = (double)pd_int16*g_s_SystemInfor.PDGet_3rd_K+g_s_SystemInfor.PDGet_3rd_B;
					
					pd_double = fabs(pd_double);

					IthVoltage[i] = vol_double*1000;			//单位值 1mV
					IthCurrent[i] = cur_double*10;				//单位值 100uA
					IthPD[i] = pd_double*10;					//单位值 0.1uA
				}

//				IthVoltage[i] = vol_double*1000+i;			//单位值 1mV
//				IthCurrent[i] = cur_double*10+i*2;				//单位值 100uA
//				IthPD[i] = pd_double*10+i*3;					//单位值 0.1uA
			}

			if(g_s_SystemInfor.BurnInMode == BURNIN_ACC_MODE)
				LDCurrentSetBit_OneStep(&g_s_DriverInfor[g_s_DutputInfor[g_s_SystemInfor.ScanPort-1].DriverNum-1],&g_s_DutputInfor[g_s_SystemInfor.ScanPort-1]);
			else if(g_s_SystemInfor.BurnInMode == BURNIN_APC_MODE)
				ScanCurrentOutPut(&g_s_DutputInfor[g_s_SystemInfor.ScanPort-1],g_s_DutputInfor[g_s_SystemInfor.ScanPort-1].ApcLDCurrentSet);

			
			if(eTaskGetState(SampDutHandle) == eSuspended)
				vTaskResume(SampDutHandle);
			
			if(eTaskGetState(SampMonitHandle) == eSuspended)
				vTaskResume(SampMonitHandle);
			
			g_s_SystemInfor.IthStatus = 0;
		}
	}
}


//***************************************************
//函数名  :  设置全部加电线程
//功能说明： 
//输入说明： void
//返回值  ：
//***************************************************
void SetupAllEntry(void *p_arg)
{
	SemaphoreHandle_t xSemaphore;
	
	xSemaphore = (SemaphoreHandle_t)p_arg;
	
	for(;;)
	{		
		if( xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdPASS )
		{
			
			LDCurrentSetdown(&g_s_DriverInfor[g_s_SystemInfor.CurrDriver],g_s_DriverInfor[g_s_SystemInfor.CurrDriver].SetValue);
			
			g_s_SystemInfor.AllsetStatus = 0;
		}
	}
}


//***************************************************
//函数名  :  SampPortEntry
//功能说明： 实时采样电流电压功能
//输入说明： void
//返回值  ：
//***************************************************
double APCModePIDSampling(void)
{
	double sampPD = 0;
	
	sampPD = g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue;
	
	return sampPD;
}


void APCModePIDOpertion(double opt_curr)
{
	if(opt_curr>=g_s_SystemInfor.APCLimitCurrent)
		opt_curr = g_s_SystemInfor.APCLimitCurrent;
	
	g_s_DutputInfor[g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].APCDutPort-1].ApcLDCurrentSet = opt_curr;
	
	ScanCurrentOutPut(&g_s_DutputInfor[g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].APCDutPort-1],g_s_DutputInfor[g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].APCDutPort-1].ApcLDCurrentSet);
}

//***************************************************
//函数名  :  SampDutEntry
//功能说明： 实时采样DUT电流电压
//输入说明： void
//返回值  ：
//***************************************************
void SampDutEntry(void *p_arg)
{
	SemaphoreHandle_t xSemaphore = NULL;
	int16_t  vol_int16 = 0;
	int16_t  cur_int16 = 0;

	xSemaphore = (SemaphoreHandle_t)p_arg;
 
	for(;;)
	{		
		if(pdFALSE == xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(20)))  //20
		{
			SwitchDUTSamp(g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].PortNum);

			DriverDelayBlockMs(5);
			
//			if(I2C_NO_ERR == ReadSampDUT(64,1,NULL,&cur_int16,&vol_int16))
//			{
//				if(g_s_DriverInfor[g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].DriverNum-1].LevelGear == 0)
//					g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Curr_GetValue  = (double)cur_int16*g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_1st_K+g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_1st_B;
//				else if(g_s_DriverInfor[g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].DriverNum-1].LevelGear == 1)
//					g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Curr_GetValue  = (double)cur_int16*g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_2nd_K+g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].GetValue_2nd_B;

//				g_s_DutputInfor[g_s_SystemInfor.CurrPort-1].Volt_GetValue = (double)vol_int16*g_s_SystemInfor.DutVoltGetK+g_s_SystemInfor.DutVoltGetB;				
//			}

			g_s_SystemInfor.CurrPort++;
			
			if(g_s_SystemInfor.CurrPort>DUT_MAX_NUM)						
				g_s_SystemInfor.CurrPort = 1;
		}
		else
		{
			vTaskSuspend(SampDutHandle);
		}
	}
}

//***************************************************
//函数名  :  SampMonitEntry
//功能说明： 实时采样PD,MPD,TH
//输入说明： void
//返回值  ：
//***************************************************
void SampMonitEntry(void *p_arg)
{
	SemaphoreHandle_t xSemaphore = NULL;
	int16_t  pd_int16 = 0;
	int16_t  mpd_int16 = 0;

	xSemaphore = (SemaphoreHandle_t)p_arg;
 
	for(;;)
	{		
		if(pdFALSE == xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(20)))  //20
		{

			SwitchMPDTH(g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].SampNum);

			DriverDelayBlockMs(5);
			
//			if(I2C_NO_ERR == ReadPDandMPD(64,1,NULL,&pd_int16,&mpd_int16))
//			{
//				if(g_s_SystemInfor.PDLevel == 0)
//					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue = (double)pd_int16*g_s_SystemInfor.PDGet_1st_K+g_s_SystemInfor.PDGet_1st_B;
//				else if(g_s_SystemInfor.PDLevel == 1)
//					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue = (double)pd_int16*g_s_SystemInfor.PDGet_2nd_K+g_s_SystemInfor.PDGet_2nd_B;
//				else if(g_s_SystemInfor.PDLevel == 2)
//					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue = (double)pd_int16*g_s_SystemInfor.PDGet_3rd_K+g_s_SystemInfor.PDGet_3rd_B;

//				if(g_s_SystemInfor.MPDLevel == 0)
//					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].MPD_GetValue = (double)pd_int16*g_s_SystemInfor.MPDGet_1st_K+g_s_SystemInfor.MPDGet_1st_B;
//				else if(g_s_SystemInfor.MPDLevel == 1)
//					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].MPD_GetValue = (double)pd_int16*g_s_SystemInfor.MPDGet_2nd_K+g_s_SystemInfor.MPDGet_2nd_B;
//				else if(g_s_SystemInfor.MPDLevel == 2)
//					g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].MPD_GetValue = (double)pd_int16*g_s_SystemInfor.MPDGet_3rd_K+g_s_SystemInfor.MPDGet_3rd_B;
//			}
			
			//APC模式下进行 PID调节
			if((g_s_SystemInfor.BurnInMode == BURNIN_APC_MODE)&&(g_s_SystemInfor.PowerStatus == SYSTEMS_ON)&& 																												\
	/***			 (g_s_DutInfor[g_s_SystemInfor.CurrPort-1].ApcLDCurrentSet<=g_s_SystemInfor.APCLimitCurrent)&& 																					\   ***/
				 (g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].APCModePDValue>0)&& 																					\
			(fabs(g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].PD_GetValue-g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].APCModePDValue)>g_s_SystemInfor.ImPdOffSet))  //PID调节限制
			{
				PID_Function(APCModePIDOpertion,APCModePIDSampling,&APC_PidRatio[g_s_SystemInfor.CurrSamp-1],g_s_SampInInfor[g_s_SystemInfor.CurrSamp-1].APCModePDValue);
			}
			
			g_s_SystemInfor.CurrSamp++;
			if(g_s_SystemInfor.CurrSamp>SAMP_MAX_NUM)							
				g_s_SystemInfor.CurrSamp = 1;		
		}
		else
		{
			vTaskSuspend(SampMonitHandle);
		}
	}
}



//***************************************************
//函数名  :  OtherItemEntry
//功能说明： 功能任务
//输入说明： void
//返回值  ：
//***************************************************
void OtherItemEntry(void *p_arg)
{
	SemaphoreHandle_t xSemaphore = NULL;

	xSemaphore = (SemaphoreHandle_t)p_arg;
	
	for(;;)
	{		
		if(pdFALSE == xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(500)))
		{		
//				//检测驱动板是否连接正常
//				if(DetectDriverStatus() == DETECT_FAIL)
//					g_s_SystemInfor.SystemErrType = SYSTEMS_DRIVER_ERROR;
//				else 
//					g_s_SystemInfor.SystemErrType = SYSTEMS_NONE_ERROR;
			
			//插拔自动下电
			if(g_s_SystemInfor.SystemAutoOutage == SYSTEMS_ON)
			{
				if((g_s_SystemInfor.SystemErrType == SYSTEMS_DRIVER_ERROR)&&(g_s_SystemInfor.PowerStatus == SYSTEMS_ON))
				{
//							RESET_PWR_A
//							RESET_PWR_B
//							RESET_PWR_C
//							LDCurrentSetdown_OneStep(&g_s_DriverInfor[0],0);
//							LDCurrentSetdown_OneStep(&g_s_DriverInfor[1],0);
//							LDCurrentSetdown_OneStep(&g_s_DriverInfor[2],0);
//							g_s_DriverInfor[0].LdCurrentSet = 0;
//							g_s_DriverInfor[1].LdCurrentSet = 0;
//							g_s_DriverInfor[2].LdCurrentSet = 0;
						g_s_DriverInfor[0].PWRStatus = SYSTEMS_OFF;
						g_s_DriverInfor[1].PWRStatus = SYSTEMS_OFF;
						g_s_DriverInfor[2].PWRStatus = SYSTEMS_OFF;
				}	
			}
		}
		else
		{
			vTaskSuspend(OtherEntryHandle);
		}
	}
}


//***************************************************
//函数名  :  SystemsDataInit
//功能说明： 功能设置任务
//输入说明： void
//返回值  ：
//作者/时间/版本号:  liuwei/201507014
//***************************************************
void SystemsDataInit(void)
{
	double reg_double = 0;
	uint8_t i2c_err = 0;
	uint8_t reg_flag = 0;
	
	g_s_SystemInfor.BurnInMode = BURNIN_ACC_MODE;			//老化加电模式 ACC恒流老化  APC恒背光老化
	
	g_s_SystemInfor.DriverLiveBit = 0x0000;					//在位检测驱动板
	GetDriverOnline(&g_s_SystemInfor.DriverLiveBit);
	
	g_s_SystemInfor.DriverTypeBit = 0x0000;				//监控驱动板类型
	GetDriverTypes(&g_s_SystemInfor.DriverTypeBit);
	
	g_s_SystemInfor.Temp_1st = 0;							//驱动板温度监控1
	g_s_SystemInfor.Temp_2nd = 0;							//驱动板温度监控2

	g_s_SystemInfor.APCLimitCurrent = 0;					//设置APC模块下LD的驱动电流保护值
	g_s_SystemInfor.ImPdOffSet = 0;							//背光调整驱动电流允许偏差值
	
	g_s_SystemInfor.PowerStatus = SYSTEMS_OFF;              //系统状态
	g_s_SystemInfor.CurrPort = 1;							//当前扫描
	g_s_SystemInfor.CurrSamp = 1;							//当前监控
	g_s_SystemInfor.CurrDriver = 1;								//当前驱动板
	
	g_s_SystemInfor.SetVpdValue = 0;							//PD设置值
	g_s_SystemInfor.SetVmpdValue = 0;							//PD设置值

	g_s_SystemInfor.SetStepNum = 10;						//设置阶梯次数
	g_s_SystemInfor.SetDelayNum = 1;
	
	//Ith扫描条件
	g_s_SystemInfor.ScanStart = 0;     					 	//LD扫描起点
	g_s_SystemInfor.ScanStep = 0;      					 	//LD扫描步进
	g_s_SystemInfor.ScanStop = 0;      					 	//LD扫描终点
	g_s_SystemInfor.IthScanNum = 1;							//扫描到数据长度	
	g_s_SystemInfor.ScanPort = 1;							//需要扫描Ith的端口
	g_s_SystemInfor.ScanPD = 1;								//需要扫描PD的端口
	g_s_SystemInfor.ScanMPD = 1;							//需要扫描MPD的端口
	g_s_SystemInfor.IthStatus = 0;                    		//Ith测试状态
	
	g_s_SystemInfor.AllsetStatus = 0;						//全部加电设置状态
	
	g_s_SystemInfor.PDLevel = 0;							//PD端电流档位 0~2档位 0为小范围  2为大范围
	g_s_SystemInfor.MPDLevel = 0;							//PD端电流档位 0~2档位 0为小范围  2为大范围

	g_s_SystemInfor.DutVoltGetK = -0.0007118;         				//电压采样校准K
	g_s_SystemInfor.DutVoltGetB = -4.163;            				//电压采样校准B

	g_s_SystemInfor.PDGet_1st_K = 0.01875;        					//0V电压下PD背光电流采样值校准参数K
	g_s_SystemInfor.PDGet_1st_B = -2.025;         					//0V电压下PD背光电流采样值校准参数B
																		
	g_s_SystemInfor.PDGet_2nd_K = 0.03753;        					//0V电压下PD背光电流采样值校准参数K
	g_s_SystemInfor.PDGet_2nd_B = -0.8883;        					//0V电压下PD背光电流采样值校准参数B
				
	g_s_SystemInfor.PDGet_3rd_K = 0.06266;        					//0V电压下PD背光电流采样值校准参数K
	g_s_SystemInfor.PDGet_3rd_B = -0.9609;        					//0V电压下PD背光电流采样值校准参数B
	
	g_s_SystemInfor.MPDGet_1st_K = 0.01875;       					//MPD背光电流采样值校准参数K
	g_s_SystemInfor.MPDGet_1st_B = -2.025;        					//MPD背光电流采样值校准参数B
															  
	g_s_SystemInfor.MPDGet_2nd_K = 0.03753;       					//MPD背光电流采样值校准参数K
	g_s_SystemInfor.MPDGet_2nd_B = -0.8883;       					//MPD背光电流采样值校准参数B
												  
	g_s_SystemInfor.MPDGet_3rd_K = 0.06266;       					//MPD背光电流采样值校准参数K
	g_s_SystemInfor.MPDGet_3rd_B = -0.9609;       					//MPD背光电流采样值校准参数B
	
	g_s_SystemInfor.Get_NTC_K = 0.01875;        						//TH采样校准系数K
	g_s_SystemInfor.Get_NTC_B = -2.025;         						//TH采样校准系数B
	
	g_s_SystemInfor.VpdDACSetK = 0.9079;         					//PD背光电流校准K
	g_s_SystemInfor.VpdDACSetB = -0.007963;      					//PD背光电流校准B
	
	g_s_SystemInfor.MPDDACSetK = 0.9079;         						//MPD电流校准K
	g_s_SystemInfor.MPDDACSetB = -0.007963;      						//MPD电流校准B

	//samp dealay
	g_s_SystemInfor.SystemErrType = SYSTEMS_NONE_ERROR;						//老化板在位
	g_s_SystemInfor.SystemAutoOutage = SYSTEMS_OFF;					//老化板拔出下电保护
	
	g_s_SystemInfor.OnChipFlag = 0;							//设置器件在位
	
	i2c_err = ReadE2promInfor(MAIN_I2C_CHN,MAIN_FLAG_ADDR,1, (uint8_t *)&reg_flag);
	if((reg_flag == MAIN_FLAG_VALUE)&&(i2c_err == I2C_CHIP_ERR_NONE))
	{
		ReadE2promInfor(MAIN_I2C_CHN, DUT_VOLT_ADDR*16,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.DutVoltGetK = reg_double;
		ReadE2promInfor(MAIN_I2C_CHN, DUT_VOLT_ADDR*16+8,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.DutVoltGetB = reg_double;

		ReadE2promInfor(MAIN_I2C_CHN, PD_1ST_ADDR*16,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.PDGet_1st_K = reg_double;
		ReadE2promInfor(MAIN_I2C_CHN, PD_1ST_ADDR*16+8,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.PDGet_1st_B = reg_double;
	
		ReadE2promInfor(MAIN_I2C_CHN, PD_2ND_ADDR*16,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.PDGet_2nd_K = reg_double;
		ReadE2promInfor(MAIN_I2C_CHN, PD_2ND_ADDR*16+8,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.PDGet_2nd_B = reg_double;
	
		ReadE2promInfor(MAIN_I2C_CHN, PD_3RD_ADDR*16,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.PDGet_3rd_K = reg_double;
		ReadE2promInfor(MAIN_I2C_CHN, PD_3RD_ADDR*16+8,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.PDGet_3rd_B = reg_double;

		ReadE2promInfor(MAIN_I2C_CHN, MPD_1ST_ADDR*16,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.MPDGet_1st_K = reg_double;
		ReadE2promInfor(MAIN_I2C_CHN, MPD_1ST_ADDR*16+8,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.MPDGet_1st_B = reg_double;

		ReadE2promInfor(MAIN_I2C_CHN, MPD_2ND_ADDR*16,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.MPDGet_2nd_K = reg_double;
		ReadE2promInfor(MAIN_I2C_CHN, MPD_2ND_ADDR*16+8,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.MPDGet_2nd_B = reg_double;

		ReadE2promInfor(MAIN_I2C_CHN, MPD_3RD_ADDR*16,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.MPDGet_3rd_K = reg_double;
		ReadE2promInfor(MAIN_I2C_CHN, MPD_3RD_ADDR*16+8,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.MPDGet_3rd_B = reg_double;

		ReadE2promInfor(MAIN_I2C_CHN, GET_NTC_ADDR*16,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.Get_NTC_K = reg_double;
		ReadE2promInfor(MAIN_I2C_CHN, GET_NTC_ADDR*16+8,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.Get_NTC_B = reg_double;

		ReadE2promInfor(MAIN_I2C_CHN, PD_SET_ADDR*16,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.VpdDACSetK = reg_double;
		ReadE2promInfor(MAIN_I2C_CHN, PD_SET_ADDR*16+8,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.VpdDACSetB = reg_double;
	
		ReadE2promInfor(MAIN_I2C_CHN, MPD_SET_ADDR*16,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.MPDDACSetK = reg_double;
		ReadE2promInfor(MAIN_I2C_CHN, MPD_SET_ADDR*16+8,8, (uint8_t *)&reg_double);
		g_s_SystemInfor.MPDDACSetB = reg_double;
	}
	
	//电压板限流设置    set_cl(voltage board used)
	SetFuncValue(14,4,2.5);
}

//***************************************************
//函数名  :  SystemsDataInit
//功能说明： 功能设置任务
//输入说明： void
//返回值  ：
//作者/时间/版本号:  liuwei/201507014
//***************************************************
void DriverBoradInit(void)
{
	uint8_t set_cnt = 0;
	uint8_t i2c_err = 0;
	double reg_double = 0;
	uint8_t reg_flag = 0;
	
	for(set_cnt=0;set_cnt<DRIVER_NUM;set_cnt++)
	{
		if(((g_s_SystemInfor.DriverLiveBit>>set_cnt*2&0x01) == 0)&&((g_s_SystemInfor.DriverLiveBit>>(set_cnt*2+1)&0x01) == 0))
			g_s_DriverInfor[set_cnt].BitStatus = DRIVER_BIT_VALID;
		else 
			g_s_DriverInfor[set_cnt].BitStatus = DRIVER_BIT_NULL;
		
		if(((g_s_SystemInfor.DriverTypeBit>>set_cnt*2&0x01) == 0)&&((g_s_SystemInfor.DriverTypeBit>>(set_cnt*2+1)&0x01) == 1))
		{
			g_s_DriverInfor[set_cnt].DriverType = DRIVER_TYPE_BOTH;	
			g_s_DriverInfor[set_cnt].SetPWR_Pos_K = -0.5419;    		
			g_s_DriverInfor[set_cnt].SetPWR_Pos_B = 4.608;    		
			g_s_DriverInfor[set_cnt].SetPWR_Neg_K = -0.3809;    		
			g_s_DriverInfor[set_cnt].SetPWR_Neg_B = -0.7362;   
		}
		else if(((g_s_SystemInfor.DriverTypeBit>>set_cnt*2&0x01) == 0)&&((g_s_SystemInfor.DriverTypeBit>>(set_cnt*2+1)&0x01) == 0))
		{
			g_s_DriverInfor[set_cnt].DriverType = DRIVER_TYPE_CURR;	
			g_s_DriverInfor[set_cnt].SetPWR_Pos_K = -0.1574;    		
			g_s_DriverInfor[set_cnt].SetPWR_Pos_B = 1.342;
			g_s_DriverInfor[set_cnt].SetPWR_Neg_K = 1;    		
			g_s_DriverInfor[set_cnt].SetPWR_Neg_B = 0;  
		}
		else if(((g_s_SystemInfor.DriverTypeBit>>set_cnt*2&0x01) == 1)&&((g_s_SystemInfor.DriverTypeBit>>(set_cnt*2+1)&0x01) == 0))
		{
			g_s_DriverInfor[set_cnt].DriverType = DRIVER_TYPE_VOLT;
			g_s_DriverInfor[set_cnt].SetPWR_Pos_K = 1;    		
			g_s_DriverInfor[set_cnt].SetPWR_Pos_B = 0;
			g_s_DriverInfor[set_cnt].SetPWR_Neg_K = -0.3809;    		
			g_s_DriverInfor[set_cnt].SetPWR_Neg_B = -0.7362;   
		}
		else 
		{
			g_s_DriverInfor[set_cnt].DriverType = DRIVER_TYPE_NULL;
		}

		g_s_DriverInfor[set_cnt].DriverNum = set_cnt+1;				
		g_s_DriverInfor[set_cnt].PWRStatus = 0;
		
		g_s_DriverInfor[set_cnt].PortOutput = 0x0000;			
		g_s_DriverInfor[set_cnt].LevelGear = 0;
		
		g_s_DriverInfor[set_cnt].PosPWRValue = 0;
		g_s_DriverInfor[set_cnt].NegPWRValue = 0;
		
		g_s_DriverInfor[set_cnt].SetValue = 0;
		g_s_DriverInfor[set_cnt].NowSetValue = 0;	
		
		g_s_DriverInfor[set_cnt].Temp_1st = 0;
		g_s_DriverInfor[set_cnt].Temp_2nd = 0;
		
		
		if(g_s_DriverInfor[set_cnt].BitStatus == DRIVER_BIT_VALID)
		{

			i2c_err = ReadE2promInfor(set_cnt,DRI_FLAG_ADDR,1, (uint8_t *)&reg_flag);
			if((reg_flag == DRI_FLAG_VALUE)&&(i2c_err == I2C_CHIP_ERR_NONE))
			{
				ReadE2promInfor(set_cnt,PWR_CAL_ADDR,8, (uint8_t *)&reg_double);
				g_s_DriverInfor[set_cnt].SetPWR_Pos_K = reg_double;
				ReadE2promInfor(set_cnt,PWR_CAL_ADDR+8,8, (uint8_t *)&reg_double);
				g_s_DriverInfor[set_cnt].SetPWR_Pos_B = reg_double;
				ReadE2promInfor(set_cnt,PWR_CAL_ADDR+16,8, (uint8_t *)&reg_double);
				g_s_DriverInfor[set_cnt].SetPWR_Neg_K = reg_double;
				ReadE2promInfor(set_cnt,PWR_CAL_ADDR+24,8, (uint8_t *)&reg_double);
				g_s_DriverInfor[set_cnt].SetPWR_Neg_B = reg_double;
			}
			
			DriverOutTCA9535(set_cnt+1,g_s_DriverInfor[set_cnt].PortOutput);
		}	
	}
}

//***************************************************
//函数名  :  PortInformationInit
//功能说明： 功能设置任务
//输入说明： void
//返回值  ：
//作者/时间/版本号:  liuwei/201507014
//***************************************************
void PortInformationInit(void)
{
	uint8_t set_cnt = 0;
	uint8_t i2c_err = 0;
	double reg_double = 0;
	uint8_t reg_flag = 0;

	//参数初始化
	for(set_cnt=0;set_cnt<DUT_MAX_NUM;set_cnt++)
	{
		g_s_DutputInfor[set_cnt].PortNum = set_cnt+1;								//端口编号
		g_s_DutputInfor[set_cnt].DriverNum = (set_cnt/DRIVER_SON)+1;				//驱动板号
		g_s_DutputInfor[set_cnt].DriverSon = set_cnt%DRIVER_SON;					//驱动子号

		g_s_DutputInfor[set_cnt].DacGroupNum = set_cnt/DAC_GROUP_VAULE;						//DAC分组
		g_s_DutputInfor[set_cnt].DacSonNum = set_cnt%DAC_GROUP_VAULE;	
		
		g_s_DutputInfor[set_cnt].ApcLDCurrentSet = 0;					//APC模式下设置电流
		g_s_DutputInfor[set_cnt].ApcLDCurrentNow = 0;					//APC模式下当前电流
		
		g_s_DutputInfor[set_cnt].Curr_GetValue = 0;            			//LD电流值
		g_s_DutputInfor[set_cnt].Volt_GetValue = 0;						//LD端电压值

		if(g_s_DriverInfor[g_s_DutputInfor[set_cnt].DriverNum-1].DriverType == DRIVER_TYPE_CURR)
		{			
			g_s_DutputInfor[set_cnt].SetValue_1st_K = 0.002338;          		//设定校准参数K1
			g_s_DutputInfor[set_cnt].SetValue_1st_B = 0.008338;  				//设定校准参数B1
			g_s_DutputInfor[set_cnt].GetValue_1st_K = 0.0551;   				//回采校准参数K1
			g_s_DutputInfor[set_cnt].GetValue_1st_B = 0.1071;   				//回采校准参数B1
			                                          
			g_s_DutputInfor[set_cnt].SetValue_2nd_K = 0.002344;          		//设定校准参数K1
			g_s_DutputInfor[set_cnt].SetValue_2nd_B = 0.006677;  				//设定校准参数B1
			g_s_DutputInfor[set_cnt].GetValue_2nd_K = 0.055; 					//回采校准参数K1
			g_s_DutputInfor[set_cnt].GetValue_2nd_B = 0.9542; 					//回采校准参数B1
		}
		else if(g_s_DriverInfor[g_s_DutputInfor[set_cnt].DriverNum-1].DriverType == DRIVER_TYPE_VOLT)
		{
			g_s_DutputInfor[set_cnt].SetValue_1st_K = -0.4043;        			//设定校准参数K1
			g_s_DutputInfor[set_cnt].SetValue_1st_B = 3.425e-05;				//设定校准参数B1
			g_s_DutputInfor[set_cnt].GetValue_1st_K = -0.002411;				//回采校准参数K1
			g_s_DutputInfor[set_cnt].GetValue_1st_B = -0.01911;					//回采校准参数B1
			                                          
			g_s_DutputInfor[set_cnt].SetValue_2nd_K = -0.4043;        			//设定校准参数K1
			g_s_DutputInfor[set_cnt].SetValue_2nd_B = 3.425e-05;				//设定校准参数B1
			g_s_DutputInfor[set_cnt].GetValue_2nd_K = -0.002411;      			//回采校准参数K1
			g_s_DutputInfor[set_cnt].GetValue_2nd_B = -0.01911;					//回采校准参数B1
		}
		else if(g_s_DriverInfor[g_s_DutputInfor[set_cnt].DriverNum-1].DriverType == DRIVER_TYPE_BOTH)
		{
			g_s_DutputInfor[set_cnt].SetValue_1st_K = -0.3947;        			//设定校准参数K1
			g_s_DutputInfor[set_cnt].SetValue_1st_B = 0.002086;					//设定校准参数B1
			g_s_DutputInfor[set_cnt].GetValue_1st_K = 1;      					//回采校准参数K1
			g_s_DutputInfor[set_cnt].GetValue_1st_B = -31.955;					//回采校准参数B1
			                                          
			g_s_DutputInfor[set_cnt].SetValue_2nd_K = -0.3947;        			//设定校准参数K1
			g_s_DutputInfor[set_cnt].SetValue_2nd_B = 0.002086;					//设定校准参数B1
			g_s_DutputInfor[set_cnt].GetValue_2nd_K = 1;      					//回采校准参数K1
			g_s_DutputInfor[set_cnt].GetValue_2nd_B = -31.955;					//回采校准参数B1
		}
		
		if(g_s_DriverInfor[g_s_DutputInfor[set_cnt].DriverNum-1].BitStatus == DRIVER_BIT_VALID)
		{
			i2c_err = ReadE2promInfor(g_s_DutputInfor[set_cnt].DriverNum,DRI_FLAG_ADDR,1, (uint8_t *)&reg_flag);
			if((reg_flag == DRI_FLAG_VALUE)&&(i2c_err == I2C_CHIP_ERR_NONE))
			{
				ReadE2promInfor(g_s_DutputInfor[set_cnt].DriverNum, g_s_DutputInfor[set_cnt].DriverSon*DUT_ADDR_SIZE+16*SET_1ST_ADDR,8, (uint8_t *)&reg_double);
				g_s_DutputInfor[set_cnt].SetValue_1st_K  = reg_double;
				ReadE2promInfor(g_s_DutputInfor[set_cnt].DriverNum, g_s_DutputInfor[set_cnt].DriverSon*DUT_ADDR_SIZE+16*SET_1ST_ADDR+8,8, (uint8_t *)&reg_double);
				g_s_DutputInfor[set_cnt].SetValue_1st_B = reg_double;
			
				ReadE2promInfor(g_s_DutputInfor[set_cnt].DriverNum, g_s_DutputInfor[set_cnt].DriverSon*DUT_ADDR_SIZE+16*GET_1ST_ADDR,8, (uint8_t *)&reg_double);
				g_s_DutputInfor[set_cnt].GetValue_1st_K  = reg_double;
				ReadE2promInfor(g_s_DutputInfor[set_cnt].DriverNum, g_s_DutputInfor[set_cnt].DriverSon*DUT_ADDR_SIZE+16*GET_1ST_ADDR+8,8, (uint8_t *)&reg_double);
				g_s_DutputInfor[set_cnt].GetValue_1st_B = reg_double;
			
				ReadE2promInfor(g_s_DutputInfor[set_cnt].DriverNum, g_s_DutputInfor[set_cnt].DriverSon*DUT_ADDR_SIZE+16*SET_2ND_ADDR,8, (uint8_t *)&reg_double);
				g_s_DutputInfor[set_cnt].SetValue_2nd_K  = reg_double;
				ReadE2promInfor(g_s_DutputInfor[set_cnt].DriverNum, g_s_DutputInfor[set_cnt].DriverSon*DUT_ADDR_SIZE+16*SET_2ND_ADDR+8,8, (uint8_t *)&reg_double);
				g_s_DutputInfor[set_cnt].SetValue_2nd_B = reg_double;
			
				ReadE2promInfor(g_s_DutputInfor[set_cnt].DriverNum, g_s_DutputInfor[set_cnt].DriverSon*DUT_ADDR_SIZE+16*GET_2ND_ADDR,8, (uint8_t *)&reg_double);
				g_s_DutputInfor[set_cnt].GetValue_2nd_K  = reg_double;
				ReadE2promInfor(g_s_DutputInfor[set_cnt].DriverNum, g_s_DutputInfor[set_cnt].DriverSon*DUT_ADDR_SIZE+16*GET_2ND_ADDR+8,8, (uint8_t *)&reg_double);
				g_s_DutputInfor[set_cnt].GetValue_2nd_B = reg_double;
			}
		}
	}
	
	for(set_cnt=0;set_cnt<SAMP_MAX_NUM;set_cnt++)
	{
		g_s_SampInInfor[set_cnt].SampNum = set_cnt+1;
		
		g_s_SampInInfor[set_cnt].PD_GetValue = 0;
		g_s_SampInInfor[set_cnt].MPD_GetValue = 0;
		g_s_SampInInfor[set_cnt].TH_GetValue = 0;
		
		g_s_SampInInfor[set_cnt].APCDutPort = set_cnt+1;
		g_s_SampInInfor[set_cnt].APCModePDValue = 0;
		
		g_s_SampInInfor[set_cnt].ApcPID_Kp = APC_PID_KP;
		g_s_SampInInfor[set_cnt].ApcPID_Ki = APC_PID_KI;
		g_s_SampInInfor[set_cnt].ApcPID_Kd = APC_PID_KD;
	}
}

//***************************************************
//函数名  :  PortApcPidInitial
//功能说明： 功能设置初始化
//					创建任务  初始化APC PID初始化
//输入说明： void
//返回值  ：
//***************************************************
void AllAPCPidInitial(void)
{
	uint8_t set_cnt=0;

	//参数初始化
	for(set_cnt=0;set_cnt<SAMP_MAX_NUM;set_cnt++)
	{
		PID_Initial(g_s_SampInInfor[set_cnt].ApcPID_Kp,g_s_SampInInfor[set_cnt].ApcPID_Ki,g_s_SampInInfor[set_cnt].ApcPID_Kd,&APC_PidRatio[set_cnt]);
	}
}

void PortAPCPidInitial(uint8_t set_cnt)
{
	if(set_cnt<SAMP_MAX_NUM)
		PID_Initial(g_s_SampInInfor[set_cnt].ApcPID_Kp,g_s_SampInInfor[set_cnt].ApcPID_Ki,g_s_SampInInfor[set_cnt].ApcPID_Kd,&APC_PidRatio[set_cnt]);
}


//***************************************************
//函数名  :  FunctionSetInit
//功能说明： 功能设置初始化
//					创建任务  初始化Dac Adc
//输入说明： void
//返回值  ：
//***************************************************
void FunctionSetInit(void)
{
	BaseType_t CreatStatus;

	//创建Ith扫描信号量
	ScanIthSemphr = xSemaphoreCreateBinary();
	if(ScanIthSemphr == NULL )
		return ;
	
	//创建DUT采样信号量
	SampDutSemphr = xSemaphoreCreateBinary();
	if(SampDutSemphr == NULL)
		return ;
	
	//创建Monit采样信号量
	SampMonitSemphr = xSemaphoreCreateBinary();
	if(SampMonitSemphr == NULL)
		return ;
	
	//其他功能采样信号量
	OtherItemSemphr = xSemaphoreCreateBinary();
	if(OtherItemSemphr == NULL)
		return ;
	
	//全加电型号量
	SetupALLSemphr = xSemaphoreCreateBinary();
	if(SetupALLSemphr == NULL)
		return ;
	
	CreatStatus = xTaskCreate(SetupAllEntry,"SetupAll",SETUP_ALL_SIZE,(void*)SetupALLSemphr,SETUP_ALL_PRIO,NULL);	
	if(CreatStatus != pdPASS)
		return ;

	CreatStatus = xTaskCreate(ScanLDIthEntry,"ScanLDIth",SCAN_ITH_SIZE,(void*)ScanIthSemphr,SCAN_ITH_PRIO,NULL);	
	if(CreatStatus != pdPASS)
		return ;

	CreatStatus = xTaskCreate(SampDutEntry,"SampDut",SAMP_DUT_SIZE,(void*)SampDutSemphr,SAMP_DUT_PRIO,&SampDutHandle);
	if(CreatStatus != pdPASS)
		return ;
	
	CreatStatus = xTaskCreate(SampMonitEntry,"SampMonit",SAMP_MONIT_SIZE,(void*)SampMonitSemphr,SAMP_MONIT_PRIO,&SampMonitHandle);
	if(CreatStatus != pdPASS)
		return ;
	
	CreatStatus = xTaskCreate(OtherItemEntry,"OtherItem",OTHER_ITEM_SIZE,(void*)OtherItemSemphr,OTHER_ITEM_PRIO,&OtherEntryHandle);
	if(CreatStatus != pdPASS)
		return ;
}

