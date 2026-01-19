#ifndef __DRIVER_I2C_H__
#define __DRIVER_I2C_H__

#include <stdint.h>
#include "stm32h7xx_hal.h"

//************************************************//
//        I2C REPORT MAP Table                    //
//                NO_REMP       PART_REMP         //
//     I2C0       PB6,PB7       PB8,PB9           //
//     I2C1       PB10,PB11                       //
// ***********************************************//
//I2C端口重映射方式
#define NO_REMP    0x00
#define PART_REMP  0x01

//配置I2C的引脚映射（需要用户配置）
#define I2C0_REMP  PART_REMP
#define I2C1_REMP  NO_REMP

//I2C1 IO口的配置
#if     I2C0_REMP==NO_REMP
#define I2C0_SCL_BANK           RCU_GPIOB
#define I2C0_SCL_PORT           GPIOB
#define I2C0_SCL_PIN            GPIO_PIN_6

#define I2C0_SDA_BANK           RCU_GPIOB
#define I2C0_SDA_PORT           GPIOB
#define I2C0_SDA_PIN            GPIO_PIN_7
#endif

#if     I2C0_REMP==PART_REMP
#define I2C0_SCL_BANK           RCU_GPIOB
#define I2C0_SCL_PORT           GPIOB
#define I2C0_SCL_PIN            GPIO_PIN_8

#define I2C0_SDA_BANK           RCU_GPIOB
#define I2C0_SDA_PORT           GPIOB
#define I2C0_SDA_PIN            GPIO_PIN_9
#endif

//I2C2 IO口的配置
#define I2C1_SCL_BANK           RCU_GPIOB
#define I2C1_SCL_PORT           GPIOB
#define I2C1_SCL_PIN            GPIO_PIN_10

#define I2C1_SDA_BANK           RCU_GPIOB
#define I2C1_SDA_PORT           GPIOB
#define I2C1_SDA_PIN            GPIO_PIN_11

//定义错误信息
#define I2C_NO_ERR                 0 //无错误
#define I2C_READY                  1 //无错误
#define I2C_ERR_START_TIMEOUT      2 //产生起始条件超时
#define I2C_ERR_ADDRESS_TIMEOUT    3 //主机发送地址超时
#define I2C_ERR_TXCHAR_TIMEOUT     4 //主机发送数据超时
#define I2C_ERR_RXCHAR_TIMEOUT     5 //主机接收数据超时
#define I2C_ERR_RXLENGTH_OVERLOARD 6 //接收数据超长
#define I2C_ERR_INIT      				 7 //I2C初始化异常

//定义主机应答信号
#define I2C_ACK_EN   0
#define I2C_ACK_DIS  1

typedef struct
{
    I2C_TypeDef *Name;
    uint8_t ErrorMessage;
	uint8_t Address;
	uint32_t ClockValue;
}I2C_TypeStruct;

//===========================================================
//功能:主机产生起始条件START
//输入:无
//输出:错误信息
//===========================================================
uint8_t Driver_I2C_MasterStart(I2C_TypeStruct I2CPara);
//===========================================================
//功能:主机发送从机地址，确定数据传输方向
//输入:SlaveAdd 从机地址
//     I2C_Direction 主机传输方向
//输出:错误信息
//===========================================================
uint8_t Driver_I2C_MasterSendAddr(I2C_TypeStruct I2CPara,uint8_t SlaveAddr,uint8_t I2C_Direction);
//******************************************************
//功能:主机向从机发送一个字节
//输入:data 待发送数据
//输出:错误信息
//******************************************************
uint8_t Driver_I2C_MasterSendOneChar(I2C_TypeStruct I2CPara,uint8_t Data);
//===========================================================
//功能:主机呼叫从机,准备通讯
//输入:SlaveAdd 从机地址
//     I2C_Direction 数据传输方向,主机发送=I2C_Direction_Transmitter
//                                主机接收=I2C_Direction_Receiver
//输出:错误信息
//===========================================================
uint8_t Driver_I2C_MasterCallSlave(I2C_TypeStruct I2CPara,uint16_t SlaveAddr,uint32_t I2C_Direction);
//******************************************************
//功能:主机向从机发送一个字节
//输入:data 待发送数据
//输出:错误信息
//******************************************************
uint8_t Driver_I2C_MasterWriteOneChar(I2C_TypeStruct I2CPara,uint8_t Data);
//===========================================================
//功能:主机向从机接收一个字节
//输入:*data 存储接收的数据
//输出:错误信息
//===========================================================
uint8_t Driver_I2C_MasterReadOneChar(I2C_TypeStruct I2CPara,uint8_t *Data);
//===========================================================
//功能:主机产生应答信号
//输入:Status 应答信号 取值 I2C_ACK_EN , I2C_ACK_DIS
//输出:无
//===========================================================
void Driver_I2C_MasterAckEnOrDis(I2C_TypeStruct I2CPara,uint8_t Status);
//===========================================================
//功能:主机产生停止条件
//输入:无
//输出:无
//===========================================================
void Driver_I2C_MasterStop(I2C_TypeStruct I2CPara);

//==========================================================
//功能:I2C初始化
//输入:I2Cx 取值I2C1, I2C2
//     OwnAddr 自己的地址，取值0~63
//     ClockSpeed I2C时钟 取值<400 000 400KHz
//输出:无
//==========================================================
void Driver_I2C_Init(I2C_TypeStruct I2CPara);

//===========================================================
//功能:主机向从机发送数据
//输入:SlaveAdd 从机地址
//     data 待发送数据
//输出:错误信息
//===========================================================
uint8_t Driver_I2C_MasterWriteData(I2C_TypeStruct I2CPara,uint8_t SlaveAddr,uint8_t *Data,uint16_t ReadLength);


//===========================================================
//功能:主机向从机发送数据
//输入:SlaveAdd 从机地址
//     Command 发送第一个数据,模块EEPROM地址
//     data 待发送数据
//输出:错误信息
//===========================================================
uint8_t Driver_I2C_MasterWriteToSlave(I2C_TypeStruct I2CPara,uint16_t SlaveAddr,uint16_t MemAddr,uint16_t MemLenght,uint8_t *Data,uint16_t ReadLength);


//===========================================================
//功能:主机向从机接收数据
//输入:SlaveAdd 从机地址
//     WriteCommand 发送第一个数据,模块EEPROM地址
//     *ReadBuffer 存储接收的数据
//     ReadLength 待接收数据长度
//输出:错误信息
//===========================================================
uint8_t Driver_I2C_MasterWriteReadFromSlave(I2C_TypeStruct I2CPara,uint16_t SlaveAddr,uint16_t MemAddr,uint16_t MemLenght,uint8_t *Data,uint16_t ReadLength);


#endif

