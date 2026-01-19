#include <stdint.h>
#include <string.h>
#include "stm32h7xx_hal.h"
#include "stm32_hal_legacy.h"
#include "cmsis_armcc.h"
#include "App_TurnToApp.h"
typedef void (*iapfun)(void);//定义一个函数类型的参数.

/********************************************************************
功能:跳转地址
输入:无
输出:无
*********************************************************************/
void iap_load_app(uint32 app_addr)
{
		uint32_t i = 0;
	
		iapfun SysJump2Boot;

    /** STM32F4的系统BootLoader地址 */
    __IO uint32_t BootloaderAddr = app_addr; 

    /** 关闭全局中断 */
    __disable_irq();

    /** 关闭滴答定时器，复位到默认值 */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    /** 设置所有时钟到默认状态，使用HSI时钟 */    
    HAL_RCC_DeInit();

    /** 关闭所有中断，清除所有中断挂起标志 */
    for (i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    } 

    /** 使能全局中断 */
    __enable_irq();

    /** 
    * 重映射到系统flash
    * 将系统bootloader的地址映射到0x00000000
    */

 //   __HAL_REMAPMEMORY_SYSTEMFLASH();
    if(((*(vuint32*)BootloaderAddr)&0x2FF80000)==0x24000000)//检查栈顶地址是否合法.即检查代码是否已经下载
    { 
			/** 跳转到系统BootLoader，首地址是MSP，地址+4是复位中断服务程序地址 */
			SysJump2Boot=(iapfun)*(vuint32*)(BootloaderAddr+4);//用户代码区第二个字为程序开始地址(复位地址)
		
			/** 设置主堆栈指针 */
			__set_MSP(*(uint32_t *)BootloaderAddr);

			/** 如果使用了RTOS工程，需要用到这条语句，设置为特权级模式，使用MSP指针 */
			__set_CONTROL(0);

			/* 跳转到系统BootLoader */
			SysJump2Boot(); 
	}
}


