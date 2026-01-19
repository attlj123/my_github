/**
  ******************************************************************************
  * @file           : EmbeddedFlashMemory.c
  * @brief          : stm32 chip flash write/read interface
  ******************************************************************************
  * Copyright (c) 2018 Stelight Instrument Co.,Ltd
  * All rights reserved.
  ******************************************************************************
**/
#include <string.h>
#include "stm32h7xx_hal.h"
#include "stm32H7xx_hal_flash_ex.h"
#include "EmbeddedFlashMemory.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#define FLASH_WAITETIME  50000

/*Variable used for Erase procedure*/
static FLASH_EraseInitTypeDef EraseInitStruct;

#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=8
#endif /* defined ( __ICCARM__ ) */

/* receive command buffer. */
__ALIGN_BEGIN static uint8_t alignProgramFlashBuf[256 / 8] __ALIGN_END = {0};//256bit / sizeof(byte)

/* Private function prototypes -----------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Gets the sector of a given address
  * @param  Address Address of the FLASH Memory
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;
    
    if(((Address < ADDR_FLASH_SECTOR_1_BANK1) && (Address >= ADDR_FLASH_SECTOR_0_BANK1)) || \
      ((Address < ADDR_FLASH_SECTOR_1_BANK2) && (Address >= ADDR_FLASH_SECTOR_0_BANK2)))
    {
        sector = FLASH_SECTOR_0;
    }
    else if(((Address < ADDR_FLASH_SECTOR_2_BANK1) && (Address >= ADDR_FLASH_SECTOR_1_BANK1)) || \
            ((Address < ADDR_FLASH_SECTOR_2_BANK2) && (Address >= ADDR_FLASH_SECTOR_1_BANK2)))
    {
        sector = FLASH_SECTOR_1;
    }
    else if(((Address < ADDR_FLASH_SECTOR_3_BANK1) && (Address >= ADDR_FLASH_SECTOR_2_BANK1)) || \
            ((Address < ADDR_FLASH_SECTOR_3_BANK2) && (Address >= ADDR_FLASH_SECTOR_2_BANK2)))
    {
        sector = FLASH_SECTOR_2;
    }
    else if(((Address < ADDR_FLASH_SECTOR_4_BANK1) && (Address >= ADDR_FLASH_SECTOR_3_BANK1)) || \
            ((Address < ADDR_FLASH_SECTOR_4_BANK2) && (Address >= ADDR_FLASH_SECTOR_3_BANK2)))
    {
        sector = FLASH_SECTOR_3;
    }
    else if(((Address < ADDR_FLASH_SECTOR_5_BANK1) && (Address >= ADDR_FLASH_SECTOR_4_BANK1)) || \
            ((Address < ADDR_FLASH_SECTOR_5_BANK2) && (Address >= ADDR_FLASH_SECTOR_4_BANK2)))
    {
        sector = FLASH_SECTOR_4;
    }
    else if(((Address < ADDR_FLASH_SECTOR_6_BANK1) && (Address >= ADDR_FLASH_SECTOR_5_BANK1)) || \
            ((Address < ADDR_FLASH_SECTOR_6_BANK2) && (Address >= ADDR_FLASH_SECTOR_5_BANK2)))
    {
        sector = FLASH_SECTOR_5;
    }
    else if(((Address < ADDR_FLASH_SECTOR_7_BANK1) && (Address >= ADDR_FLASH_SECTOR_6_BANK1)) || \
            ((Address < ADDR_FLASH_SECTOR_7_BANK2) && (Address >= ADDR_FLASH_SECTOR_6_BANK2)))
    {
        sector = FLASH_SECTOR_6;
    }
    else if(((Address < ADDR_FLASH_SECTOR_0_BANK2) && (Address >= ADDR_FLASH_SECTOR_7_BANK1)) || \
            ((Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_7_BANK2)))
    {
        sector = FLASH_SECTOR_7;
    }
    else
    {
        sector = FLASH_SECTOR_7;
    }
    
    return sector;
}






int ReadEmbeddedFlash(uint32_t addr, uint8_t *pbuf, uint32_t len)
{
    int32_t validRangeBank1 = (addr >= FLASH_BANK1_USER_START_ADDR) && ((addr + len) <= FLASH_BANK1_USER_END_ADDR);
    int32_t validRangeBank2 = (addr >= FLASH_BANK2_USER_START_ADDR) && ((addr + len) <= FLASH_BANK2_USER_END_ADDR);
    if(!(validRangeBank1 || validRangeBank2))
    {
        return HAL_ERROR;
    }
    
    uint8_t *readbuf = (uint8_t *)addr;
    int i = 0;
    while(i < len)
    {        
        pbuf[i] = readbuf[i];
        i++;
    }
    
    return HAL_OK;
}

static int32_t CheckWriteEmbeddFlashAddr(uint32_t addr, const uint8_t *pbuf, uint32_t len)
{
    int32_t validRangeBank1 = (addr >= FLASH_BANK1_USER_START_ADDR) && (addr <= FLASH_BANK1_USER_END_ADDR)
                           && ((addr + len) >= FLASH_BANK1_USER_START_ADDR) && ((addr + len) <= FLASH_BANK1_USER_END_ADDR);
    int32_t validRangeBank2 = (addr >= FLASH_BANK2_USER_START_ADDR) && (addr <= FLASH_BANK2_USER_END_ADDR)
                           && ((addr + len) >= FLASH_BANK2_USER_START_ADDR) && ((addr + len) <= FLASH_BANK2_USER_END_ADDR);
    int32_t updateVersion = (addr >= ADDR_FLASH_SECTOR_0_BANK2) && (addr <= FLASH_BANK2_USER_START_ADDR-1)
                           && ((addr + len) >= ADDR_FLASH_SECTOR_0_BANK2) && ((addr + len) <= FLASH_BANK2_USER_START_ADDR);
    if((pbuf == 0) || (len == 0) || (!(validRangeBank1 || validRangeBank2 || updateVersion)))
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

int32_t EraseEmbeddFlash(uint32_t addr, uint32_t len)
{
    uint32_t SECTORError = 0;
    uint32_t FirstSector = 0;
    uint32_t NbOfSectors = 0;
    uint32_t alignAddr = (addr % 32 == 0) ? (addr) : ((addr / 32 + 1) * 32);
    uint32_t alignLen = (len % 32 == 0) ? (len) : ((len / 32 + 1) * 32);
    
    /* Get the 1st sector to erase */
    FirstSector = GetSector(addr);
    /* Get the number of sector to erase from 1st sector*/
    NbOfSectors = GetSector(addr + len) - FirstSector + 1;
   
    uint32_t bank = addr < ADDR_FLASH_SECTOR_0_BANK2 ? FLASH_BANK_1 : FLASH_BANK_2;

    uint8_t *readFlashAddr = (uint8_t *)addr;
    for(uint32_t i = 0; i < len; i++)
    {
        if(*(readFlashAddr + i) != 0xFF)//如果要写入的区域全部都是0xFF的话,也有可能不去擦除,可以直接写入
        {
            /* Fill EraseInit structure*/
            EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
            EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
            EraseInitStruct.Banks         = bank;
            EraseInitStruct.Sector        = FirstSector;
            EraseInitStruct.NbSectors     = NbOfSectors;
            
            if(HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
            {
                /*
                  Error occurred while sector erase.
                  User can add here some code to deal with this error.
                  SECTORError will contain the faulty sector and then to know the code error on this sector,
                  user can call function 'HAL_FLASH_GetError()'
                */
                /* Infinite loop */
                //while (1)
                //{
                //;;
                //}
                
                return HAL_ERROR;
            }
            FLASH_WaitForLastOperation(FLASH_WAITETIME, bank);
            //SCB_InvalidateDCache_by_Addr((uint32_t *)addr, len);
            SCB_InvalidateDCache_by_Addr((uint32_t *)alignAddr, alignLen);
        }
    }
    
    return HAL_OK;
}

static int32_t PorgrammEmbeddedFlash(uint32_t addr, const uint8_t *pbuf, uint32_t len)
{
    uint32_t bank = addr < ADDR_FLASH_SECTOR_0_BANK2 ? FLASH_BANK_1 : FLASH_BANK_2;
    uint32_t Address = addr;
    int remainSize = len;
    
    HAL_StatusTypeDef flashProgResult = HAL_OK;
    const uint8_t *srcPtr = pbuf;

    uint32_t firstWriteAddr = 0;
    uint32_t addrRemainder = Address % 32;
    if (addrRemainder != 0)
    {
        firstWriteAddr = Address - addrRemainder;
        for(uint32_t index = 0; index < addrRemainder; ++index)
        {
            alignProgramFlashBuf[index] = 0xFF;
        }

        if (addrRemainder + len >= 32)
        {
            for(uint32_t index = addrRemainder; index < 32; ++index)
            {
                alignProgramFlashBuf[index] = *srcPtr++;
            }

            remainSize -= (32 - addrRemainder);
        }
        else
        {
            for(uint32_t index = addrRemainder; index < addrRemainder + len; ++index)
            {
                alignProgramFlashBuf[index] = *srcPtr++;
            }

            for(uint32_t index = addrRemainder + len; index < 32; ++index)
            {
                alignProgramFlashBuf[index] = 0xFF;
            }

            remainSize = 0;
        }

        flashProgResult = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, firstWriteAddr, (uint32_t)alignProgramFlashBuf);
        FLASH_WaitForLastOperation(FLASH_WAITETIME, bank);

        Address = firstWriteAddr + 32;
    }
    
    while((flashProgResult == HAL_OK) && (Address < addr + len))
    {
        if(remainSize >= 32)
        {
            for(uint32_t index = 0; index < 32; ++index)
            {
                alignProgramFlashBuf[index] = *srcPtr++;
            }
        }
        else
        {
            for(uint32_t index = 0; index < remainSize; ++index)
            {
                alignProgramFlashBuf[index] = *srcPtr++;
            }
            
            for(uint32_t index = remainSize; index < 32; ++index)
            {
                alignProgramFlashBuf[index] = 0xFF;//填充erase后的默认值
            }
        }        
        
        flashProgResult = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, Address, (uint32_t)alignProgramFlashBuf);
        FLASH_WaitForLastOperation(FLASH_WAITETIME, bank);
        Address = Address + 32; /* increment for the next Flash word*/
        remainSize -= 32;
    }

    return flashProgResult;
}

static int32_t VerifyPorgrammEmbeddedFlash(uint32_t addr, const uint8_t *pbuf, uint32_t len)
{
    //stm32 example
    /*Address = FLASH_USER_START_ADDR;
    uint64_t* writeData = (uint64_t*)flashWriteBuffer;//验证时候64bit 的效率最高，循环最少
    MemoryProgramStatus = 0x0;
    
    while (Address < FLASH_USER_END_ADDR)
    {
      for(Index = 0; Index < (FALSH_WORD_LEN / 8); Index++)
      {
        data64 = *(uint64_t*)Address;
        __DSB();
        if(data64 != writeData[Index])
        {
          MemoryProgramStatus++;
        }
        Address +=8;
      }
    }*/
    
    uint32_t Address = addr;
    __IO uint32_t MemoryProgramStatus = 0;

    uint32_t j = 0;
    uint8_t data8 = 0;
    while(j < len)
    {
        data8 = *(((uint8_t *)Address) + j);
        __DSB();
        if(data8 != pbuf[j])
        {
            MemoryProgramStatus++;
        }
        
        j++;
    }
    
    if(MemoryProgramStatus == 0)
    {
        /* No error detected. Switch on LED1*/
        return HAL_OK;
    }
    else
    {
        /* Error detected. Toggle LED3*/
        //while (1)
        //{
        //HAL_Delay(100);
        //}
        return HAL_ERROR;
    }
    
    /* Infinite loop */
    //while (1)
    //{
    //}
}

/*
   1.由于擦除以128kB为单位，所以可能会有数据丢失！
   本函数用于一次写入falsh的情况, 有erase操作，请调用方自己组织buf，一次写入
   如果要是想考虑在有eraser下保存原来的内容的情况，要不然使用一个flash做备份，并且极大的拖慢速度和增加复杂性并减少使用寿命，要不然的话就要有一个128kB的大内存做临时buffer，都是不合适的
   所以本函数只考虑擦除后重新写入，不对原来的内容做读取
   2.允许翻页,但是牵涉到的页都会被擦除,不能跨越bank
   3.本函数具有自动检测功能，如果要写入的地址全部都是FF，则不会再去做擦除操作。也就是自动做到只擦除一次。但是依然会有数据丢失的可能
*/

/*擦除与写入时间
  datasheet: STM32H743xI table52:
  tERASE128KB:max 4s 
  tprog:Word (266 bits: 256bit + 10ECCbit) programming time: 580(8bit)-260(32bit)-200(64bit)us
*/

int WriteEmbeddedFlash(uint32_t addr, const uint8_t *pbuf, uint32_t len)//不考虑分页的情况,暂时只考虑从256bit的对齐位数开始写
{
    int32_t result = 0;
    
    /* -1- Check addr */    
    if(CheckWriteEmbeddFlashAddr(addr, pbuf, len) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* -2- Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();
    
    /* -3- Erase the user Flash area
      (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
    result = EraseEmbeddFlash(addr, len);
    
    /* -4- Program the user Flash area word by word
      (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
    if (result == HAL_OK)
    {
        result = PorgrammEmbeddedFlash(addr, pbuf, len);
    }    
    
    /* -5- Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    if(result != HAL_OK)
    {
        return HAL_ERROR;
    }
    
    /* -6- Check if the programmed data is OK
        MemoryProgramStatus = 0: data programmed correctly
        MemoryProgramStatus != 0: number of words not programmed correctly ******/
    result = VerifyPorgrammEmbeddedFlash(addr, pbuf, len);
    
    /* -7- Check if there is an issue to program data*/    
    return result;
}

int testEmbeddedFlash(void)
{
    int result = 0xFF;
    uint8_t writeBuf[128] = {0};
    int testWriteInterval = sizeof(writeBuf);
    for (int i = 0; i < sizeof(writeBuf); i++)
    {
        writeBuf[i] = i;
    }

    for (int testCmdTime = 0; testCmdTime < 5; testCmdTime++)
    {
        switch (testCmdTime)
        {
            case 0:
                result = WriteEmbeddedFlash(ADDR_FLASH_SECTOR_6_BANK1, writeBuf, sizeof(writeBuf));
                break;
            case 1:
                result = WriteEmbeddedFlash(ADDR_FLASH_SECTOR_6_BANK1 + testWriteInterval * testCmdTime + 3, writeBuf, sizeof(writeBuf) - 3);
                break;
            case 2:
                result = WriteEmbeddedFlash(ADDR_FLASH_SECTOR_6_BANK1 + testWriteInterval * testCmdTime + 23, writeBuf, sizeof(writeBuf) - 23);
                break;
            case 3:
                result = WriteEmbeddedFlash(ADDR_FLASH_SECTOR_6_BANK1 + testWriteInterval * testCmdTime + 23, writeBuf, 32);
                break;
            case 4:
                result = WriteEmbeddedFlash(ADDR_FLASH_SECTOR_6_BANK1 + testWriteInterval * testCmdTime + 5, writeBuf, 4);
                break;
            default:
                break;
        }   
    }    
    
    return result;
}


////////////////////////////////////////////by hongming//////////////////////////////



/*
*********************************************************************************************************
*	函 数 名: bsp_GetSector
*	功能说明: 根据地址计算扇区首地址
*	形    参: 无
*	返 回 值: 扇区号（0-7)
*********************************************************************************************************
*/
uint32_t bsp_GetSector(uint32_t Address)
{
	uint32_t sector = 0;

	if (((Address < ADDR_FLASH_SECTOR_1_BANK1) && (Address >= ADDR_FLASH_SECTOR_0_BANK1)) || \
		((Address < ADDR_FLASH_SECTOR_1_BANK2) && (Address >= ADDR_FLASH_SECTOR_0_BANK2)))    
	{
		sector = FLASH_SECTOR_0;  
	}
	else if (((Address < ADDR_FLASH_SECTOR_2_BANK1) && (Address >= ADDR_FLASH_SECTOR_1_BANK1)) || \
	  ((Address < ADDR_FLASH_SECTOR_2_BANK2) && (Address >= ADDR_FLASH_SECTOR_1_BANK2)))    
	{
		sector = FLASH_SECTOR_1;  
	}
	else if (((Address < ADDR_FLASH_SECTOR_3_BANK1) && (Address >= ADDR_FLASH_SECTOR_2_BANK1)) || \
	  ((Address < ADDR_FLASH_SECTOR_3_BANK2) && (Address >= ADDR_FLASH_SECTOR_2_BANK2)))    
	{
		sector = FLASH_SECTOR_2;  
	}
	else if (((Address < ADDR_FLASH_SECTOR_4_BANK1) && (Address >= ADDR_FLASH_SECTOR_3_BANK1)) || \
	  ((Address < ADDR_FLASH_SECTOR_4_BANK2) && (Address >= ADDR_FLASH_SECTOR_3_BANK2)))    
	{
		sector = FLASH_SECTOR_3;  
	}
	else if (((Address < ADDR_FLASH_SECTOR_5_BANK1) && (Address >= ADDR_FLASH_SECTOR_4_BANK1)) || \
	  ((Address < ADDR_FLASH_SECTOR_5_BANK2) && (Address >= ADDR_FLASH_SECTOR_4_BANK2)))    
	{
		sector = FLASH_SECTOR_4;  
	}
	else if (((Address < ADDR_FLASH_SECTOR_6_BANK1) && (Address >= ADDR_FLASH_SECTOR_5_BANK1)) || \
	  ((Address < ADDR_FLASH_SECTOR_6_BANK2) && (Address >= ADDR_FLASH_SECTOR_5_BANK2)))    
	{
		sector = FLASH_SECTOR_5;  
	}
	else if (((Address < ADDR_FLASH_SECTOR_7_BANK1) && (Address >= ADDR_FLASH_SECTOR_6_BANK1)) || \
	  ((Address < ADDR_FLASH_SECTOR_7_BANK2) && (Address >= ADDR_FLASH_SECTOR_6_BANK2)))    
	{
		sector = FLASH_SECTOR_6;  
	}
	else if (((Address < ADDR_FLASH_SECTOR_0_BANK2) && (Address >= ADDR_FLASH_SECTOR_7_BANK1)) || \
	  ((Address < CPU_FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_7_BANK2)))
	{
		sector = FLASH_SECTOR_7;  
	}
	else
	{
		sector = FLASH_SECTOR_7;
	}

	return sector;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_ReadCpuFlash
*	功能说明: 读取CPU Flash的内容
*	形    参:  _ucpDst : 目标缓冲区
*			 _ulFlashAddr : 起始地址
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值: 0=成功，1=失败
*********************************************************************************************************
*/
uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpDst, uint32_t _ulSize)
{
	uint32_t i;

	if (_ulFlashAddr + _ulSize > CPU_FLASH_BASE_ADDR + CPU_FLASH_SIZE)
	{
		return 1;
	}

	/* 长度为0时不继续操作,否则起始地址为奇地址会出错 */
	if (_ulSize == 0)
	{
		return 1;
	}

	for (i = 0; i < _ulSize; i++)
	{
		*_ucpDst++ = *(uint8_t *)_ulFlashAddr++;
	}

	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CmpCpuFlash
*	功能说明: 比较Flash指定地址的数据.
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpBuf : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值:
*			FLASH_IS_EQU		0   Flash内容和待写入的数据相等，不需要擦除和写操作
*			FLASH_REQ_WRITE		1	Flash不需要擦除，直接写
*			FLASH_REQ_ERASE		2	Flash需要先擦除,再写
*			FLASH_PARAM_ERR		3	函数参数错误
*********************************************************************************************************
*/
uint8_t bsp_CmpCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpBuf, uint32_t _ulSize)
{
	uint32_t i;
	uint8_t ucIsEqu;	/* 相等标志 */
	uint8_t ucByte;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > CPU_FLASH_BASE_ADDR + CPU_FLASH_SIZE)
	{
		return FLASH_PARAM_ERR;		/*　函数参数错误　*/
	}

	/* 长度为0时返回正确 */
	if (_ulSize == 0)
	{
		return FLASH_IS_EQU;		/* Flash内容和待写入的数据相等 */
	}

	ucIsEqu = 1;			/* 先假设所有字节和待写入的数据相等，如果遇到任何一个不相等，则设置为 0 */
	for (i = 0; i < _ulSize; i++)
	{
		ucByte = *(uint8_t *)_ulFlashAddr;

		if (ucByte != *_ucpBuf)
		{
			if (ucByte != 0xFF)
			{
				return FLASH_REQ_ERASE;		/* 需要擦除后再写 */
			}
			else
			{
				ucIsEqu = 0;	/* 不相等，需要写 */
			}
		}

		_ulFlashAddr++;
		_ucpBuf++;
	}

	if (ucIsEqu == 1)
	{
		return FLASH_IS_EQU;	/* Flash内容和待写入的数据相等，不需要擦除和写操作 */
	}
	else
	{
		return FLASH_REQ_WRITE;	/* Flash不需要擦除，直接写 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_EraseCpuFlash
*	功能说明: 擦除CPU FLASH一个扇区 （128KB)
*	形    参: _ulFlashAddr : Flash地址
*	返 回 值: 0 成功， 1 失败
*			  HAL_OK       = 0x00,
*			  HAL_ERROR    = 0x01,
*			  HAL_BUSY     = 0x02,
*			  HAL_TIMEOUT  = 0x03
*
*********************************************************************************************************
*/
uint8_t bsp_EraseCpuFlash(uint32_t _ulFlashAddr)
{
	uint32_t FirstSector = 0, NbOfSectors = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SECTORError = 0;
	uint8_t re;

	/* 解锁 */
	HAL_FLASH_Unlock();
	
	/* 获取此地址所在的扇区 */
	FirstSector = bsp_GetSector(_ulFlashAddr);
	
	/* 固定1个扇区 */
	NbOfSectors = 1;	

	/* 擦除扇区配置 */
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	
	if (_ulFlashAddr >= ADDR_FLASH_SECTOR_0_BANK2)
	{
		EraseInitStruct.Banks         = FLASH_BANK_2;
	}
	else
	{
		EraseInitStruct.Banks         = FLASH_BANK_1;
	}
	
	EraseInitStruct.Sector        = FirstSector;
	EraseInitStruct.NbSectors     = NbOfSectors;
	
	/* 扇区擦除 */	
	re = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
	
	/* 擦除完毕后，上锁 */
	HAL_FLASH_Lock();	
	
	return re;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_WriteCpuFlash
*	功能说明: 写数据到CPU 内部Flash。 必须按32字节整数倍写。不支持跨扇区。扇区大小128KB. \
*			  写之前需要擦除扇区. 长度不是32字节整数倍时，最后几个字节末尾补0写入.
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpSrc : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节, 必须是32字节整数倍）
*	返 回 值: 0-成功，1-数据长度或地址溢出，2-写Flash出错(估计Flash寿命到)
*********************************************************************************************************
*/
uint8_t bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpSrc, uint32_t _ulSize)
{
	uint32_t i;
	uint8_t ucRet;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > CPU_FLASH_BASE_ADDR + CPU_FLASH_SIZE)
	{
		return 1;
	}

	/* 长度为0时不继续操作  */
	if (_ulSize == 0)
	{
		return 0;
	}

	ucRet = bsp_CmpCpuFlash(_ulFlashAddr, _ucpSrc, _ulSize);

	if (ucRet == FLASH_IS_EQU)
	{
		return 0;
	}

	__set_PRIMASK(1);  		/* 关中断 */

	/* FLASH 解锁 */
	HAL_FLASH_Unlock();

	for (i = 0; i < _ulSize / 32; i++)	
	{
		uint64_t FlashWord[4];
		
		memcpy((char *)FlashWord, _ucpSrc, 32);
		_ucpSrc += 32;
		
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, _ulFlashAddr, (uint64_t)((uint32_t)FlashWord)) == HAL_OK)
		{
			_ulFlashAddr = _ulFlashAddr + 32; /* 递增，操作下一个256bit */				
		}		
		else
		{
			goto err;
		}
	}
	
	/* 长度不是32字节整数倍 */
	if (_ulSize % 32)
	{
		uint64_t FlashWord[4];
		
		FlashWord[0] = 0;
		FlashWord[1] = 0;
		FlashWord[2] = 0;
		FlashWord[3] = 0;
		memcpy((char *)FlashWord, _ucpSrc, _ulSize % 32);
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, _ulFlashAddr, (uint64_t)((uint32_t)FlashWord)) == HAL_OK)
		{
			; // _ulFlashAddr = _ulFlashAddr + 32;		
		}		
		else
		{
			goto err;
		}
	}
	
  	/* Flash 加锁，禁止写Flash控制寄存器 */
  	HAL_FLASH_Lock();

  	__set_PRIMASK(0);  		/* 开中断 */

	return 0;
	
err:
  	/* Flash 加锁，禁止写Flash控制寄存器 */
  	HAL_FLASH_Lock();

  	__set_PRIMASK(0);  		/* 开中断 */

	return 1;
}





