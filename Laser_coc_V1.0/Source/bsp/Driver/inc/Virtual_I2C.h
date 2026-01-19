#ifndef _VIRTUAL_I2C_H_
#define _VIRTUAL_I2C_H_

#include <stdint.h>

#define VIRTUAL_I2C_TIME_OUT     				 100

#define VIRTUAL_I2C_ACK_FAULT						 1
#define VIRTUAL_I2C_ACK_SUCCESS          0

void Hal_IIC_WaitUs(uint16_t SetCnt);
void Virtual_I2C_Init(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin);
void Virtual_I2C_PortDeinit(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin);
void Virtual_I2C_Start(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin);
void Virtual_I2C_Stop(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin);
uint8_t Virtual_I2C_Wait_Ack(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin);
void Virtual_I2C_Ack(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin);
void Virtual_I2C_NAck(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin);
void Virtual_I2C_Send_Byte(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin,uint8_t data); 
uint8_t Virtual_I2C_Read_Byte(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin);
uint8_t Virtual_I2C_SlaveWriteByte(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin,uint8_t dev_addr, uint8_t addr, const uint8_t *pdata);
uint8_t Virtual_I2C_SlaveWriteString(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin,uint8_t dev_addr, uint8_t addr, uint8_t *pdata, uint16_t num);
uint8_t Virtual_I2C_SlaveReadString(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin,uint8_t dev_addr, uint8_t addr, uint8_t *pdata, uint16_t num);

#endif

