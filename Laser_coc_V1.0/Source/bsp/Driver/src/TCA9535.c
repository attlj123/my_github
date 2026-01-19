#include "TCA9535.h"

//**************************************************
//函数：TCA9535ConfigInit
//功能：TCA9535初始化配置
//说明：
//**************************************************
uint8_t TCA9535ConfigInit(I2C_TypeStruct I2CPara,TCA9535_TypeStruct *TCA9535Para)
{
	uint8_t reg_data[2] = {0x00,0x00};
	
	uint8_t num = 0;
	I2CPara.ErrorMessage = I2C_READY;
	while((I2CPara.ErrorMessage != I2C_NO_ERR))
	{
		//设置PORT0/1 output bit 0
		I2CPara.ErrorMessage = Driver_I2C_MasterWriteToSlave(I2CPara,TCA9535Para->Address,TCA9535ADD_OUTPUT_PORT0,I2C_MEMADD_SIZE_8BIT,reg_data,2);
		num++;
		if(num>=TCA9535_I2CMUL_NUM)
		{
				break;
		}
	}
	
	if(I2CPara.ErrorMessage != I2C_NO_ERR)
		return I2CPara.ErrorMessage;
	
	reg_data[0] = TCA9535Para->Port0_CfgReg;		//配置寄出去  0：输出  1：输入
	reg_data[1] = TCA9535Para->Port1_CfgReg;
	
	num = 0;
	I2CPara.ErrorMessage = I2C_READY;
	while((I2CPara.ErrorMessage != I2C_NO_ERR))
	{
		//设置PORT0/1 config bit out 0
		I2CPara.ErrorMessage = Driver_I2C_MasterWriteToSlave(I2CPara,TCA9535Para->Address,TCA9535ADD_CONFIG_PORT0,I2C_MEMADD_SIZE_8BIT,reg_data,2);
		num++;
		if(num>=TCA9535_I2CMUL_NUM)
		{
				break;
		}
	}
		
	return I2CPara.ErrorMessage;
}

//**************************************************
//函数：TCA9535SetOutput
//功能：TCA9535配置IO输出状态
//说明：
//**************************************************
uint8_t TCA9535SetOutput(I2C_TypeStruct I2CPara,TCA9535_TypeStruct *TCA9535Para)
{
	uint8_t reg_data[2] = {0x00,0x00};
	uint8_t num = 0;
	
	reg_data[0] = TCA9535Para->Port0_Output;
	reg_data[1] = TCA9535Para->Port1_Output;
	
	I2CPara.ErrorMessage = I2C_READY;
	while((I2CPara.ErrorMessage != I2C_NO_ERR))
	{
		//设置PORT0/1 config bit out 0
		I2CPara.ErrorMessage = Driver_I2C_MasterWriteToSlave(I2CPara,TCA9535Para->Address,TCA9535ADD_OUTPUT_PORT0,I2C_MEMADD_SIZE_8BIT,reg_data,2);
		num++;
		if(num>=TCA9535_I2CMUL_NUM)
		{
				break;
		}
	}

	return I2CPara.ErrorMessage;
}

//**************************************************
//函数：TCA9535ReadBit
//功能：TCA9535读取设置状态
//说明：
//**************************************************
uint8_t TCA9535ReadBit(I2C_TypeStruct I2CPara,TCA9535_TypeStruct *TCA9535Para)
{
	uint8_t num = 0;
	uint8_t reg_data[2] = {0x00,0x00};

	I2CPara.ErrorMessage = I2C_READY;
	while((I2CPara.ErrorMessage != I2C_NO_ERR))
	{
		//设置PORT0/1 config bit out 0
		I2CPara.ErrorMessage = Driver_I2C_MasterWriteReadFromSlave(I2CPara,TCA9535Para->Address,TCA9535ADD_INPUT_PORT0,I2C_MEMADD_SIZE_8BIT,reg_data,2);
		num++;
		if(num>=TCA9535_I2CMUL_NUM)
		{
				break;
		}
	}
	
	TCA9535Para->Port0_Input = reg_data[0];
	TCA9535Para->Port1_Input = reg_data[1];

	return I2CPara.ErrorMessage;
}

