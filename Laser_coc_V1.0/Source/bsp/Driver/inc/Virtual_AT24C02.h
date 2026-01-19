#ifndef _VIRTUAL_AT24C02_
#define _VIRTUAL_AT24C02_

#include <stdint.h>

#define 	AT24C02_WRITE_DELAY 	 500

//TCA9535 
typedef struct
{
    uint8_t 		Slave_Address;							//Ó²¼þµØÖ·
		uint32_t	  I2C_SDA_Port;					//I2C SDA
		uint32_t	  I2C_SDA_Pin;					//I2C SDA
		uint32_t	  I2C_SCL_Port;					//I2C SCL
		uint32_t	  I2C_SCL_Pin;					//I2C SCL
}Virtual_AT24C02_TypeStruct;

void VirtualI2C_AT24C02Init(Virtual_AT24C02_TypeStruct *AT24Para);
uint8_t VirtualAT24C02Write(Virtual_AT24C02_TypeStruct *AT24Para,uint8_t addr, uint8_t len, uint8_t *pBuf);
uint8_t VirtualAT24C02Read(Virtual_AT24C02_TypeStruct *AT24Para,uint8_t addr, uint8_t len, uint8_t *pBuf);

#endif
