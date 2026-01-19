#ifndef _TMP112_H_
#define _TMP112_H_

#include 	<stdint.h>
#include 	"Driver_I2C.h"

//TMP112 	寄出去参数值
#define 	TMP112_TEMPVALUE_ADDR		0x00			//输入PORT0地址

//TMP112 	单bit值
#define		TMP112_DOUBLE_BIT			0.0625

//配置地址寄存器
#define 	TMP112_I2CMUL_NUM			0x05

//TMP112 
typedef struct
{
    uint8_t 	Address;							//硬件地址
	double      TempValue;							//测试温度值
}TMP112_TypeStruct;

uint8_t Tmp112ReadTemperature(I2C_TypeStruct I2CPara,TMP112_TypeStruct *TMP112Para);

#endif
