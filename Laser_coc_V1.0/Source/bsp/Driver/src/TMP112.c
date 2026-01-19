#include "TMP112.h"

//**************************************************
//函数：TCA9535ConfigInit
//功能：TCA9535初始化配置
//说明：
//**************************************************
uint8_t Tmp112ReadTemperature(I2C_TypeStruct I2CPara,TMP112_TypeStruct *TMP112Para)
{
	uint8_t reg_data[2] = {0x00,0x00};
	double temp_date = 0;
	
	uint8_t num = 0;
	I2CPara.ErrorMessage = I2C_READY;
	while((I2CPara.ErrorMessage != I2C_NO_ERR))
	{
		//设置PORT0/1 output bit 0
		I2CPara.ErrorMessage = Driver_I2C_MasterWriteReadFromSlave(I2CPara,TMP112Para->Address,TMP112_TEMPVALUE_ADDR,I2C_MEMADD_SIZE_8BIT,reg_data,2);
		num++;
		if(num>=TMP112_I2CMUL_NUM)
		{
				break;
		}
	}
	
	if(I2CPara.ErrorMessage != I2C_NO_ERR)
		return I2CPara.ErrorMessage;
	
	temp_date = ((((int16_t)reg_data[0])<<8)|((int16_t)reg_data[1]))>>4;
	
	TMP112Para->TempValue = temp_date*TMP112_DOUBLE_BIT;

	return I2CPara.ErrorMessage;
}



