//******************************************************************************
//File Name  :   ADS1115.c
//Description:   ADS1115驱动
//Author/Time/ Version number:
// Modify Record:
//******************************************************************************
#include "ADS1115.h"

//**************************************************
//函数：ADS1115WriteReg
//功能：ADS1115参数写入
//说明：
//**************************************************
uint8_t ADS1115WriteReg(I2C_TypeStruct I2CPara,uint8_t reg,ADS1115_TypeStruct *ADS1115Para)
{
	uint8_t reg_data[2] = {0x00,0x00};
	uint8_t num = 0;
	
	reg_data[1] = (uint8_t)ADS1115Para->RegData;
	reg_data[0] = (uint8_t)(ADS1115Para->RegData>>8);

	I2CPara.ErrorMessage = I2C_READY;
	while((I2CPara.ErrorMessage != I2C_NO_ERR))
	{
		I2CPara.ErrorMessage = Driver_I2C_MasterWriteToSlave(I2CPara,ADS1115Para->Address,reg,I2C_MEMADD_SIZE_8BIT,reg_data,2);
		num++;
		if(num>=ADS1115_I2CMUL_NUM)
		{
				break;
		}
	}
		
	return I2CPara.ErrorMessage;
}

//**************************************************
//函数：ADS1115ReadReg
//功能：ADS1115参数写入
//说明：
//**************************************************
uint8_t ADS1115ReadReg(I2C_TypeStruct I2CPara,uint8_t reg,ADS1115_TypeStruct *ADS1115Para)
{
	uint8_t reg_data[2] = {0x00,0x00};
	uint8_t num = 0;
	
	I2CPara.ErrorMessage = I2C_READY;
	while((I2CPara.ErrorMessage != I2C_NO_ERR))
	{
		I2CPara.ErrorMessage = Driver_I2C_MasterWriteReadFromSlave(I2CPara,ADS1115Para->Address,reg,I2C_MEMADD_SIZE_8BIT,reg_data,2);	
		num++;
		if(num>=ADS1115_I2CMUL_NUM)
		{
				break;
		}
	}

	if(I2CPara.ErrorMessage == I2C_NO_ERR)
		ADS1115Para->RegData = ((uint16_t)reg_data[0]<<8)|(uint16_t)reg_data[1];
	
	return I2CPara.ErrorMessage;
}


