#include <string.h>
#include <stdio.h>

#include "EmbeddedFlashMemory.h"
#include "stm32h7xx_hal.h"
#include "ota.h"

ota_hdr_t g_ota_t;
ota_flag_t g_m;

static uint32_t crc32_full_at(uint32_t base, uint32_t size)
{    
	uint32_t c = 0xFFFFFFFF;
	for (uint32_t i = 0; i < size; i++)    
	{        
		c ^= *(uint8_t *)(base + i);   
		for (int j = 0; j < 8; j++)  
		{           
			c = (c & 1) ? (0xEDB88320u^(c >> 1)):(c >> 1);       
		}    
	}    
	return c^0xFFFFFFFF;
}


static int meta_read(ota_flag_t *m)
{	
	memcpy(m, (void *)Flage_PAGE_ADDR, sizeof(*m));    
	return (m->magic == 0xB00710AD && m->committed == 0x5AA55AA5) ? 0 : -1;
}


static void meta_clear(void)
{    
	bsp_EraseCpuFlash(Flage_PAGE_ADDR);
}


static void ram_swap_copy(uint32_t src, uint32_t size)
{     
    __disable_irq(); // 禁止中断，防止中断取指访问 Flash
   
	uint32_t remain = size;

	bsp_EraseCpuFlash(ADDR_FLASH_SECTOR_1_BANK1);
	bsp_EraseCpuFlash(ADDR_FLASH_SECTOR_2_BANK1);

	// 3. 从暂存区逐半字编程到 APP 区    
	uint32_t s = src;    
	uint32_t d = APP_BASE_ADDR;    
	uint64_t FlashWord[4];
	remain = size; 

	while (remain)    
	{        
		FlashWord[0] = 0xFFFFFFFFFFFFFFFF;
		FlashWord[1] = 0xFFFFFFFFFFFFFFFF;
		FlashWord[2] = 0xFFFFFFFFFFFFFFFF;
		FlashWord[3] = 0xFFFFFFFFFFFFFFFF;
	
		if (remain >= 32)        
		{            
			// 正常拷贝四个字节            
			bsp_ReadCpuFlash(s,(uint8_t*)FlashWord,32);
			s += 32;
			remain -= 32;
		}
		else
		{
			bsp_ReadCpuFlash(s,(uint8_t*)FlashWord,remain);
			s += 32;
			remain = 0;        
		}
			
		bsp_WriteCpuFlash(d,(uint8_t*)FlashWord, 32);
		d += 32;
	}
	
	// 5. 复位 MCU，启动后直接运行新 APP    
	NVIC_SystemReset();
}

void OTA_SwapToBase_FromStaging(uint32_t staging_addr, uint32_t size)
{    
	ram_swap_copy(staging_addr, size);
}


void OTA_CheckAndSwap_Early(void)
{    
	ota_flag_t m;    
	if (meta_read(&m) != 0)        
		return;
	
// 基本边界检查：大小为 0 或超过暂存区容量（预留 1 页给标志）    
	if (m.fw_size == 0 || m.fw_size > (FLASH_END_ADDR - STAGING_BASE - PAGE_SIZE))    
	{        
		meta_clear();        
		return;    
	}
// 计算暂存区固件 CRC32 并比对    
	if (crc32_full_at(STAGING_BASE, m.fw_size) != m.fw_crc32)    
	{        
		meta_clear();       
		return;    
	}
// 标志页清除，准备覆盖    
	meta_clear();    

// 从暂存区覆盖到 0x08000000 基址（该函数应在 RAM 中运行并最终复位）    
	OTA_SwapToBase_FromStaging(STAGING_BASE, m.fw_size);
}


int start_ota(uint32_t total_t,uint16_t block_t,uint16_t size_t)
{    
	uint32_t total = 0; 
	
	const uint32_t staging_limit = FLASH_END_ADDR - STAGING_BASE - PAGE_SIZE; /* 预留最后页存 meta */
    total = total_t; 
	
	g_ota_t.total_size = total_t;
	g_ota_t.block_num = block_t;
	g_ota_t.block_size = size_t;

	if(total == 0 || total > staging_limit)       
		return -102;    

	return 0;
}

int commit_ota(uint16_t off_blk,uint8_t *data,uint16_t lengt)
{    
	uint32_t dst = STAGING_BASE + off_blk*g_ota_t.block_size;

	g_ota_t.chunk_len = lengt;
	g_ota_t.chunk[0] = *data;

	if(((uint32_t)dst == ADDR_FLASH_SECTOR_0_BANK2)||			\
		 ((uint32_t)dst == ADDR_FLASH_SECTOR_1_BANK2)||
		 ((uint32_t)dst == ADDR_FLASH_SECTOR_2_BANK2)||
		 ((uint32_t)dst == ADDR_FLASH_SECTOR_3_BANK2)||
		 ((uint32_t)dst == ADDR_FLASH_SECTOR_4_BANK2)||
		 ((uint32_t)dst == ADDR_FLASH_SECTOR_5_BANK2))			\
					bsp_EraseCpuFlash(dst);

	int ret = bsp_WriteCpuFlash(dst,data,lengt); 
	if (ret != 0)        
		return -106;    
  
	return 0;
}

int verfy_ota_and_mark(void)
{    
	// 这里读了 20 字节到 buf，但未使用；保留以保持功能不变（可能用于你的调试）    
	uint32_t rolling_crc = crc32_full_at(STAGING_BASE, g_ota_t.total_size);    

	ota_flag_t g_m = {0xB00710ADu, g_ota_t.total_size, rolling_crc, 0x5AA55AA5u};

	uint8_t *pm = (uint8_t *)&g_m;

	bsp_EraseCpuFlash(Flage_PAGE_ADDR);
	
	int ret = bsp_WriteCpuFlash(Flage_PAGE_ADDR,pm,sizeof(g_m));
	if (ret != 0)        
		return -107;   

	NVIC_SystemReset();  // 复位，启动后 OTA_CheckAndSwap_Early() 会依据标志执行覆盖    
	
	return 0;
}

