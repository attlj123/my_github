#ifndef _TCA9535_H_
#define _TCA9535_H_

#include 	<stdint.h>
#include 	"Driver_I2C.h"

//配置地址寄存器
#define 	TCA9535ADD_INPUT_PORT0  				0x00			//输入PORT0地址
#define 	TCA9535ADD_INPUT_PORT1 					0x01			//输入PORT1地址
#define 	TCA9535ADD_OUTPUT_PORT0  				0x02			//输出PORT0地址
#define 	TCA9535ADD_OUTPUT_PORT1 				0x03      		//输出PORT1地址
#define 	TCA9535ADD_POLAR_PORT0  				0x04			//反转PORT0地址
#define 	TCA9535ADD_POLAR_PORT1 					0x05      		//反转PORT1地址
#define 	TCA9535ADD_CONFIG_PORT0 			 	0x06			//配置PORT0地址
#define 	TCA9535ADD_CONFIG_PORT1 			  	0x07			//配置PORT1地址

#define 	TCA9535_I2CMUL_NUM 						0x05
//TCA9535 
typedef struct
{
    uint8_t 	Address;							//硬件地址
	uint8_t	  	Port0_CfgReg;						//PORT0 配置输入或输出状态
	uint8_t	  	Port1_CfgReg;						//PORT1 
	uint8_t	  	Port0_Output;						//PORT0 配置输出基础器
	uint8_t	  	Port1_Output;						//PORT1 
	uint8_t	  	Port0_Input;						//PORT0 配置输入寄出去
	uint8_t	  	Port1_Input;						//PORT1 
}TCA9535_TypeStruct;

uint8_t TCA9535ConfigInit(I2C_TypeStruct I2CPara,TCA9535_TypeStruct *TCA9535Para);
uint8_t TCA9535SetOutput(I2C_TypeStruct I2CPara,TCA9535_TypeStruct *TCA9535Para);
uint8_t TCA9535ReadBit(I2C_TypeStruct I2CPara,TCA9535_TypeStruct *TCA9535Para);

#endif
