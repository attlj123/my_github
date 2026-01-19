#include "Driver_Sleep.h"
#include "App_Link.h"
#include "App_Usart.h"

#include "embeddedFlashMemory.h"

#include "stm32h7xx_hal.h"

uint8 errormessage=0;

extern uint16 g_Rs485Add;
extern void Error_Handler(void);
/********************************************************************
功能:计算帧校验位
输入:DataBuffer 帧首地址
     Length 帧长度
输出:校验值
*********************************************************************/
uint8 App_Link_GetVerify(uint8* DataBuffer,uint16 Length)
{
    uint16 i=0;
    uint8 Temp=0;
    
    for(i=1;i<Length;i++)
    {
        Temp=Temp+*(DataBuffer+i);
    }
    Temp=~Temp+VERIFY_HEX;
    
    return Temp;
}


/********************************************************************
功能:检查帧校验位是否正确
输入:DataBuffer 帧首地址
     Length 帧长度
     kind 存储帧kind位
     Index 存储帧Index位
输出:校验是否正确
*********************************************************************/
uint8 App_Link_CheckVerify(uint8* DataBuffer,uint16 Length,uint8 *kind,uint8 *Index)
{
    if(*DataBuffer!=FRAME_HEAD)
        return FRAME_HEAD_ERROR;
    if(*(DataBuffer+Length-1)!=FRAME_END)
        return FRAME_END_ERROR;
    if(*(DataBuffer+Length-2)!=App_Link_GetVerify((uint8*)DataBuffer,Length-2))
        return FRAME_VERIFY_ERROR;

    *kind=*(DataBuffer+1);
    *Index=*(DataBuffer+2);
    
    return FRAME_SUCCESS;
}


/********************************************************************
功能:Usart发送应答信息
输入:无
输出:无
*********************************************************************/
void App_Link_TxAckPacket(uint8 Kind,uint8 Index)
{
    g_UsartStruct.UsartTxBuffer[0]=FRAME_HEAD;
    g_UsartStruct.UsartTxBuffer[1]=Kind;
    g_UsartStruct.UsartTxBuffer[2]=Index;
    g_UsartStruct.UsartTxBuffer[3]=0x00;
    g_UsartStruct.UsartTxBuffer[4]=0x00;
    g_UsartStruct.UsartTxBuffer[5]=App_Link_GetVerify(g_UsartStruct.UsartTxBuffer,5);
    g_UsartStruct.UsartTxBuffer[6]=FRAME_END;
    
    App_Usart_TxFrame(g_UsartStruct.UsartTxBuffer,7);
}


/********************************************************************
功能:获取文件大小信息
输入:CardNum 板卡编号
     TimeOut 超时时间
     FileSize 存储文件大小
输出:错误信息
*********************************************************************/
uint8 App_Link_ReadFileSize (uint32 TimeOut,uint32 *FileSize)
{
    uint32 j=TimeOut;
    uint8 UsartMessage=GET_FRAME_NOT_OVER;
    uint8 Kind=0;
    uint8 Index=0;
    
    App_Link_TxAckPacket(0x06,0x00) ; //发送准备好应答帧
    while(--j)
    {
        UsartMessage=App_Usart_GetFrame();
        if(UsartMessage==GET_FRAME_OVER)
        {
            errormessage=App_Link_CheckVerify(g_UsartStruct.UsartRxBuffer,g_UsartStruct.UsartRxFrameLength,&Kind,&Index);
            if( errormessage )
            {
                App_Usart_ClearFrame();
                return errormessage;
            }

            if(Kind!=0x01)
            {
                App_Usart_ClearFrame();
                return FRAME_KIND_ERROR;
            }
            
            if(Index!=0x00)
            {
                App_Usart_ClearFrame();
                return FRAME_INDEX_ERROR;
            }
            
            if(g_UsartStruct.UsartRxBuffer[4]!=g_Rs485Add)
            {//板卡号错误
                App_Usart_ClearFrame();
                return FRAME_CARDNUM_ERROR;
            }

            *FileSize=(((uint32)g_UsartStruct.UsartRxBuffer[5])<<24)+(((uint32)g_UsartStruct.UsartRxBuffer[6])<<16)+(((uint32)g_UsartStruct.UsartRxBuffer[7])<<8)+g_UsartStruct.UsartRxBuffer[8];
            
            App_Usart_ClearFrame();
            
            return RECEIVE_SUCCESS;
        }
    }

    return TIME_OUT;
}


/********************************************************************
功能:获取文件数据
输入:TimeOut 超时时间
     IndexNeed 第几帧数据
     FileOver  存储文件结束信息
     sign 读取成功信息
     DataLength 存储文件长度
输出:文件数据
*********************************************************************/
uint8* App_Link_ReadData(uint32 TimeOut,uint8 IndexNeed,uint8 *FileOver,uint8 *sign,uint8 *DataLength)
{
    uint32 j=TimeOut;
    uint8 UsartMessage=GET_FRAME_NOT_OVER;
    uint8 Kind=0;
    uint8 Index=0;

    App_Link_TxAckPacket(0x04,IndexNeed) ; //获取文件内容
    while(--j)
    {
        UsartMessage=App_Usart_GetFrame();
        if(UsartMessage==GET_FRAME_OVER)
        {
            errormessage=App_Link_CheckVerify(g_UsartStruct.UsartRxBuffer,g_UsartStruct.UsartRxFrameLength,&Kind,&Index);
            if( errormessage )
            {
                App_Usart_ClearFrame();
                *sign=errormessage;
                return PNULL;
            }
            
            if(Index!=(uint8)(IndexNeed+1))
            {
                App_Usart_ClearFrame();
                *sign=FRAME_INDEX_ERROR;
                return PNULL;
            }

            if(Kind==0x02)//没有完成
            {
                App_Usart_ClearFrame();
                
                *FileOver=DATA_TRANS_NOT_OVER;
                *DataLength=g_UsartStruct.UsartRxBuffer[3];  //实际数据的长度
                *sign=RECEIVE_SUCCESS;
                
                return &(g_UsartStruct.UsartRxBuffer[4]);
            }
            else if(Kind==0x03)//接收完成
            {
                App_Usart_ClearFrame();
                
                *FileOver=DATA_TRANS_OVER;
                *DataLength=g_UsartStruct.UsartRxBuffer[3];  //实际数据的长度
                *sign=RECEIVE_SUCCESS;
                
                return &(g_UsartStruct.UsartRxBuffer[4]);
            }
            else
            {
                App_Usart_ClearFrame();
                *sign=FRAME_KIND_ERROR;
                return PNULL;
            }
        }
    }
    App_Usart_ClearFrame();
    *sign=TIME_OUT;
    return PNULL;
}


/********************************************************************
功能:计算文件需要暂用多少页FLASH
输入:FileSize 文件大小
输出:文件数据
*********************************************************************/
uint32 App_Link_FLASH_PagesMask(uint32 FileSize)
{
    uint32 pagenumber = 0;
    uint32 size = FileSize;

    if ((size % FLASH_PAGE_SIZE) != 0)
    {
        pagenumber = (size / FLASH_PAGE_SIZE) + 1;
    }
    else
    {
        pagenumber = size / FLASH_PAGE_SIZE;
    }
    
    return pagenumber;
}

/********************************************************************
功能:STMFLASH_Write
输入:Flash 写入
输出:文件数据
*********************************************************************/
static uint32_t STMFLASH_GetFlashSector(uint32_t Address)
{
   uint32_t sector = 0;

	if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) 
		sector = FLASH_SECTOR_0;
	else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1)) 
		sector = FLASH_SECTOR_1;
	else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
		sector = FLASH_SECTOR_2;
	else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3)) 
		sector = FLASH_SECTOR_3;
	else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
		sector = FLASH_SECTOR_4;
	else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
		sector = FLASH_SECTOR_5; 
	else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6)) 
		sector = FLASH_SECTOR_6; 
	else 
		sector = FLASH_SECTOR_7;
   return sector;
}

uint32_t STMFLASH_ReadWord(uint32_t faddr)
{
	return *(uint32_t*)faddr; 
}

void STMFLASH_Read(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t NumToRead)   	
{
	uint32_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//读取4个字节.
		ReadAddr+=4;//偏移4个字节.	
	}
}

void STMFLASH_Write(uint32_t WriteAddr,uint32_t *pBuffer,uint32_t NumToWrite)	
{ 
	FLASH_EraseInitTypeDef FlashEraseInit;
	HAL_StatusTypeDef FlashStatus=HAL_OK;
	uint32_t SectorError=0;
	uint32_t addrx=0;
	uint32_t endaddr=0;	
	
	uint32_t FlashWriteData = 0;
	
	if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)
		return;	//非法地址
    
	HAL_FLASH_Unlock();             //解锁	
	addrx=WriteAddr;				//写入的起始地址
	endaddr=WriteAddr+NumToWrite*4;	//写入的结束地址
    
	if(addrx<0X1FF00000)    //第一块FLASH 1M位置
	{
		while(addrx<endaddr)		//扫清一切障碍.(对非FFFFFFFF的地方,先擦除)
		{
			if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//有非0XFFFFFFFF的地方,要擦除这个扇区
			{   
				FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;       //擦除类型，扇区擦除 
				FlashEraseInit.Sector=STMFLASH_GetFlashSector(addrx);   //要擦除的扇区
				FlashEraseInit.NbSectors=1;                             //一次只擦除一个扇区
				FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_2;      //电压范围，VCC=2.7~3.6V之间!!
				if(HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError)!=HAL_OK) 
				{
					break;//发生错误了	
				}
			}
			else
				addrx+=4;
				
			FLASH_WaitForLastOperation(FLASH_WAITETIME,FLASH_BANK_1);                //等待上次操作完成
		}
	}
	FlashStatus=FLASH_WaitForLastOperation(FLASH_WAITETIME,FLASH_BANK_1);            //等待上次操作完成
	if(FlashStatus==HAL_OK)
	{
		 while(WriteAddr<endaddr)//写数据
		 {
//			 FlashWriteData = (((uint32_t)(*pBuffer)) << 24) + (((uint32_t)(*(pBuffer+1))) << 16) + (((uint32_t)(*(pBuffer+2))) << 8) + (uint32_t)(*(pBuffer+3));
			FlashWriteData = *pBuffer;
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,WriteAddr,FlashWriteData)!=HAL_OK)//写入数据
			{ 
				break;	//写入异常
			}
			WriteAddr+=4;
			pBuffer++;
		}  
	}
	HAL_FLASH_Lock();           //上锁
} 

/********************************************************************
功能:下载程序文件
输入:无
输出:执行错误信息
*********************************************************************/
uint8 App_Link_BinFileDownLoad(void)
{
    uint32 i=START_TRY_TIMES;
    uint32 FileSize=0;
    uint32 PageNum=0;
    uint8 IndexTemp=0;
    uint8 FileTranOver=DATA_TRANS_NOT_OVER;
    uint32 DataAddress=PROGRAM_BASE_ADDR;
    
    uint8 *p_Data=(void*)0;
    uint8 DataLengthTemp=0;
	
    //请求升级 超时约 600000*0.17=102ms
    while(--i)
    {
        errormessage=App_Link_ReadFileSize(1000000,&FileSize);
        if(!errormessage)
            break;
    }
		
		SleepMs(100);
    //共请求START_TRY_TIMES=10次 约1s 都未响应 超时 退出升级
    if(errormessage)
        return TIME_OUT;
    
    //计算需要擦除Flash多少页
    PageNum=App_Link_FLASH_PagesMask(FileSize);
    if(PageNum>MAX_PAGE)
        return FILE_SIZE_OVERLOAD;
    
    //开始接收并读写
    IndexTemp=0x00;
    FileTranOver=DATA_TRANS_NOT_OVER;
    while(FileTranOver==DATA_TRANS_NOT_OVER)
    {
        //下载文件 超时约 600000*0.17=102ms
        p_Data=App_Link_ReadData(12000000,IndexTemp,&FileTranOver,&errormessage,&DataLengthTemp);

        if(errormessage)
        {
						HAL_FLASH_Lock();           //上锁
            return errormessage;
        }
				
				//STMFLASH_Write(DataAddress,p_Data,DataLengthTemp/4);
				//WriteEmbeddedFlash(DataAddress,p_Data,DataLengthTemp);
				
				if(((uint32_t)DataAddress == ADDR_FLASH_SECTOR_0_BANK1)||			\
					 ((uint32_t)DataAddress == ADDR_FLASH_SECTOR_1_BANK1)||
					 ((uint32_t)DataAddress == ADDR_FLASH_SECTOR_2_BANK1)||
					 ((uint32_t)DataAddress == ADDR_FLASH_SECTOR_3_BANK1)||
					 ((uint32_t)DataAddress == ADDR_FLASH_SECTOR_4_BANK1)||
					 ((uint32_t)DataAddress == ADDR_FLASH_SECTOR_5_BANK1))			\
								bsp_EraseCpuFlash(DataAddress);
				
				bsp_WriteCpuFlash(DataAddress,p_Data,DataLengthTemp);
				
				DataAddress=DataAddress+DataLengthTemp;

        IndexTemp++;
    }
    
    if(FileTranOver==DATA_TRANS_OVER)
    {
        App_Link_TxAckPacket(0x01,IndexTemp); //发送最后一个应答帧
    }
    
		HAL_FLASH_Lock();           //上锁
    return  RECEIVE_SUCCESS;
}

