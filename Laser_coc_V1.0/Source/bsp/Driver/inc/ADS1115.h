#ifndef _ADS1115_H_
#define _ADS1115_H_

#include <stdint.h>
#include "Driver_I2C.h"

#define  ADS1115_REGADD_CON					0x00
#define  ADS1115_REGADD_CFG					0x01
#define  ADS1115_REGADD_LOTHR       		0x02
#define  ADS1115_REGADD_HITHR				0x03

#define  ADS1115_I2CMUL_NUM   		  		0x05

typedef struct
{
	uint8_t 	Address;					//硬件地址
	uint16_t	RegData;					//操作寄存器数据
}ADS1115_TypeStruct;

uint8_t ADS1115WriteReg(I2C_TypeStruct I2CPara,uint8_t reg,ADS1115_TypeStruct *ADS1115Para);
uint8_t ADS1115ReadReg(I2C_TypeStruct I2CPara,uint8_t reg,ADS1115_TypeStruct *ADS1115Para);

#endif






