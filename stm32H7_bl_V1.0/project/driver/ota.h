#ifndef OTA_H
#define OTA_H
#include <stdint.h>

#define 		APP_BASE_ADDR 		ADDR_FLASH_SECTOR_0_BANK1 + PAGE_SIZE					// APP 起始地址
#define 		APP_LIMIT					ADDR_FLASH_SECTOR_4_BANK1            					// APP 使用区域结束地址
#define 		STAGING_BASE      ADDR_FLASH_SECTOR_0_BANK2           					// ota 文件存放起始地址
//#define     FLASH_END_ADDR    ADDR_FLASH_SECTOR_4_BANK2           					// ota 文件结束地址
#define     PAGE_SIZE        	FLASH_PAGE_SIZE                  							// 单页flash 大小
#define     Flage_PAGE_ADDR  	ADDR_FLASH_SECTOR_4_BANK1 - PAGE_SIZE					// 最后一页用来存放标志位

typedef struct __attribute__((packed))
{
    uint32_t total_size;// 总ota文件大小
    uint16_t block_num;
    uint16_t block_size; 		// 当前已经发送的偏移/1 本片段数据长度
    uint16_t chunk_len;
    uint8_t chunk[0];		// 当前发送数据内容
}ota_hdr_t;

typedef struct 
{
    uint32_t magic; // 魔术字，用来标识这个结构是否有效
    uint32_t fw_size;  // 新固件的大小(字节数)，一般 ≤ 暂存区大小
    uint32_t fw_crc32; // 新固件的 CRC32 校验值
    uint32_t committed; //升级标志:0x5AA55AA5 表示下次重启要执行覆盖
}ota_flag_t;

//void uart_ota_data(uint8_t *uart3_rx_buff,uint16_t size);

int start_ota(uint32_t total_t,uint16_t block_t,uint16_t size_t);
int commit_ota(uint16_t off_blk,uint8_t *data,uint16_t lengt);
int verfy_ota_and_mark(void);

void OTA_checkAndswap_Early(void);
void OTA_SwapToBase_Fromstaging(uint32_t staging_addr, uint32_t size);

#endif

