#include <stdint.h>
#include "Virtual_I2C.h"
#include "stm32h7xx_hal_gpio.h"

//**************************************************
//函数：Hal_HW_WaitUs
//功能AT24CM01数据写入
//说明：
//**************************************************
static void Virtual_I2C_SDA_IN(uint32_t SDA_Port, uint32_t SDA_Pin)
{
  gpio_init(SDA_Port,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,SDA_Pin);
}

//**************************************************
//函数：Hal_HW_WaitUs
//功能AT24CM01数据写入
//说明：
//**************************************************
static void Virtual_I2C_SDA_OUT(uint32_t SDA_Port, uint32_t SDA_Pin)
{
  gpio_init(SDA_Port,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,SDA_Pin);
}

//**************************************************
//函数：Hal_HW_WaitUs
//功能I2C时序等待
//说明：
//**************************************************
void Hal_IIC_WaitUs(uint16_t SetCnt)
{
	uint16_t i = 0;
	uint16_t j = 0;
 
	for(i=0; i<SetCnt; i++)
		for(j=0; j<54; j++);
}

//**************************************************
//函数：Virtual_I2C_Init
//功能AT24CM01数据写入
//说明：
//**************************************************
void Virtual_I2C_Init(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin)
{
	gpio_init(SDA_Port,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,SDA_Pin);
	gpio_init(SCL_Port,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,SCL_Pin);
}

//**************************************************
//函数：Virtual_I2C_PortDeinit
//功能AT24CM01数据写入
//说明：
//**************************************************
void Virtual_I2C_PortDeinit(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin)
{
    gpio_bit_reset(SCL_Port,SCL_Pin);
		Virtual_I2C_SDA_OUT(SDA_Port,SDA_Pin);
    gpio_bit_reset(SDA_Port,SDA_Pin);
}

//**************************************************
//函数：Virtual_I2C_Start
//功能AT24CM01数据写入
//说明：
//**************************************************
void Virtual_I2C_Start(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin)
{
		Virtual_I2C_SDA_OUT(SDA_Port,SDA_Pin);
    gpio_bit_set(SDA_Port,SDA_Pin);
    Hal_IIC_WaitUs(5);
    gpio_bit_set(SCL_Port,SCL_Pin);
    Hal_IIC_WaitUs(5);
    gpio_bit_reset(SDA_Port,SDA_Pin);
    Hal_IIC_WaitUs(5);
    gpio_bit_reset(SCL_Port,SCL_Pin);
    Hal_IIC_WaitUs(5);
}

//**************************************************
//函数：Virtual_I2C_Stop
//功能AT24CM01数据写入
//说明：
//**************************************************
void Virtual_I2C_Stop(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin)
{
		Virtual_I2C_SDA_OUT(SDA_Port,SDA_Pin);
    gpio_bit_reset(SDA_Port,SDA_Pin);
    gpio_bit_set(SCL_Port,SCL_Pin);
    Hal_IIC_WaitUs(5);
    gpio_bit_set(SDA_Port,SDA_Pin);
    Hal_IIC_WaitUs(5);
}

//**************************************************
//函数：Virtual_I2C_Wait_Ack
//功能AT24CM01数据写入
//说明：
//**************************************************
uint8_t Virtual_I2C_Wait_Ack(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin)
{
		uint8_t ucErrTime=0;
		
		gpio_bit_set(SDA_Port,SDA_Pin);
		Virtual_I2C_SDA_IN(SDA_Port,SDA_Pin);
		Hal_IIC_WaitUs(5);
		gpio_bit_set(SCL_Port,SCL_Pin);
		Hal_IIC_WaitUs(5);
		while(gpio_input_bit_get(SDA_Port,SDA_Pin))
		{
				ucErrTime++;
				if(ucErrTime>250)
				{
					Virtual_I2C_Stop(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
					return VIRTUAL_I2C_ACK_FAULT;
				}
		}
		
		gpio_bit_reset(SCL_Port,SCL_Pin);
		
		return VIRTUAL_I2C_ACK_SUCCESS;
} 

//**************************************************
//函数：Virtual_I2C_Wait_Ack
//功能AT24CM01数据写入
//说明：
//**************************************************
void Virtual_I2C_Ack(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin)
{
		Virtual_I2C_SDA_OUT(SDA_Port,SDA_Pin);
		gpio_bit_reset(SDA_Port,SDA_Pin);
		Hal_IIC_WaitUs(5);
		gpio_bit_set(SCL_Port,SCL_Pin);
		Hal_IIC_WaitUs(5);
		gpio_bit_reset(SCL_Port,SCL_Pin);
}

//**************************************************
//函数：Virtual_I2C_Wait_Ack
//功能AT24CM01数据写入
//说明：
//**************************************************		    
void Virtual_I2C_NAck(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin)
{
		Virtual_I2C_SDA_OUT(SDA_Port,SDA_Pin);
		gpio_bit_set(SDA_Port,SDA_Pin);
		Hal_IIC_WaitUs(5);
		gpio_bit_set(SCL_Port,SCL_Pin);
		Hal_IIC_WaitUs(5);
		gpio_bit_reset(SCL_Port,SCL_Pin);
}		

//**************************************************
//函数：Virtual_I2C_Send_Byte
//功能AT24CM01数据写入
//说明：
//**************************************************	
void Virtual_I2C_Send_Byte(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin,uint8_t data)
{                        
    uint8_t i = 8;
    
		Virtual_I2C_SDA_OUT(SDA_Port,SDA_Pin);
    while(i--)
    {
				gpio_bit_reset(SCL_Port,SCL_Pin);
				Hal_IIC_WaitUs(10);
        if(data & 0x80)
					gpio_bit_set(SDA_Port,SDA_Pin);       
        else
					gpio_bit_reset(SDA_Port,SDA_Pin);   
        
				Hal_IIC_WaitUs(5);
        data <<= 1;
				gpio_bit_set(SCL_Port,SCL_Pin);    
				Hal_IIC_WaitUs(5);
				gpio_bit_reset(SCL_Port,SCL_Pin);   
				Hal_IIC_WaitUs(5);
    }   
} 	  

//**************************************************
//函数：Virtual_I2C_Read_Byte
//功能AT24CM01数据写入
//说明：
//**************************************************	  
uint8_t Virtual_I2C_Read_Byte(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin)
{	
    uint8_t i = 8;
    uint8_t data = 0;
    
		gpio_bit_set(SDA_Port,SDA_Pin);
		Virtual_I2C_SDA_IN(SDA_Port,SDA_Pin);
	
    while(i--)
    {
        data <<= 1;
				gpio_bit_reset(SCL_Port,SCL_Pin); 
        Hal_IIC_WaitUs(5);
				gpio_bit_set(SCL_Port,SCL_Pin); 
        Hal_IIC_WaitUs(5);
        if(gpio_input_bit_get(SDA_Port,SDA_Pin)) 
            data |= 0x01;
    }
		
		gpio_bit_reset(SCL_Port,SCL_Pin); 
    
    return(data);    
}

//**************************************************
//函数：Virtual_I2C_SlaveWriteByte
//功能AT24CM01数据写入
//说明：
//**************************************************	 
uint8_t Virtual_I2C_SlaveWriteByte(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin,uint8_t dev_addr, uint8_t addr, const uint8_t *pdata)
{    
    Virtual_I2C_Start(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
	
    Virtual_I2C_Send_Byte(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin,dev_addr);
    if(Virtual_I2C_Wait_Ack(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin) == VIRTUAL_I2C_ACK_FAULT)
    {
        Virtual_I2C_Stop(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
        return VIRTUAL_I2C_ACK_FAULT;
    }
		
    Virtual_I2C_Send_Byte(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin,addr);
    if(Virtual_I2C_Wait_Ack(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin) == VIRTUAL_I2C_ACK_FAULT)
    {
        Virtual_I2C_Stop(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
        return VIRTUAL_I2C_ACK_FAULT;
    }

    Virtual_I2C_Send_Byte(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin,*pdata);
    if(Virtual_I2C_Wait_Ack(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin) == VIRTUAL_I2C_ACK_FAULT)
    {
        Virtual_I2C_Stop(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
        return VIRTUAL_I2C_ACK_FAULT;
    }

		Virtual_I2C_Stop(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
    
    return VIRTUAL_I2C_ACK_SUCCESS;    
}

//**************************************************
//函数：Virtual_I2C_ReadString
//功能模拟I2C读取字符串
//说明：
//**************************************************	 
uint8_t Virtual_I2C_SlaveReadString(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin,uint8_t dev_addr, uint8_t addr, uint8_t *pdata, uint16_t num)
{
    uint8_t i;
    
    Virtual_I2C_Start(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
	
    Virtual_I2C_Send_Byte(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin,dev_addr);
    if(Virtual_I2C_Wait_Ack(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin) == VIRTUAL_I2C_ACK_FAULT)
    {
        Virtual_I2C_Stop(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
        return VIRTUAL_I2C_ACK_FAULT;
    }
		
    Virtual_I2C_Send_Byte(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin,addr);
    if(Virtual_I2C_Wait_Ack(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin) == VIRTUAL_I2C_ACK_FAULT)
    {
        Virtual_I2C_Stop(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
        return VIRTUAL_I2C_ACK_FAULT;
    }
    
    Virtual_I2C_Start(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
    Virtual_I2C_Send_Byte(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin,dev_addr|0x01);
    if(Virtual_I2C_Wait_Ack(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin) == VIRTUAL_I2C_ACK_FAULT)
    {
        Virtual_I2C_Stop(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
        return VIRTUAL_I2C_ACK_FAULT;
    }
		
    for(i = 0; i < num; i++)
    {
        *pdata++ = Virtual_I2C_Read_Byte(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
        if(i < (num - 1))
            Virtual_I2C_Ack(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
    }
		
		Virtual_I2C_NAck(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
		
		Virtual_I2C_Stop(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin);
    
    return VIRTUAL_I2C_ACK_SUCCESS;    
}

//**************************************************
//函数：Virtual_I2C_WriteString
//功能模拟I2C写入字符串
//说明：
//**************************************************	 
uint8_t Virtual_I2C_SlaveWriteString(uint32_t SDA_Port, uint32_t SDA_Pin, uint32_t SCL_Port, uint32_t SCL_Pin,uint8_t dev_addr, uint8_t addr, uint8_t *pdata, uint16_t num)
{
    uint8_t ret = 0;
    uint16_t i;

    for(i = 0; i < num; i++)
    {
        ret = Virtual_I2C_SlaveWriteByte(SDA_Port,SDA_Pin,SCL_Port,SCL_Pin,dev_addr, addr++, pdata++);
        if(ret == VIRTUAL_I2C_ACK_FAULT)
        {
            return VIRTUAL_I2C_ACK_FAULT;
        }
        /* 此处延时应大于4ms，等待芯片内部写完成 */
        Hal_IIC_WaitUs(1500);
    }
    return VIRTUAL_I2C_ACK_SUCCESS;
}
