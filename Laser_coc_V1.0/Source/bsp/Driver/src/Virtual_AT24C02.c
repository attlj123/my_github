#include "Virtual_I2C.h"
#include "Virtual_AT24C02.h"

//**************************************************
//函数：VirtualI2C_AT24C02Init
//功能模拟I2C AT24C02数据写入
//说明：
//**************************************************	 
void VirtualI2C_AT24C02Init(Virtual_AT24C02_TypeStruct *AT24Para)
{
	Virtual_I2C_Init(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);
}


//**************************************************
//函数：VirtualAT24C02CWrite
//功能模拟I2C AT24C02数据写入
//说明：
//**************************************************	 
uint8_t VirtualAT24C02Write(Virtual_AT24C02_TypeStruct *AT24Para,uint8_t addr, uint8_t len, uint8_t *pBuf)
{
  uint8_t cnt = 0;   //KJ-
	uint8_t reg_add = addr;

  for (cnt = 0; cnt < len; cnt++)
  {                   
		if(Virtual_I2C_SlaveWriteByte(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin,AT24Para->Slave_Address,reg_add,pBuf++) == VIRTUAL_I2C_ACK_FAULT)
		{
				Virtual_I2C_Stop(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);
				return VIRTUAL_I2C_ACK_FAULT;
		}

		reg_add++;
		Hal_IIC_WaitUs(AT24C02_WRITE_DELAY);
  }
	
  return VIRTUAL_I2C_ACK_SUCCESS;
}

//**************************************************
//函数：VirtualAT24C02Read
//功能模拟I2C AT24C02数据写入
//说明：
//**************************************************	 
uint8_t VirtualAT24C02Read(Virtual_AT24C02_TypeStruct *AT24Para,uint8_t addr, uint8_t len, uint8_t *pBuf)
{
  uint8_t cnt = 0;   //KJ-

	Virtual_I2C_Start(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);

	Virtual_I2C_Send_Byte(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin,AT24Para->Slave_Address);
	if(Virtual_I2C_Wait_Ack(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin) == VIRTUAL_I2C_ACK_FAULT)
	{
			Virtual_I2C_Stop(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);
			return VIRTUAL_I2C_ACK_FAULT;
	}
		
	Virtual_I2C_Send_Byte(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin,addr);
	if(Virtual_I2C_Wait_Ack(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin) == VIRTUAL_I2C_ACK_FAULT)
	{
			Virtual_I2C_Stop(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);
			return VIRTUAL_I2C_ACK_FAULT;
	}
	
  //KJ- Write Start_Signal for Read
	Virtual_I2C_Start(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);
 
	Virtual_I2C_Send_Byte(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin,AT24Para->Slave_Address|0x01);
	if(Virtual_I2C_Wait_Ack(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin) == VIRTUAL_I2C_ACK_FAULT)
	{
			Virtual_I2C_Stop(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);
			return VIRTUAL_I2C_ACK_FAULT;
	}

  for (cnt = 0; cnt < len; cnt++)
  {
		*(pBuf+cnt) = Virtual_I2C_Read_Byte(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);
		
    // slave devices require NACK to be sent after reading last byte
    if (cnt == len-1)
			Virtual_I2C_NAck(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);//NA
		else
			Virtual_I2C_Ack(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);//NA
  }

	Virtual_I2C_Stop(AT24Para->I2C_SDA_Port,AT24Para->I2C_SDA_Pin,AT24Para->I2C_SCL_Port,AT24Para->I2C_SCL_Pin);

  return VIRTUAL_I2C_ACK_SUCCESS;
}





