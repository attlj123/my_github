#include "Driver_I2C.h"

#include "SGM5348.h"
#include "ADS1115.h"
#include "TCA9535.h"
#include "TCA9548.h"
#include "TMP112.h"
#include "AT24CM01.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "app_cfg.h"

#include "ControlParam.h"

//I2C配置
I2C_TypeStruct       g_s_I2C1Structure;									//i2c功能配置
I2C_TypeStruct       g_s_I2C3Structure;									//i2c功能配置

//TCA9548   I2C扩展配置
TCA9548_TypeStruct	 g_s_TCA9548_I2CSwitch;								//i2c切换

//TCA9535设置
TCA9535_TypeStruct   g_s_TCA9535_OnLine;								//TCA9535在位检测 DRVIER
TCA9535_TypeStruct   g_s_TCA9535_GetType;								//TCA9535检测 DriverType
TCA9535_TypeStruct   g_s_TCA9535_CtrlSamp;								//TCA9535  DUT采样切换
TCA9535_TypeStruct	 g_s_TCA9535_PWRLevel;								//TCA9535  PWR 使能以及驱动板档位切换
TCA9535_TypeStruct   g_s_TCA9535_DACCtrl;								//TCA9535  DAC 控制
TCA9535_TypeStruct   g_s_TCA9535_CtrlPD;								//TCA9535  MPD PD TH采样切换

//Drive 板上I2C控制
TCA9535_TypeStruct 	 g_s_TCA9535_DutSwitch;								//TCA9535  DUT输出控制状态
TMP112_TypeStruct	 g_s_TMP112_1stTemper;								//TMP112   温度监控1
TMP112_TypeStruct	 g_s_TMP112_2ndTemper;								//TMP112   温度监控2

ADS1115_TypeStruct   g_s_ADS1115_DUTVoltage;							//地址 0x90
ADS1115_TypeStruct   g_s_ADS1115_DUTCurrent;							//地址 0x92
ADS1115_TypeStruct   g_s_ADS1115_PDValue;								//地址 0x96
ADS1115_TypeStruct   g_s_ADS1115_MpdThValue;							//地址 0x94

SemaphoreHandle_t 	 LinkLedSemphr = NULL;
SemaphoreHandle_t 	 FirstLedSemphr = NULL;		
SemaphoreHandle_t 	 SecondLedSemphr = NULL;
SemaphoreHandle_t 	 ThirdLedSemphr = NULL;

TaskHandle_t 		 FirstLedHandle;					//采样DUT线程
TaskHandle_t 		 SecondLedHandle;					//采样监控现场
TaskHandle_t         ThirdLedHandle;

extern System_TypeStruct   g_s_SystemInfor;
//***************************************************
//函数名  :  I2cSamplingChipInit
//功能说明： 初始化I2C2以及采样ADC初始化
//输入说明： void
//***************************************************
uint8_t I2cSamplingChipInit(void)
{
	//通讯I2C配置
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;

	g_s_I2C3Structure.Name = I2C3;
	g_s_I2C3Structure.ClockValue = 0x10C0ECFF;   //100k
	g_s_I2C3Structure.Address = 0x32;
	g_s_I2C3Structure.ErrorMessage = I2C_NO_ERR;
	Driver_I2C_Init(g_s_I2C3Structure);

	//配置ADS1115   采样LD电压初始化
	g_s_ADS1115_DUTVoltage.Address = 0x90;
	g_s_I2C3Structure.ErrorMessage = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_DUTVoltage);
	if(g_s_I2C3Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_ADS1115_VOLT_ERR|err_i2c;
	else
	{
		g_s_ADS1115_DUTVoltage.RegData = 0x00e3;				//配置寄存器参数  0x00e3   			 vef：+/-6.144V
		ADS1115WriteReg(g_s_I2C3Structure,ADS1115_REGADD_CFG,&g_s_ADS1115_DUTVoltage);
	}
	
	//配置ADS1115   采样LD电流初始化
	g_s_ADS1115_DUTCurrent.Address = 0x92;
	g_s_I2C3Structure.ErrorMessage = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_DUTCurrent);
	if(g_s_I2C3Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_ADS1115_CURR_ERR|err_i2c;
	else
	{
		g_s_ADS1115_DUTCurrent.RegData = 0x42e3;			//配置寄存器参数							vef：+/-2.048V
		ADS1115WriteReg(g_s_I2C3Structure,ADS1115_REGADD_CFG,&g_s_ADS1115_DUTCurrent);
	}
	
	//配置ADS1115   采样PD前光初始化
	g_s_ADS1115_PDValue.Address = 0x96;
	g_s_I2C3Structure.ErrorMessage = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_PDValue);
	if(g_s_I2C3Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_ADS1115_PD_ERR|err_i2c;
	else
	{
		g_s_ADS1115_PDValue.RegData = 0x44e3;			//配置寄存器参数							vef：+/-2.048V
		ADS1115WriteReg(g_s_I2C3Structure,ADS1115_REGADD_CFG,&g_s_ADS1115_PDValue);
	}
	
	//配置ADS1115   采样MPD,TH初始化
	g_s_ADS1115_MpdThValue.Address = 0x94;
	g_s_I2C3Structure.ErrorMessage = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_MpdThValue);
	if(g_s_I2C3Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_ADS1115_MPD_ERR|err_i2c;
	else
	{
		g_s_ADS1115_MpdThValue.RegData = 0x44e3;			//配置寄存器参数							vef：+/-2.048V
		ADS1115WriteReg(g_s_I2C3Structure,ADS1115_REGADD_CFG,&g_s_ADS1115_MpdThValue);
	}

	return err_i2c;
}


//***************************************************
//函数名  :  I2cMainControlInit
//功能说明： 初始化I2C1以及主板控制初始化
//输入说明： void
//***************************************************
uint8_t I2cMainControlInit(void)
{
	//通讯I2C配置
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;

	g_s_I2C1Structure.Name = I2C1;
	g_s_I2C1Structure.ClockValue = 0x10C0ECFF;   //100k
	g_s_I2C1Structure.Address = 0x31;
	g_s_I2C1Structure.ErrorMessage = I2C_NO_ERR;
	Driver_I2C_Init(g_s_I2C1Structure);
	
	//TCA9548  I2C复用切换
	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;

	//TCA9535   驱动板在位
	g_s_TCA9535_OnLine.Address = 0x40;
	g_s_TCA9535_OnLine.Port0_CfgReg = 0xFF;		//读取
	g_s_TCA9535_OnLine.Port1_CfgReg = 0xFF;
	g_s_I2C1Structure.ErrorMessage = TCA9535ReadBit(g_s_I2C1Structure,&g_s_TCA9535_OnLine);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_ONL_ERR|err_i2c;
	else
		TCA9535ConfigInit(g_s_I2C1Structure,&g_s_TCA9535_OnLine);
	
	//TCA9535   驱动板类型
	g_s_TCA9535_GetType.Address = 0x42;
	g_s_TCA9535_GetType.Port0_CfgReg = 0xFF;		//读取
	g_s_TCA9535_GetType.Port1_CfgReg = 0xFF;
	g_s_I2C1Structure.ErrorMessage = TCA9535ReadBit(g_s_I2C1Structure,&g_s_TCA9535_GetType);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_TYPE_ERR|err_i2c;
	else
		TCA9535ConfigInit(g_s_I2C1Structure,&g_s_TCA9535_GetType);
	
	//TCA9535   DUT采样控制
	g_s_TCA9535_CtrlSamp.Address = 0x44;
	g_s_TCA9535_CtrlSamp.Port0_CfgReg = 0x00;		//输出
	g_s_TCA9535_CtrlSamp.Port1_CfgReg = 0x00;
	g_s_I2C1Structure.ErrorMessage = TCA9535ReadBit(g_s_I2C1Structure,&g_s_TCA9535_CtrlSamp);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_SAMP_ERR|err_i2c;
	else
		TCA9535ConfigInit(g_s_I2C1Structure,&g_s_TCA9535_CtrlSamp);
	
	//TCA9535   PWR，档位控制
	g_s_TCA9535_PWRLevel.Address = 0x46;
	g_s_TCA9535_PWRLevel.Port0_CfgReg = 0x00;		//输出
	g_s_TCA9535_PWRLevel.Port1_CfgReg = 0x00;
	g_s_I2C1Structure.ErrorMessage = TCA9535ReadBit(g_s_I2C1Structure,&g_s_TCA9535_PWRLevel);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_PWR_ERR|err_i2c;
	else
		TCA9535ConfigInit(g_s_I2C1Structure,&g_s_TCA9535_PWRLevel);
	
	//TCA9535   DAC控制切换
	g_s_TCA9535_DACCtrl.Address = 0x48;
	g_s_TCA9535_DACCtrl.Port0_CfgReg = 0x00;		//输出
	g_s_TCA9535_DACCtrl.Port1_CfgReg = 0x00;
	g_s_I2C1Structure.ErrorMessage = TCA9535ReadBit(g_s_I2C1Structure,&g_s_TCA9535_DACCtrl);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_DAC_ERR|err_i2c;
	else
		TCA9535ConfigInit(g_s_I2C1Structure,&g_s_TCA9535_DACCtrl);
	
	//TCA9535   PD,MPD,TH控制切换
	g_s_TCA9535_CtrlPD.Address = 0x4A;
	g_s_TCA9535_CtrlPD.Port0_CfgReg = 0x00;			//输出
	g_s_TCA9535_CtrlPD.Port1_CfgReg = 0x00;
	g_s_I2C1Structure.ErrorMessage = TCA9535ReadBit(g_s_I2C1Structure,&g_s_TCA9535_CtrlPD);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_PD_ERR|err_i2c;
	else
		TCA9535ConfigInit(g_s_I2C1Structure,&g_s_TCA9535_CtrlPD);

	return err_i2c;
}


//***************************************************
//函数名  :  I2cDriverCtrlInit
//功能说明： 初始化I2C1以及主板控制初始化
//输入说明： void
//***************************************************
uint8_t I2cDriverCtrlInit(void)
{
	//通讯I2C配置
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	uint8_t i = 0;

	for(i=0;i<DRIVER_NUM;i++)
	{
		switch(i+1)
		{
			case 1:
				g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_1;break;
			case 2:
				g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_2;break;
			case 3:
				g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_3;break;
			case 4:
				g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_4;break;
			case 5:
				g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_5;break;
			case 6:
				g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_6;break;
			case 7:
				g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_7;break;
			case 8:
				g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;break;
			default:break;
		}
		
		g_s_TCA9548_I2CSwitch.Address = 0xEE;
		g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
		if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
			err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;

		//TCA9535   驱动板输出
		g_s_TCA9535_DutSwitch.Address = 0x40;
		g_s_TCA9535_DutSwitch.Port0_CfgReg = 0x00;			//输出
		g_s_TCA9535_DutSwitch.Port1_CfgReg = 0x00;
		g_s_I2C1Structure.ErrorMessage = TCA9535ReadBit(g_s_I2C1Structure,&g_s_TCA9535_DutSwitch);
		if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
			err_i2c = I2C_TCA9535_DRI_ERR|err_i2c;
		else
			TCA9535ConfigInit(g_s_I2C1Structure,&g_s_TCA9535_DutSwitch);
		
		//TMP112   1st温度传感器
		g_s_TMP112_1stTemper.Address = 0x90;
		g_s_I2C1Structure.ErrorMessage = Tmp112ReadTemperature(g_s_I2C1Structure,&g_s_TMP112_1stTemper);
		if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
			err_i2c = I2C_TMP112_1ST_ERR|err_i2c;
		
		//TMP112   2nd温度传感器
		g_s_TMP112_2ndTemper.Address = 0x92;
		g_s_I2C1Structure.ErrorMessage = Tmp112ReadTemperature(g_s_I2C1Structure,&g_s_TMP112_2ndTemper);
		if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
			err_i2c = I2C_TMP112_2ND_ERR|err_i2c;

	}

	return err_i2c;
}
	


//***************************************************
//函数名  :  GetDriverOnline
//功能说明： 获取
//输入说明： num 1~64
//***************************************************
uint8_t GetDriverOnline(uint16_t *online)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	uint16_t Bit_LO = 0;
	uint16_t Bit_HI = 0;

	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;
	
	g_s_I2C1Structure.ErrorMessage = TCA9535ReadBit(g_s_I2C1Structure,&g_s_TCA9535_OnLine);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_ONL_ERR|err_i2c;
	
	Bit_LO = g_s_TCA9535_OnLine.Port0_Input;
	Bit_HI = g_s_TCA9535_OnLine.Port1_Input;

	*online = Bit_HI<<8|Bit_LO;
	
	return err_i2c;
}

//***************************************************
//函数名  :  GetDriverTypes
//功能说明： 获取
//输入说明： num 1~64
//***************************************************
uint8_t GetDriverTypes(uint16_t *types)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	uint16_t Bit_LO = 0;
	uint16_t Bit_HI = 0;

	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;
	
	g_s_I2C1Structure.ErrorMessage = TCA9535ReadBit(g_s_I2C1Structure,&g_s_TCA9535_GetType);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_TYPE_ERR|err_i2c;
	
	Bit_LO = g_s_TCA9535_GetType.Port0_Input;
	Bit_HI = g_s_TCA9535_GetType.Port1_Input;

	*types = Bit_HI<<8|Bit_LO;
	
	return err_i2c;
}

//***************************************************
//函数名  :  SwithcDUTSamp
//功能说明： 切换DUT电流、电压采样通道
//输入说明： num 1~96
//***************************************************
uint8_t SwitchDUTSamp(uint8_t num)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	
	uint8_t chl_bit = (num-1)%16;
	uint8_t dri_bit = (num-1)/16;
	uint8_t en_bit = 0;
	
	switch(dri_bit)
	{
		case 0:en_bit=0x41;break;
		case 1:en_bit=0x42;break;
		case 2:en_bit=0x44;break;
		case 3:en_bit=0x48;break;
		case 4:en_bit=0x50;break;
		case 5:en_bit=0x60;break;
		default:break;
	}
	
	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;
	
	g_s_TCA9535_CtrlSamp.Port0_Output = 0x00;		//输出 SAMP_NONE_DISABLE
	g_s_TCA9535_CtrlSamp.Port1_Output = 0x00;
	g_s_I2C1Structure.ErrorMessage = TCA9535SetOutput(g_s_I2C1Structure,&g_s_TCA9535_CtrlSamp);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_SAMP_ERR|err_i2c;
	
	g_s_TCA9535_CtrlSamp.Port0_Output = (dri_bit<<4)|chl_bit;		//输出 切换   驱动板位号左移4bit;
	g_s_TCA9535_CtrlSamp.Port1_Output = en_bit;
	g_s_I2C1Structure.ErrorMessage = TCA9535SetOutput(g_s_I2C1Structure,&g_s_TCA9535_CtrlSamp);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_SAMP_ERR|err_i2c;
	
	return err_i2c;
}


//***************************************************
//函数名  :  SwitchMPDTH
//功能说明：切换TH,PD,MPD监控
//输入说明： num 1~64
//***************************************************
uint8_t SwitchMPDTH(uint8_t num)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	
	uint8_t chl_bit = (num-1)%16;
	uint8_t dri_bit = (num-1)/16;
	uint8_t en_bit = 0;
	
	switch(dri_bit)
	{
		case 0:en_bit=0x10;break;
		case 1:en_bit=0x20;break;
		case 2:en_bit=0x40;break;
		case 3:en_bit=0x80;break;
		default:break;
	}
	
	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;
	
	g_s_TCA9535_CtrlPD.Port0_Output = 0x00;					//输出 SAMP_NONE_DISABLE
	g_s_I2C1Structure.ErrorMessage = TCA9535SetOutput(g_s_I2C1Structure,&g_s_TCA9535_CtrlPD);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_PD_ERR|err_i2c;
	
	g_s_TCA9535_CtrlPD.Port0_Output = en_bit|chl_bit;		//输出 切换   驱动板位号左移4bit;
	g_s_I2C1Structure.ErrorMessage = TCA9535SetOutput(g_s_I2C1Structure,&g_s_TCA9535_CtrlPD);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_PD_ERR|err_i2c;
	
	return err_i2c;
}

//***************************************************
//函数名  :  MpdthTCA9535Bit
//功能说明：tca设置bit
//输入说明： num 1~64
//***************************************************
uint8_t MpdthTCA9535Bit(uint8_t bit,uint8_t statu)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	uint8_t read_reg = 0;
	
	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;
	
	read_reg = g_s_TCA9535_CtrlPD.Port1_Output;

	switch(bit)
	{
		case 1:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_1ST_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_1ST_DISABLE;	
			break;
		case 2:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_2ND_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_2ND_DISABLE;	
			break;
		case 3:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_3RD_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_3RD_DISABLE;		
			break;
		case 4:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_4TH_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_4TH_DISABLE;
			break;				
		case 5:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_5TH_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_5TH_DISABLE;
			break;				
		case 6:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_6TH_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_6TH_DISABLE;
			break;				
		case 7:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_7TH_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_7TH_DISABLE;
			break;				
		case 8:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_8TH_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_8TH_DISABLE;
			break;				
		default:break;
	}
	
	g_s_TCA9535_CtrlPD.Port1_Output = read_reg;

	g_s_I2C1Structure.ErrorMessage = TCA9535SetOutput(g_s_I2C1Structure,&g_s_TCA9535_CtrlPD);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_PD_ERR|err_i2c;
	
	return err_i2c;
}

//***************************************************
//函数名  :  MainTCA9535Bit
//功能说明： 设置pwr输出状态
//输入说明： bit 1~6 		statu ENABLE/DISABLE      Port0_Output PWR 使能, Port1_Output  level档位切换 
//***************************************************
uint8_t MainTCA9535Bit(uint8_t reg,uint8_t bit,uint8_t statu)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	uint8_t read_reg = 0;
	
	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;
	
	if(reg == TCA9535_REG_OUT_0)
		read_reg = g_s_TCA9535_PWRLevel.Port0_Output;
	else 	if(reg == TCA9535_REG_OUT_1)
		read_reg = g_s_TCA9535_PWRLevel.Port1_Output;

	switch(bit)
	{
		case 0:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_1ST_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_1ST_DISABLE;	
			break;
		case 1:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_2ND_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_2ND_DISABLE;	
			break;
		case 2:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_3RD_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_3RD_DISABLE;		
			break;
		case 3:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_4TH_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_4TH_DISABLE;
			break;			
		case 4:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_5TH_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_5TH_DISABLE;
			break;			
		case 5:
			if(statu == ENABLE)
				read_reg = read_reg|BIT_6TH_ENABLE;	
			else if(statu == DISABLE)
				read_reg = read_reg&BIT_6TH_DISABLE;
			break;			
		default:break;
	}
	
	if(reg == TCA9535_REG_OUT_0)
		g_s_TCA9535_PWRLevel.Port0_Output = read_reg;
	else 	if(reg == TCA9535_REG_OUT_1)
		g_s_TCA9535_PWRLevel.Port1_Output = read_reg;

	g_s_I2C1Structure.ErrorMessage = TCA9535SetOutput(g_s_I2C1Structure,&g_s_TCA9535_PWRLevel);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_PWR_ERR|err_i2c;
	
	return err_i2c;
}

//***************************************************
//函数名  :  EnableDacTCA9535
//功能说明： 设置pwr输出状态
//输入说明： bit 0~15 		statu ENABLE/DISABLE 
//***************************************************
uint8_t EnableDacTCA9535(uint8_t bit,uint8_t statu)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	uint16_t reg_data = 0x0001;
	
	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;

	if(statu == ENABLE)
	{
		if(bit<=15)
		{
			reg_data = ~(reg_data<<bit);
			g_s_TCA9535_DACCtrl.Port0_Output = (uint8_t)reg_data;	
			g_s_TCA9535_DACCtrl.Port1_Output = (uint8_t)(reg_data>>8);
		}
	}
	else if(statu == DISABLE)
	{
		g_s_TCA9535_DACCtrl.Port0_Output = 0xFF;	
		g_s_TCA9535_DACCtrl.Port1_Output = 0xFF;	
	}

	g_s_I2C1Structure.ErrorMessage = TCA9535SetOutput(g_s_I2C1Structure,&g_s_TCA9535_DACCtrl);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_DAC_ERR|err_i2c;
	
	return err_i2c;
}


//***************************************************
//函数名  :  DriverOutTCA9535
//功能说明： 设置Driver板上控制输出   TCA9535控制输出
//输入说明： bit 0~15 		statu ENABLE/DISABLE 
//***************************************************
uint8_t DriverOutTCA9535(uint8_t i2c_chn,uint16_t reg_data)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	
	switch(i2c_chn)
	{
		case 1:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_1;break;
		case 2:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_2;break;
		case 3:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_3;break;
		case 4:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_4;break;
		case 5:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_5;break;
		case 6:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_6;break;
		case 7:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_7;break;
		case 8:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;break;
		default:break;
	}

	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;

	g_s_TCA9535_DutSwitch.Port0_Output = (uint8_t)reg_data;	
	g_s_TCA9535_DutSwitch.Port1_Output = (uint8_t)(reg_data>>8);

	g_s_I2C1Structure.ErrorMessage = TCA9535SetOutput(g_s_I2C1Structure,&g_s_TCA9535_DutSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9535_DRI_ERR|err_i2c;
	
	return err_i2c;
}


//***************************************************
//函数名  :  ReadTMP112Temp
//功能说明： 读取TMP112温度
//输入说明： bit 0~15 		statu ENABLE/DISABLE 
//***************************************************
uint8_t ReadTMP112Temp(uint8_t i2c_chn,double *temp_1st,double *temp_2nd)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	g_s_TMP112_1stTemper.TempValue = 0;
	g_s_TMP112_2ndTemper.TempValue = 0;
	
	switch(i2c_chn)
	{
		case 1:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_1;break;
		case 2:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_2;break;
		case 3:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_3;break;
		case 4:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_4;break;
		case 5:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_5;break;
		case 6:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_6;break;
		case 7:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_7;break;
		case 8:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;break;
		default:break;
	}

	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;

	g_s_I2C1Structure.ErrorMessage = Tmp112ReadTemperature(g_s_I2C1Structure,&g_s_TMP112_1stTemper);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TMP112_1ST_ERR|err_i2c;
	
	*temp_1st = g_s_TMP112_1stTemper.TempValue;
	
	g_s_I2C1Structure.ErrorMessage = Tmp112ReadTemperature(g_s_I2C1Structure,&g_s_TMP112_2ndTemper);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TMP112_2ND_ERR|err_i2c;
	
	*temp_2nd = g_s_TMP112_2ndTemper.TempValue;
	
	return err_i2c;
}


//***************************************************
//函数名  :  ReadSampDUT
//功能说明：采样DUT
//输入说明： void
//***************************************************
uint8_t ReadSampDUT(uint8_t samp,uint16_t delay,void(*f_Time)(uint16_t),int16_t *curr,int16_t *vol)
{
	uint8_t err = I2C_NO_ERR;

	uint8_t i = 0;
	int32_t vol_sum = 0;
	int32_t cur_sum = 0;

	int16_t vol_arr[100] = {0};
	int16_t cur_arr[100] = {0};

	
	int16_t vol_min = 0;
	int16_t vol_max = 0;
	
	int16_t cur_min = 0;
	int16_t cur_max = 0;

	for(i=0;i<samp;i++)
	{
		err = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_DUTVoltage);
		if(I2C_NO_ERR == err)
			vol_arr[i] = (int16_t)g_s_ADS1115_DUTVoltage.RegData;

		err = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_DUTCurrent);
		if(I2C_NO_ERR == err)
			cur_arr[i] = (int16_t)g_s_ADS1115_DUTCurrent.RegData;
		
		if(f_Time != NULL)
			f_Time(delay);
	}
	
	vol_min = vol_arr[0];
	vol_max = vol_arr[0];
	
	cur_min = cur_arr[0];
	cur_max = cur_arr[0];
			
	for (i=0;i<samp;i++)
	{
		vol_sum += vol_arr[i];
		cur_sum += cur_arr[i];

		vol_max = vol_max<vol_arr[i]?vol_arr[i]:vol_max;
		vol_min = vol_min>vol_arr[i]?vol_arr[i]:vol_min;
		
		cur_max = cur_max<cur_arr[i]?cur_arr[i]:cur_max;
		cur_min = cur_min>cur_arr[i]?cur_arr[i]:cur_min;
	}
	
	if(err == I2C_NO_ERR)
	{
		*vol = (vol_sum - vol_max - vol_min)/(samp-2);
		*curr = (cur_sum - cur_max - cur_min)/(samp-2);
	}
	
	return err;
}


//***************************************************
//函数名  :  ReadSampDUT
//功能说明：采样DUT
//输入说明： void
//***************************************************
uint8_t ReadPDandMPD(uint8_t samp,uint16_t delay,void(*f_Time)(uint16_t),int16_t *pd,int16_t *mpd)
{
	uint8_t err = I2C_NO_ERR;

	uint8_t i = 0;
	int32_t pd_sum = 0;
	int32_t mpd_sum = 0;

	int16_t pd_arr[100] = {0};
	int16_t mpd_arr[100] = {0};

	int16_t pd_min = 0;
	int16_t pd_max = 0;
	
	int16_t mpd_min = 0;
	int16_t mpd_max = 0;

	for(i=0;i<samp;i++)
	{
		err = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_PDValue);
		if(I2C_NO_ERR == err)
			pd_arr[i] = (int16_t)g_s_ADS1115_PDValue.RegData;

		err = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_MpdThValue);
		if(I2C_NO_ERR == err)
			mpd_arr[i] = (int16_t)g_s_ADS1115_MpdThValue.RegData;
		
		if(f_Time != NULL)
			f_Time(delay);
	}
	
	pd_min = pd_arr[0];
	pd_max = pd_arr[0];
	
	mpd_min = mpd_arr[0];
	mpd_max = mpd_arr[0];
			
	for (i=0;i<samp;i++)
	{
		pd_sum += pd_arr[i];
		mpd_sum += mpd_arr[i];

		pd_max = pd_max<pd_arr[i]?pd_arr[i]:pd_max;
		pd_min = pd_min>pd_arr[i]?pd_arr[i]:pd_min;
		
		mpd_max = mpd_max<mpd_arr[i]?mpd_arr[i]:mpd_max;
		mpd_min = mpd_min>mpd_arr[i]?mpd_arr[i]:mpd_min;
	}
	
	if(err == I2C_NO_ERR)
	{
		*pd = (pd_sum - pd_max - pd_min)/(samp-2);
		*mpd = (mpd_sum - mpd_max - mpd_min)/(samp-2);
	}
	
	return err;
}

//***************************************************
//函数名  :  ReadPDScanPIV
//功能说明： 采样电压端
//输入说明： void
//***************************************************
uint8_t ReadScanPIV(int16_t *vol,int16_t *curr,int16_t *pd)
{
	uint8_t err = I2C_NO_ERR;

	err = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_DUTVoltage);
	if(I2C_NO_ERR == err)
		*vol = (int16_t)g_s_ADS1115_DUTVoltage.RegData;

	err = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_DUTCurrent);
	if(I2C_NO_ERR == err)
		*curr = (int16_t)g_s_ADS1115_DUTCurrent.RegData;
	
	err = ADS1115ReadReg(g_s_I2C3Structure,ADS1115_REGADD_CON,&g_s_ADS1115_PDValue);
	if(I2C_NO_ERR == err)
		*pd = (int16_t)g_s_ADS1115_PDValue.RegData;
	
	return err;
}


//***************************************************
//函数名  :  WriteBoardInfor
//功能说明： 写入老化板信息
//输入说明： num 1~64
//***************************************************
uint8_t WriteE2promInfor(uint8_t i2c_chn, uint16_t addr, uint8_t len, uint8_t *pBuf)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	
	switch(i2c_chn)
	{
		case 1:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_1;break;
		case 2:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_2;break;
		case 3:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_3;break;
		case 4:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_4;break;
		case 5:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_5;break;
		case 6:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_6;break;
		case 7:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_7;break;
		case 8:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;break;
		default:break;
	}

	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;

	g_s_I2C1Structure.ErrorMessage = DriverAT24CM01Write(g_s_I2C1Structure,0xa0,addr,len,pBuf);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_AT24CM01_ERR|err_i2c;

	return err_i2c;
}

//***************************************************
//函数名  :  ReadBoardInfor
//功能说明： 读取老化板信息
//输入说明： num 1~64
//***************************************************
uint8_t ReadE2promInfor(uint8_t i2c_chn, uint16_t addr, uint8_t len, uint8_t *pBuf)
{
	uint8_t err_i2c = I2C_CHIP_ERR_NONE;
	
	switch(i2c_chn)
	{
		case 1:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_1;break;
		case 2:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_2;break;
		case 3:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_3;break;
		case 4:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_4;break;
		case 5:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_5;break;
		case 6:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_6;break;
		case 7:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_7;break;
		case 8:
			g_s_TCA9548_I2CSwitch.RegConfig = TCA9548_CHN_8;break;
		default:break;
	}

	g_s_TCA9548_I2CSwitch.Address = 0xEE;
	g_s_I2C1Structure.ErrorMessage = DriverTCA9548Config(g_s_I2C1Structure,&g_s_TCA9548_I2CSwitch);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_TCA9548_EXP_ERR|err_i2c;
	
	g_s_I2C1Structure.ErrorMessage = DriverAT24CM01Read(g_s_I2C1Structure,0xa0,addr,len,pBuf);
	if(g_s_I2C1Structure.ErrorMessage != I2C_NO_ERR)
		err_i2c = I2C_AT24CM01_ERR|err_i2c;

	return err_i2c;
}

//***************************************************
//函数名  :  ConfigSGM5348Value
//功能说明： 设置输出5348片内输出 0~11 dut输出 12 pwr 
//输入说明： 
//***************************************************
void ConfigSGM5348Value(uint8_t chip,uint8_t chl,uint16_t dac_bit)
{
	if(chip<15)
	{
		MpdthTCA9535Bit(8,ENABLE);
		
		EnableDacTCA9535(chip,ENABLE);

	}
	else if(chip==15)
	{
		EnableDacTCA9535(chip,DISABLE);
		
		MpdthTCA9535Bit(8,DISABLE);
	}

	DriverSGM5348SetDac(chl,dac_bit);
	
	EnableDacTCA9535(chip,DISABLE);
	
	MpdthTCA9535Bit(8,ENABLE);
}

//***************************************************
//函数名  :  ConfigSGM5348Value
//功能说明： 设置输出5348片内输出 0~11 dut输出 12 pwr 
//输入说明： 
//***************************************************
void ConfigSGM5348Reg(uint8_t chip,void (*f_SetRegValue)(uint16_t),uint16_t reg_val)
{
	if(chip<15)
	{
		MpdthTCA9535Bit(8,ENABLE);
		
		EnableDacTCA9535(chip,ENABLE);

	}
	else if(chip==15)
	{
		EnableDacTCA9535(chip,DISABLE);
		
		MpdthTCA9535Bit(8,DISABLE);
	}

	if(f_SetRegValue!=NULL)
		f_SetRegValue(reg_val);
	
	EnableDacTCA9535(chip,DISABLE);
	
	MpdthTCA9535Bit(8,ENABLE);
}


//***************************************************
//函数名  :  ConfigPDVoltage
//功能说明： 设置电压
//输入说明：
//***************************************************
void ConfigSGM5348Init(uint8_t chip)
{
	ConfigSGM5348Reg(chip,DriverSPIRegWrite,0x9000);   //WTM model 写入寄存器改变输出
	ConfigSGM5348Reg(chip,DriverSPIRegWrite,0xE0FF);   //输出 100k对地 outputs
	
	//默认状态WRM,将通道寄存器数据清除
	ConfigSGM5348Reg(chip,DriverSPIRegWrite,0x0000);		//WRM model 写入寄存器清零输出
	ConfigSGM5348Reg(chip,DriverSPIRegWrite,0x1000);		//WRM model 写入寄存器清零输出
	ConfigSGM5348Reg(chip,DriverSPIRegWrite,0x2000);		//WRM model 写入寄存器清零输出
	ConfigSGM5348Reg(chip,DriverSPIRegWrite,0x3000);		//WRM model 写入寄存器清零输出
	ConfigSGM5348Reg(chip,DriverSPIRegWrite,0x4000);		//WRM model 写入寄存器清零输出
	ConfigSGM5348Reg(chip,DriverSPIRegWrite,0x5000);		//WRM model 写入寄存器清零输出
	ConfigSGM5348Reg(chip,DriverSPIRegWrite,0x6000);		//WRM model 写入寄存器清零输出
	ConfigSGM5348Reg(chip,DriverSPIRegWrite,0x7000);		//WRM model 写入寄存器清零输出
}


//***************************************************
//函数名  :  DriverGPIO_Init
//功能说明： 初始化MCU_GPIO
//输入说明：
//***************************************************
void DriverGPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOA_CLK_ENABLE();	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
		
	//LED初始化
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Pin = LED_RUN_PIN;
	HAL_GPIO_Init(LED_RUN_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LED_LINK_PIN;
	HAL_GPIO_Init(LED_LINK_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LED_ERR_PIN;
	HAL_GPIO_Init(LED_ERR_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LED_1ST_PIN;
	HAL_GPIO_Init(LED_1ST_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LED_2ND_PIN;
	HAL_GPIO_Init(LED_2ND_PORT, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LED_3RD_PIN;
	HAL_GPIO_Init(LED_3RD_PORT, &GPIO_InitStruct);
}


//***************************************************
//函数名  :  ReadMuxADS1115
//功能说明： 采样电压端
//输入说明： void
//***************************************************
//uint8_t ConfigMuxADS1115Ch(uint8_t ch)
//{
//	uint8_t err = I2C_NO_ERR;

//	switch(ch)
//	{
//		case 0:
//			g_s_ADS1115_SampADMux.RegData = 0x40e3;					//配置寄存器参数
//			err = ADS1115WriteReg(g_s_I2C1Structure,ADS1115_REGADD_CFG,g_s_ADS1115_SampADMux);
//			break;
//		case 1:
//			g_s_ADS1115_SampADMux.RegData = 0x50e3;					//配置寄存器参数
//			err = ADS1115WriteReg(g_s_I2C1Structure,ADS1115_REGADD_CFG,g_s_ADS1115_SampADMux);
//			break;
//	}
//	
//	return err;
//}


////***************************************************
////函数名  :  ErrLedControlEntry
////功能说明： 错误灯控制
////					
////输入说明： void
////返回值  ：
////作者/时间/版本号:  liuwei/20150704
////***************************************************
//void ErrLedControlEntry(void *p_arg)
//{	
//	(void)p_arg;
//	
//	for(;;)
//	{
//		if(g_s_SystemInfor.SystemErrType == SYSTEMS_NONE_ERROR)
//		{
//			RESET_LED_ERR
//			vTaskDelay(pdMS_TO_TICKS(500));
//		}
//		else if(g_s_SystemInfor.SystemErrType == SYSTEMS_DRIVER_ERROR)
//		{
//			SET_LED_ERR
//			vTaskDelay(pdMS_TO_TICKS(100));
//			RESET_LED_ERR
//			vTaskDelay(pdMS_TO_TICKS(400));
//		}
//	}
//}

//***************************************************
//函数名  :  RunLedControlEntry
//功能说明： 功能设置初始化
//					创建任务  初始化Dac Adc
//输入说明： void
//返回值  ：
//***************************************************
void RunLedControlEntry(void *p_arg)
{
	(void)p_arg;
	
	for(;;)
	{
		SET_LED_RUN
		vTaskDelay(pdMS_TO_TICKS(1000));
		RESET_LED_RUN
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

//***************************************************
//函数名  :  LinkLedControlEntry
//功能说明： link灯控制
//输入说明： void
//返回值  ：
//作者/时间/版本号:  liuwei/20150704
//***************************************************
void LinkLedControlEntry(void *p_arg)
{
	SemaphoreHandle_t xSemaphore;
	
	xSemaphore = (SemaphoreHandle_t)p_arg;

	for(;;)
	{
		if(pdFALSE != xSemaphoreTake(xSemaphore,portMAX_DELAY))
		{
			SET_LED_LINK
			vTaskDelay(pdMS_TO_TICKS(5));
			RESET_LED_LINK
		}
	}
}

//***************************************************
//函数名  :  RunLedControlEntry
//功能说明： 功能设置初始化
//					创建任务  初始化Dac Adc
//输入说明： void
//返回值  ：
//***************************************************
void FirstLedControlEntry(void *p_arg)
{
	SemaphoreHandle_t xSemaphore;
	
	xSemaphore = (SemaphoreHandle_t)p_arg;
	
	for(;;)
	{
		if(pdFALSE == xSemaphoreTake(xSemaphore,pdMS_TO_TICKS(20)))
		{
			SET_LED_1ST
			vTaskDelay(pdMS_TO_TICKS(1000));
			RESET_LED_1ST
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
		else
		{
			if(eTaskGetState(FirstLedHandle) != eSuspended)
				vTaskSuspend(FirstLedHandle);
		}
	}
}

//***************************************************
//函数名  :  RunLedControlEntry
//功能说明： 功能设置初始化
//					创建任务  初始化Dac Adc
//输入说明： void
//返回值  ：
//***************************************************
void SecondLedControlEntry(void *p_arg)
{
	SemaphoreHandle_t xSemaphore;
	
	xSemaphore = (SemaphoreHandle_t)p_arg;
	
	for(;;)
	{
		if(pdFALSE == xSemaphoreTake(xSemaphore,pdMS_TO_TICKS(20)))
		{
			SET_LED_2ND
			vTaskDelay(pdMS_TO_TICKS(1000));
			RESET_LED_2ND
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
		else
		{
			if(eTaskGetState(SecondLedHandle) != eSuspended)
				vTaskSuspend(SecondLedHandle);
		}
	}
}

//***************************************************
//函数名  :  RunLedControlEntry
//功能说明： 功能设置初始化
//					创建任务  初始化Dac Adc
//输入说明： void
//返回值  ：
//***************************************************
void ThirdLedControlEntry(void *p_arg)
{
	SemaphoreHandle_t xSemaphore;
	
	xSemaphore = (SemaphoreHandle_t)p_arg;
	
	for(;;)
	{
		if(pdFALSE == xSemaphoreTake(xSemaphore,pdMS_TO_TICKS(20)))
		{
			SET_LED_3RD
			vTaskDelay(pdMS_TO_TICKS(1000));
			RESET_LED_3RD
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
		else
		{
			if(eTaskGetState(ThirdLedHandle) != eSuspended)
				vTaskSuspend(ThirdLedHandle);
		}
	}
}


//***************************************************
//函数名  :  ControlParamInit
//功能说明： 功能设置初始化
//					创建任务  初始化Dac Adc
//输入说明： void
//返回值  ：
//***************************************************
void HardParamInit(void)
{
	uint8_t i = 0;
	
	g_s_SystemInfor.I2cSlaveStatus = I2cSamplingChipInit();
	
	g_s_SystemInfor.I2cSlaveStatus = I2cMainControlInit();
	
	g_s_SystemInfor.I2cSlaveStatus = I2cDriverCtrlInit();

	//SGM5348设置初始化
	DriverSPIConfig();

	//SGM5348 LD current Init
	for(i=0;i<=15;i++)
		ConfigSGM5348Init(i);

	DriverGPIO_Init();
	
	SystemsDataInit();
	
	DriverBoradInit();

	PortInformationInit();

	AllAPCPidInitial();
}


//***************************************************
//函数名  :  ControlParamInit
//功能说明： 功能设置初始化
//					创建任务  初始化Dac Adc
//输入说明： void
//返回值  ：
//***************************************************
void ControlParamInit(void)
{
	BaseType_t CreatStatus;
	
	LinkLedSemphr = xSemaphoreCreateBinary();
	if(LinkLedSemphr == NULL)
		return ;
	
	FirstLedSemphr = xSemaphoreCreateBinary();
	if(FirstLedSemphr == NULL)
		return ;
	
	SecondLedSemphr = xSemaphoreCreateBinary();
	if(SecondLedSemphr == NULL)
		return ;
	
	ThirdLedSemphr = xSemaphoreCreateBinary();
	if(ThirdLedSemphr == NULL)
		return ;

	//创建任务堆栈
	CreatStatus = xTaskCreate(LinkLedControlEntry,"LinkLedControl",LINK_LED_SIZE,(void*)LinkLedSemphr,LINK_LED_PRIO,NULL);
	if(CreatStatus != pdPASS)
		return ;
	
	CreatStatus = xTaskCreate(RunLedControlEntry,"RunLedControl",RUN_LED_SIZE,NULL,RUN_LED_PRIO,NULL);
	if(CreatStatus != pdPASS)
		return ;
	
	CreatStatus = xTaskCreate(FirstLedControlEntry,"FirstLedControl",FIRST_LED_SIZE,(void*)FirstLedSemphr,FIRST_LED_PRIO,&FirstLedHandle);
	if(CreatStatus != pdPASS)
		return ;
	
	CreatStatus = xTaskCreate(SecondLedControlEntry,"SecondLedControl",SECOND_LED_SIZE,(void*)SecondLedSemphr,SECOND_LED_PRIO,&SecondLedHandle);
	if(CreatStatus != pdPASS)
		return ;
	
	CreatStatus = xTaskCreate(ThirdLedControlEntry,"ThirdLedControl",THIRD_LED_SIZE,(void*)ThirdLedSemphr,THIRD_LED_PRIO,&ThirdLedHandle);
	if(CreatStatus != pdPASS)
		return ;
	
	if(eTaskGetState(FirstLedHandle) != eSuspended)
		vTaskSuspend(FirstLedHandle);
	
	if(eTaskGetState(SecondLedHandle) != eSuspended)
		vTaskSuspend(SecondLedHandle);
	
	if(eTaskGetState(ThirdLedHandle) != eSuspended)
		vTaskSuspend(ThirdLedHandle);
}


