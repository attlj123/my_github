#include "AT24CM01.h"
#include "Driver_I2C.h"

//**************************************************
//函数：DriverAT24CM01Write
//功能AT24CM01数据写入
//说明：
//**************************************************
uint8_t DriverAT24CM01Write(I2C_TypeStruct I2CPara,uint8_t SlaveAddr,uint16_t addr, uint8_t len, uint8_t *pBuf)
{
	I2CPara.ErrorMessage = Driver_I2C_MasterWriteToSlave(I2CPara,SlaveAddr,addr,I2C_MEMADD_SIZE_16BIT,pBuf,len);

	return I2CPara.ErrorMessage;
}

//**************************************************
//函数：DriverAT24CM01Read
//功能：读取AT24CM01数据
//说明：
//**************************************************
uint8_t DriverAT24CM01Read(I2C_TypeStruct I2CPara,uint8_t SlaveAddr,uint16_t addr, uint8_t len, uint8_t *pBuf)				
{					
	I2CPara.ErrorMessage = Driver_I2C_MasterWriteReadFromSlave(I2CPara,SlaveAddr,addr,I2C_MEMADD_SIZE_16BIT,pBuf,len);
  
	return I2CPara.ErrorMessage;

}	





