#include "TCA9548.h"

#define 	DELAY_US_CNT 					480000000/2000000

void DriverTCA9548Delay(uint16_t SetCnt)
{
	uint16_t i = 0;
	uint16_t j = 0;
 
	for(i=0; i<SetCnt; i++)
		for(j=0; j<DELAY_US_CNT; j++);
}

//**************************************************
//函数：DriverPCA9548Channel
//功能：PCA9548选择通道
//说明：
//**************************************************
uint8_t DriverTCA9548Config(I2C_TypeStruct I2CPara,TCA9548_TypeStruct *TCA9548Para)								
{					
	uint8_t num = 0;
	uint8_t reg_data = 0;
	
	reg_data = TCA9548Para->RegConfig;
	
	I2CPara.ErrorMessage = I2C_READY;
	while((I2CPara.ErrorMessage != I2C_NO_ERR))
	{
//		DriverTCA9548Delay(1000);
		
		//设置PORT0/1 config bit out 0
		I2CPara.ErrorMessage = Driver_I2C_MasterWriteData(I2CPara,TCA9548Para->Address,&reg_data,1);

//		DriverTCA9548Delay(1000);
		
		num++;
		if(num>=TCA9548_I2CMUL_NUM)
		{
				break;
		}
	}
		
	return I2CPara.ErrorMessage;
}	



