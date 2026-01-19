#ifndef _AT24C02_
#define _AT24C02_

#include <stdint.h>
#include "Driver_I2C.h"

#define AT24CM01_ADDRESS       0xA0

uint8_t DriverAT24CM01Write(I2C_TypeStruct I2CPara,uint8_t SlaveAddr,uint16_t addr, uint8_t len, uint8_t *pBuf);
uint8_t DriverAT24CM01Read(I2C_TypeStruct I2CPara,uint8_t SlaveAddr,uint16_t addr, uint8_t len, uint8_t *pBuf);

#endif
