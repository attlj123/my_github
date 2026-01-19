/**
  ******************************************************************************
  * @file           : EmbeddedFlashMemory.h
  * @brief          : stm32 chip flash write/read interface
  ******************************************************************************
  * Copyright (c) 2018 Stelight Instrument Co.,Ltd
  * All rights reserved.
  ******************************************************************************
**/
#ifndef _EMBEDDED_FLASH_MEMORY_H
#define _EMBEDDED_FLASH_MEMORY_H

#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define FLASH_BASE_ADDR      (uint32_t)(FLASH_BASE)
#define FLASH_END_ADDR       (uint32_t)(0x081FFFFF)
#define FLASH_PAGE_SIZE      (uint32_t)0x20000  /* Page size = 128KByte */


/* Base address of the Flash sectors Bank 1 */
#define ADDR_FLASH_SECTOR_0_BANK1     ((uint32_t)0x08000000) /* Base @ of Sector 0, 128 Kbytes */
#define ADDR_FLASH_SECTOR_1_BANK1     ((uint32_t)0x08020000) /* Base @ of Sector 1, 128 Kbytes */
#define ADDR_FLASH_SECTOR_2_BANK1     ((uint32_t)0x08040000) /* Base @ of Sector 2, 128 Kbytes */
#define ADDR_FLASH_SECTOR_3_BANK1     ((uint32_t)0x08060000) /* Base @ of Sector 3, 128 Kbytes */
#define ADDR_FLASH_SECTOR_4_BANK1     ((uint32_t)0x08080000) /* Base @ of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5_BANK1     ((uint32_t)0x080A0000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6_BANK1     ((uint32_t)0x080C0000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7_BANK1     ((uint32_t)0x080E0000) /* Base @ of Sector 7, 128 Kbytes */

/* Base address of the Flash sectors Bank 2 */
#define ADDR_FLASH_SECTOR_0_BANK2     ((uint32_t)0x08100000) /* Base @ of Sector 0, 128 Kbytes */
#define ADDR_FLASH_SECTOR_1_BANK2     ((uint32_t)0x08120000) /* Base @ of Sector 1, 128 Kbytes */
#define ADDR_FLASH_SECTOR_2_BANK2     ((uint32_t)0x08140000) /* Base @ of Sector 2, 128 Kbytes */
#define ADDR_FLASH_SECTOR_3_BANK2     ((uint32_t)0x08160000) /* Base @ of Sector 3, 128 Kbytes */
#define ADDR_FLASH_SECTOR_4_BANK2     ((uint32_t)0x08180000) /* Base @ of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5_BANK2     ((uint32_t)0x081A0000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6_BANK2     ((uint32_t)0x081C0000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7_BANK2     ((uint32_t)0x081E0000) /* Base @ of Sector 7, 128 Kbytes */

/* ************************************************************************** */
#define ADDR_FLASH_BACKUPVERSION_START      ADDR_FLASH_SECTOR_0_BANK2
#define ADDR_FLASH_BACKUPVERSION_END        (ADDR_FLASH_SECTOR_7_BANK2 - 1)

#define FLASH_SECTOR_VERSION_INFO_USING      ADDR_FLASH_SECTOR_7_BANK1
#define FLASH_SECTOR_VERSION_INFO_BAKUP      ADDR_FLASH_SECTOR_7_BANK2

#define FLASH_MCU_VERSION_NAME_START        0
#define FLASH_MCU_VERSION_NAME_LEN          64
#define FLASH_MCU_VERSION_SIZE_START        64
#define FLASH_MCU_VERSION_SIZE_LEN          4
#define FLASH_MCU_VERSION_CRC_START         (FLASH_MCU_VERSION_SIZE_START + FLASH_MCU_VERSION_SIZE_LEN)
#define FLASH_MCU_VERSION_CRC_LEN           4
#define FLASH_MCU_VERSION_VERIFY_FLAG_START 128
#define FLASH_MCU_VERSION_VERIFY_FLAG_LEN   4//如果使用bootloader,application版本第一次正常运行时,会将64字节开始的flash的值写成0x50415353("PASS"),表示自己正常启动过



/* ************************************************************************** */




#define CPU_FLASH_BASE_ADDR      ADDR_FLASH_SECTOR_6_BANK2		/* 0x081C0000 */
#define CPU_FLASH_END_ADDR       (uint32_t)(0x081FFFFF)

#define CPU_FLASH_SIZE       	(2 * 1024 * 1024)	/* FLASH总容量 */
#define CPU_FLASH_SECTOR_SIZE	(128 * 1024)		/* 扇区大小，字节 */

#define FLASH_IS_EQU		0   /* Flash内容和待写入的数据相等，不需要擦除和写操作 */
#define FLASH_REQ_WRITE		1	/* Flash不需要擦除，直接写 */
#define FLASH_REQ_ERASE		2	/* Flash需要先擦除,再写 */
#define FLASH_PARAM_ERR		3	/* 函数参数错误 */




#define ADDR_FLASH_CaliVoltFactork0       ( (uint32_t)0x081C0000 + 8*0)               //100V
#define ADDR_FLASH_CaliVoltFactorb0       ( (uint32_t)0x081C0000 + 8*1)
    
#define ADDR_FLASH_CaliVoltFactork1       ( (uint32_t)0x081C0000 + 8*2)               //10V
#define ADDR_FLASH_CaliVoltFactorb1       ( (uint32_t)0x081C0000 + 8*3)
    
#define ADDR_FLASH_CaliVoltFactork2       ( (uint32_t)0x081C0000 + 8*4)               //5V
#define ADDR_FLASH_CaliVoltFactorb2       ( (uint32_t)0x081C0000 + 8*5)

#define ADDR_FLASH_CaliVoltFactork3       ( (uint32_t)0x081C0000 + 8*6)               //1V
#define ADDR_FLASH_CaliVoltFactorb3       ( (uint32_t)0x081C0000 + 8*7)

#define ADDR_FLASH_CaliCurrFactork0       ( (uint32_t)0x081C0000 + 8*8)               //100UA
#define ADDR_FLASH_CaliCurrFactorb0       ( (uint32_t)0x081C0000 + 8*9)

#define ADDR_FLASH_CaliCurrFactork1       ( (uint32_t)0x081C0000 + 8*10)               //1MA
#define ADDR_FLASH_CaliCurrFactorb1       ( (uint32_t)0x081C0000 + 8*11)

#define ADDR_FLASH_CaliCurrFactork2       ( (uint32_t)0x081C0000 + 8*12)               //10MA
#define ADDR_FLASH_CaliCurrFactorb2       ( (uint32_t)0x081C0000 + 8*13)

#define ADDR_FLASH_CaliCurrFactork3       ( (uint32_t)0x081C0000 + 8*14)               //100MA
#define ADDR_FLASH_CaliCurrFactorb3       ( (uint32_t)0x081C0000 + 8*15)

#define ADDR_FLASH_CaliCurrFactork4       ( (uint32_t)0x081C0000 + 8*16)               //1A
#define ADDR_FLASH_CaliCurrFactorb4       ( (uint32_t)0x081C0000 + 8*17)

#define ADDR_FLASH_CaliCurrFactork5       ( (uint32_t)0x081C0000 + 8*18)               //10A
#define ADDR_FLASH_CaliCurrFactorb5       ( (uint32_t)0x081C0000 + 8*19)
    
#define ADDR_FLASH_CaliCurrFactork6       ( (uint32_t)0x081C0000 + 8*20)               //10A
#define ADDR_FLASH_CaliCurrFactorb6       ( (uint32_t)0x081C0000 + 8*21)    
    

/* ************************************************************************** */

/* Exported functions ------------------------------------------------------- */

#define FLASH_BANK1_USER_START_ADDR         ADDR_FLASH_SECTOR_1_BANK1        /* Start @ of user Flash area Bank1 */
#define FLASH_BANK1_USER_END_ADDR           (ADDR_FLASH_SECTOR_7_BANK1 + FLASH_PAGE_SIZE - 1)  /* End @ of user Flash area Bank1 */

#define FLASH_BANK2_USER_START_ADDR         ADDR_FLASH_SECTOR_0_BANK2        /* Start @ of user Flash area Bank1 */
#define FLASH_BANK2_USER_END_ADDR           (ADDR_FLASH_SECTOR_3_BANK2 + FLASH_PAGE_SIZE - 1)  /* End @ of user Flash area Bank1 */
    
    


/* ************************************************************************** */
uint32_t GetSector(uint32_t Address);
int ReadEmbeddedFlash(uint32_t addr, uint8_t *pbuf, uint32_t len);
int WriteEmbeddedFlash(uint32_t addr, const uint8_t *pbuf, uint32_t len);
int testEmbeddedFlash(void);
int32_t EraseEmbeddFlash(uint32_t addr, uint32_t len);



uint8_t bsp_EraseCpuFlash(uint32_t _ulFlashAddr);
uint32_t bsp_GetSector(uint32_t Address);
uint8_t bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpSrc, uint32_t _ulSize);
uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpDst, uint32_t _ulSize);

#endif

