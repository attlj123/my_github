#ifndef _ControlParam_H_
#define _ControlParam_H_

#include "FunctionSet.h"

#define 	ERRLED_QUEUE_LENGTH							1
#define 	ERRLED_QUEUE_ITEM_SIZE 					sizeof(uint8_t)

//初始化i2c chip 异常
#define   I2C_CHIP_ERR_NONE				0x00
#define   I2C_ADS1115_VOLT_ERR			0x02
#define   I2C_ADS1115_CURR_ERR			0x04
#define   I2C_ADS1115_MPD_ERR			0x08
#define   I2C_ADS1115_PD_ERR			0x10

//主板I2C控制异常
#define   I2C_TCA9548_EXP_ERR			0x02
#define   I2C_TCA9535_ONL_ERR			0x04
#define   I2C_TCA9535_TYPE_ERR			0x08
#define   I2C_TCA9535_SAMP_ERR			0x10
#define   I2C_TCA9535_PWR_ERR			0x20
#define   I2C_TCA9535_DAC_ERR			0x40
#define   I2C_TCA9535_PD_ERR			0x80

//驱动板子I2C读取状态
#define   I2C_TCA9535_DRI_ERR			0xAA
#define   I2C_TMP112_1ST_ERR			0xBB
#define   I2C_TMP112_2ND_ERR			0xCC
#define   I2C_AT24CM01_ERR				0xDD

//驱动板I2C控制异常
#define   I2C_DUTPUT_1ST_ERR			0x02
#define   I2C_DUTPUT_2ND_ERR			0x04
#define   I2C_DUTPUT_3RD_ERR			0x08
#define   I2C_DUTPUT_4TH_ERR			0x10
#define   I2C_DUTPUT_5TH_ERR			0x20
#define   I2C_DUTPUT_6TH_ERR			0x40

#define   TCA9535_REG_OUT_0             0x00
#define   TCA9535_REG_OUT_1             0x01

//PWR控制使能
#define   BIT_1ST_ENABLE				0x01
#define   BIT_2ND_ENABLE				0x02
#define   BIT_3RD_ENABLE				0x04
#define   BIT_4TH_ENABLE				0x08
#define   BIT_5TH_ENABLE				0x10
#define   BIT_6TH_ENABLE				0x20
#define   BIT_7TH_ENABLE				0x40
#define   BIT_8TH_ENABLE				0x80

#define   BIT_1ST_DISABLE				0xFE
#define   BIT_2ND_DISABLE				0xFD
#define   BIT_3RD_DISABLE				0xFB
#define   BIT_4TH_DISABLE				0xF7
#define   BIT_5TH_DISABLE				0xEF
#define   BIT_6TH_DISABLE				0xDF
#define   BIT_7TH_DISABLE				0xBF
#define   BIT_8TH_DISABLE				0x7F

//LED板上
#define   LED_LINK_PORT    				GPIOB
#define	  LED_LINK_PIN					GPIO_PIN_14
#define   RESET_LED_LINK				HAL_GPIO_WritePin(LED_LINK_PORT,LED_LINK_PIN,GPIO_PIN_RESET);			
#define   SET_LED_LINK					HAL_GPIO_WritePin(LED_LINK_PORT,LED_LINK_PIN,GPIO_PIN_SET);

#define   LED_RUN_PORT    				GPIOE
#define	  LED_RUN_PIN					GPIO_PIN_12
#define   RESET_LED_RUN					HAL_GPIO_WritePin(LED_RUN_PORT,LED_RUN_PIN,GPIO_PIN_SET);			
#define   SET_LED_RUN					HAL_GPIO_WritePin(LED_RUN_PORT,LED_RUN_PIN,GPIO_PIN_RESET);

#define   LED_ERR_PORT    				GPIOE
#define	  LED_ERR_PIN					GPIO_PIN_13
#define   RESET_LED_ERR					HAL_GPIO_WritePin(LED_ERR_PORT,LED_ERR_PIN,GPIO_PIN_RESET);			
#define   SET_LED_ERR					HAL_GPIO_WritePin(LED_ERR_PORT,LED_ERR_PIN,GPIO_PIN_SET);

#define   LED_1ST_PORT    				GPIOE
#define	  LED_1ST_PIN					GPIO_PIN_8
#define   RESET_LED_1ST					HAL_GPIO_WritePin(LED_1ST_PORT,LED_1ST_PIN,GPIO_PIN_RESET);			
#define   SET_LED_1ST					HAL_GPIO_WritePin(LED_1ST_PORT,LED_1ST_PIN,GPIO_PIN_SET);

#define   LED_2ND_PORT    				GPIOE
#define	  LED_2ND_PIN					GPIO_PIN_9
#define   RESET_LED_2ND					HAL_GPIO_WritePin(LED_2ND_PORT,LED_2ND_PIN,GPIO_PIN_RESET);			
#define   SET_LED_2ND					HAL_GPIO_WritePin(LED_2ND_PORT,LED_2ND_PIN,GPIO_PIN_SET);

#define   LED_3RD_PORT    				GPIOE
#define	  LED_3RD_PIN					GPIO_PIN_10
#define   RESET_LED_3RD					HAL_GPIO_WritePin(LED_3RD_PORT,LED_3RD_PIN,GPIO_PIN_RESET);			
#define   SET_LED_3RD					HAL_GPIO_WritePin(LED_3RD_PORT,LED_3RD_PIN,GPIO_PIN_SET);

uint8_t I2cSamplingChipInit(void);
uint8_t I2cMainControlInit(void);
uint8_t I2cDriverCtrlInit(void);
uint8_t GetDriverOnline(uint16_t *online);
uint8_t GetDriverTypes(uint16_t *types);
uint8_t SwitchDUTSamp(uint8_t num);
uint8_t SwitchMPDTH(uint8_t num);
uint8_t MainTCA9535Bit(uint8_t reg,uint8_t bit,uint8_t statu);
uint8_t MpdthTCA9535Bit(uint8_t bit,uint8_t statu);
uint8_t EnableDacTCA9535(uint8_t bit,uint8_t statu);
uint8_t DriverOutTCA9535(uint8_t i2c_chn,uint16_t reg_data);
uint8_t ReadTMP112Temp(uint8_t i2c_chn,double *temp_1st,double *temp_2nd);
uint8_t ReadSampDUT(uint8_t samp,uint16_t delay,void(*f_Time)(uint16_t),int16_t *curr,int16_t *vol);
uint8_t ReadPDandMPD(uint8_t samp,uint16_t delay,void(*f_Time)(uint16_t),int16_t *pd,int16_t *mpd);
uint8_t ReadScanPIV(int16_t *vol,int16_t *curr,int16_t *pd);
uint8_t WriteE2promInfor(uint8_t i2c_chn, uint16_t addr, uint8_t len, uint8_t *pBuf);
uint8_t ReadE2promInfor(uint8_t i2c_chn, uint16_t addr, uint8_t len, uint8_t *pBuf);
void ConfigSGM5348Value(uint8_t chip,uint8_t chl,uint16_t dac_bit);
void ConfigSGM5348Init(uint8_t chip);

uint8_t DetectDriverStatus(void);
void ControlParamInit(void);
void HardParamInit(void);



#endif
