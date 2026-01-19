#ifndef _FunctionSet_
#define _FunctionSet_

#include <stdint.h>

#define 	DELAY_US_CNT 					480000000/2000000
#define 	DELAY_MS_CNT 					480000000/2000

//最大端口数量
#define 	DUT_MAX_NUM						96		//输出通道
#define 	SAMP_MAX_NUM					48		//采样通道

#define     DAC_GROUP_VAULE					8		//DAC通道数

#define     DRIVER_SON						16		//驱动板通道数
#define     DRIVER_NUM						6		//驱动板数

//外部DAC参数定义
#define 	SGM_DAC_BIT						4095
#define 	SGM_DAC_VREF					2.5

//外部DAC参数定义
#define 	ADS_MUX_BIT						32768
//#define 	ADS_MUX_VREF					6.144

//默认APC_PID调节参数
#define     APC_PID_KP						0.005
#define     APC_PID_KI						0.015
#define     APC_PID_KD						0.01

//阈值扫描最大点数
#define     ITH_MAX_COUNT     				1024

//系统错误
#define 	SYSTEMS_NONE_ERROR				0
#define 	SYSTEMS_DRIVER_ERROR			1

//系统开关状态
#define 	SYSTEMS_OFF						0
#define 	SYSTEMS_ON						1

//I2C通道定义
#define 	MAIN_I2C_CHN					8

//主板参数地址
#define		DUT_VOLT_ADDR					0
#define		PD_1ST_ADDR						1
#define		PD_2ND_ADDR						2
#define		PD_3RD_ADDR						3
#define		MPD_1ST_ADDR					4
#define		MPD_2ND_ADDR					5
#define		MPD_3RD_ADDR					6
#define		GET_NTC_ADDR					7
#define		PD_SET_ADDR						8
#define		MPD_SET_ADDR					9

#define     MAIN_FLAG_ADDR					0x100
#define     MAIN_FLAG_VALUE					0x77

//Driver板校准系数
#define     SET_1ST_ADDR					0
#define     GET_1ST_ADDR					1
#define     SET_2ND_ADDR					2
#define     GET_2ND_ADDR					3

#define     DUT_ADDR_SIZE					64
#define     PWR_CAL_ADDR					0x500
#define     DRI_FLAG_ADDR					0x600
#define     DRI_FLAG_VALUE					0x89

//老化设置ACC,APC模式
typedef enum
{
	BURNIN_ACC_MODE = 0,				//ACC模式恒流驱动
	BURNIN_APC_MODE = 1					//APC模式背光反馈
}BURNIN_MODE_TYPE;

typedef enum						//驱动板类型
{
	DRIVER_TYPE_NULL = 0,			//无类型
	DRIVER_TYPE_CURR = 1,			//电流型驱动板
	DRIVER_TYPE_VOLT = 2,			//电压型驱动板
	DRIVER_TYPE_BOTH = 3			//双类型驱动板
}SET_DRIVER_TYPE;

typedef enum						//驱动板在位状态
{
	DRIVER_BIT_NULL  = 0,			//驱动板不在位
	DRIVER_BIT_VALID = 1			//驱动板在位
}DRIVER_BIT_STATUS;

//输出端口数据类型
typedef struct 
{
	uint8_t PortNum;				//端口编号
	
	uint8_t DriverNum;				//驱动板号
	uint8_t DriverSon;				//
			
	uint8_t DacGroupNum;			//DAC分组
	uint8_t DacSonNum;	

	double ApcLDCurrentSet;			//APC模式下设置电流
	double ApcLDCurrentNow;			//APC模式下当前电流
	
	double Curr_GetValue;           //电流值
	double Volt_GetValue;			//电压值

	double SetValue_1st_K;         	//设定校准参数K1
	double SetValue_1st_B;         	//设定校准参数B1
	double GetValue_1st_K;         	//回采校准参数K1
	double GetValue_1st_B;         	//回采校准参数B1
	
	double SetValue_2nd_K;         	//设定校准参数K2
	double SetValue_2nd_B;         	//设定校准参数B2
	double GetValue_2nd_K;         	//回采校准参数K2
	double GetValue_2nd_B;         	//回采校准参数B2
}DUT_TypeStruct;


//回踩监控数据类型
typedef struct 
{
	uint8_t SampNum;				//端口编号

	double PD_GetValue;             //PD电流值		
	double TH_GetValue;             //NTC监控值	
	double MPD_GetValue;            //MPD电流值	

	uint8_t APCDutPort;				//APC模式下控制DUT通道
	double APCModePDValue;			//设置APC模式下PD设置值

	//PID参数校准					APC恒功率模式 PID参数
	double ApcPID_Kp;
	double ApcPID_Ki;
	double ApcPID_Kd;
}SampIn_TypeStruct;

//端口数据类型
typedef struct 
{
	SET_DRIVER_TYPE   DriverType;				//设置驱动板类型
	DRIVER_BIT_STATUS BitStatus;				//驱动板在位状态
	
	uint8_t DriverNum;							//端口编号	
	uint8_t PWRStatus;							//电源状态
	
	uint16_t PortOutput;						//驱动板每个端口输出状态 0xFFFF 标识输出状态
	uint8_t LevelGear;           				//档位，1为1000mA大电流，0为50mA电流档, BOTH切换电压型还是电流型
	
	double PosPWRValue;							//设置电源板值
	double NegPWRValue;							//设置电源板值

	double SetValue;           					//需要设定的LD驱动电流值
	double NowSetValue;      	 				//LD驱动电流当前设定值

	double SetPWR_Pos_K;      					//正电源PWR设置系数
	double SetPWR_Pos_B;      					//正电源PWR设置系数
	double SetPWR_Neg_K;      					//负电源PWR设置系数
	double SetPWR_Neg_B;      					//正电源PWR设置系数
	
	double Temp_1st;							//驱动板温度监控1
	double Temp_2nd;							//驱动板温度监控1
}Driver_TypeStruct;

//Systems状态参数
typedef struct 
{
	BURNIN_MODE_TYPE BurnInMode;				//老化加电模式 ACC恒流老化  APC恒背光老化
	
	uint16_t DriverLiveBit;						//在位检测驱动板
	uint16_t DriverTypeBit;						//监控驱动板类型
	
	double Temp_1st;							//驱动板温度监控1
	double Temp_2nd;							//驱动板温度监控2
	double RH_Value;							//湿度监控

	double APCLimitCurrent;						//设置APC模块下LD的驱动电流保护值
	double ImPdOffSet;							//背光调整驱动电流允许偏差值
	
	uint8_t PowerStatus;                  		//系统状态
	uint8_t CurrPort;							//当前扫描
	uint8_t CurrSamp;							//当前监控
	uint8_t CurrDriver;							//当前驱动板
	
	double SetVpdValue;							//PD设置值
	double SetVmpdValue;							//PD设置值

	uint16_t SetStepNum;						//设置阶梯次数
	uint16_t SetDelayNum;
	
	//Ith扫描条件
	double ScanStart;     					 	//LD扫描起点
	double ScanStep;      					 	//LD扫描步进
	double ScanStop;      					 	//LD扫描终点
	uint16_t IthScanNum;						//扫描到数据长度	
	
	uint8_t ScanPort;							//需要扫描Ith的端口
	uint8_t ScanPD;								//需要扫描PD的端口
	uint8_t ScanMPD;							//需要扫描MPD的端口
	uint8_t IthStatus;                    		//Ith测试状态
	
	uint8_t AllsetStatus;						//全部加电设置状态
	
	uint8_t PDLevel;							//PD端电流档位 0~2档位 0为小范围  2为大范围
	uint8_t MPDLevel;							//PD端电流档位 0~2档位 0为小范围  2为大范围

	double DutVoltGetK;         					//电压采样校准K
	double DutVoltGetB;         					//电压采样校准B

	double VpdDACSetK;      					//PD背光电流校准K
	double VpdDACSetB;      					//PD背光电流校准B

	double PDGet_1st_K;        					//0V电压下PD背光电流采样值校准参数K
	double PDGet_1st_B;        					//0V电压下PD背光电流采样值校准参数B
																		
	double PDGet_2nd_K;        					//0V电压下PD背光电流采样值校准参数K
	double PDGet_2nd_B;        					//0V电压下PD背光电流采样值校准参数B
				
	double PDGet_3rd_K;        					//0V电压下PD背光电流采样值校准参数K
	double PDGet_3rd_B;        					//0V电压下PD背光电流采样值校准参数B
	
	double MPDDACSetK;      					//MPD电流校准K
	double MPDDACSetB;      					//MPD电流校准B

	double MPDGet_1st_K;        				//MPD背光电流采样值校准参数K
	double MPDGet_1st_B;        				//MPD背光电流采样值校准参数B
												  
	double MPDGet_2nd_K;        				//MPD背光电流采样值校准参数K
	double MPDGet_2nd_B;        				//MPD背光电流采样值校准参数B
												  
	double MPDGet_3rd_K;        				//MPD背光电流采样值校准参数K
	double MPDGet_3rd_B;        				//MPD背光电流采样值校准参数B
	
	double Get_NTC_K;        					//TH采样校准系数K
	double Get_NTC_B;        					//TH采样校准系数B

	//系统信息参数
	uint8_t I2cSlaveStatus;								//硬件i2c状态
	uint8_t VirtualI2cStatus;							//模拟i2c状态

	//samp dealay
	uint8_t SystemErrType;						//老化板在位
	uint8_t SystemAutoOutage;					//老化板拔出下电保护
	
	uint64_t OnChipFlag;							//设置器件在位
}System_TypeStruct;

void SystemsDataInit(void);
void DriverBoradInit(void);
void PortInformationInit(void);

void LDCurrentSetBit_OneStep(Driver_TypeStruct *driver,DUT_TypeStruct* port);   //单路设置电流
void LDCurrentSetdown(Driver_TypeStruct *driver,double curr);
void LDCurrentSetdown_OneStep(Driver_TypeStruct *driver,double curr);
void AllAPCPidInitial(void);
void PortAPCPidInitial(uint8_t set_cnt);

void DriverDelayBlockUs(uint16_t SetCnt);
void DriverDelayBlockMs(uint16_t SetCnt);
void SetFuncValue(uint8_t type,uint8_t chn,double dac);

void FunctionSetInit(void);				//函数设置初始化

#endif


