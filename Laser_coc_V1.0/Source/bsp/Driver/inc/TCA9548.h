#ifndef _TCA9548_H_
#define _TCA9548_H_

#include 	<stdint.h>
#include 	"Driver_I2C.h"

//IICÍ¨µÀÊä³ö
//ÅäÖÃµØÖ·¼Ä´æÆ÷
#define      TCA9548_CHN_NONE							0x00
#define		 TCA9548_CHN_1								0x01
#define		 TCA9548_CHN_2								0x02
#define		 TCA9548_CHN_3								0x04
#define		 TCA9548_CHN_4								0x08
#define		 TCA9548_CHN_5								0x10
#define		 TCA9548_CHN_6								0x20
#define		 TCA9548_CHN_7								0x40
#define		 TCA9548_CHN_8								0x80
#define		 TCA9548_CHN_ALL							0xFF

#define 	TCA9548_I2CMUL_NUM 						0x05

//TCA9535 
typedef struct
{
    uint8_t 	Address;							//Ó²¼þµØÖ·
	uint8_t     RegConfig;							//¼Ä´æÆ÷ÅäÖÃ
}TCA9548_TypeStruct;	

uint8_t DriverTCA9548Config(I2C_TypeStruct I2CPara,TCA9548_TypeStruct *TCA9548Para);	

#endif
