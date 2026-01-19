#include "gd32f10x_usart.h"
#include "gd32f10x_gpio.h"
#include "gd32f10x_rcu.h"
#include "gd32f10x_it.h"
#include "gd32f10x_misc.h"
#include "gd32f10x.h"

#include "Driver_Uart.h"
//***************************************************
//函数名  :  DriverUartInitial
//功能说明： 初始化通信串口
//输入说明： Config:配置串口的相关信息
//           Com:具体的COM口
//返回值  ：无
//***************************************************
void DriverUartInitial(UartConfigInfo Config,DriverUartNum Com)
{
	//使能部件时钟，配置GPIO口
	if(Com==DRI_UART0)
	{
		if(Config.RxInterruptEnable==UART_RX_INTERRUPT_EN)
		{
			//注册中断函数
			IntVectSet(USART0_IRQn,Config.UartRxhandle);
			
			nvic_irq_enable(USART0_IRQn,Config.RxPreInterruptPrioty,Config.RxSubInterruptPrioty);
		}

    rcu_periph_clock_enable(Uart0TxIOBank); //时能IOBANK时钟
    rcu_periph_clock_enable(Uart0RxIOBank); //时能IOBANK时钟
		rcu_periph_clock_enable(RCU_AF); //时能IOBANK时钟		
		rcu_periph_clock_enable(RCU_USART0); 
		
		#if UART0_REMP==PART_REMP  //引脚重映射
		gpio_pin_remap_config(GPIO_USART0_REMAP,ENABLE);
		#endif
	
		gpio_init(Uart0TxPort,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,Uart0TxPin);
		gpio_init(Uart0RxPort,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,Uart0RxPin);
	}
	else if(Com==DRI_UART1)
	{
		if(Config.RxInterruptEnable==UART_RX_INTERRUPT_EN)
		{
			//注册中断函数
			IntVectSet(USART1_IRQn,Config.UartRxhandle);
			
			nvic_irq_enable(USART1_IRQn,Config.RxPreInterruptPrioty,Config.RxSubInterruptPrioty);
		}

    rcu_periph_clock_enable(Uart1TxIOBank); //时能IOBANK时钟
    rcu_periph_clock_enable(Uart1RxIOBank); //时能IOBANK时钟
		rcu_periph_clock_enable(RCU_AF); //时能IOBANK时钟		
		rcu_periph_clock_enable(RCU_USART1); 
		
		#if UART1_REMP==PART_REMP  //引脚重映射
		gpio_pin_remap_config(GPIO_USART1_REMAP,ENABLE);
		#endif
	
		gpio_init(Uart1TxPort,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,Uart1TxPin);
		gpio_init(Uart1RxPort,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,Uart1RxPin);
	}
	else if(Com==DRI_UART2)
	{
		if(Config.RxInterruptEnable==UART_RX_INTERRUPT_EN)
		{
			//注册中断函数
			IntVectSet(USART2_IRQn,Config.UartRxhandle);
			
			//中断使能
			nvic_irq_enable(USART2_IRQn,Config.RxPreInterruptPrioty,Config.RxSubInterruptPrioty);
		}
		
    rcu_periph_clock_enable(Uart2TxIOBank); //时能IOBANK时钟
    rcu_periph_clock_enable(Uart2RxIOBank); //时能IOBANK时钟
		rcu_periph_clock_enable(RCU_AF); //时能IOBANK时钟		
		rcu_periph_clock_enable(RCU_USART2); 
		
		#if UART2_REMP==PART_REMP  //引脚重映射
				gpio_pin_remap_config(GPIO_USART2_PARTIAL_REMAP,ENABLE);
		#endif
		
		#if UART2_REMP==ALL_REMP  //引脚重映射
				gpio_pin_remap_config(GPIO_USART2_FULL_REMAP,ENABLE);
		#endif
	
		gpio_init(Uart2TxPort,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,Uart2TxPin);
		gpio_init(Uart2RxPort,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,Uart2RxPin);
	}
	else if(Com==DRI_UART3)
	{
		if(Config.RxInterruptEnable==UART_RX_INTERRUPT_EN)
		{
			//注册中断函数
			IntVectSet(UART3_IRQn,Config.UartRxhandle);
			
			//中断使能
			nvic_irq_enable(UART3_IRQn,Config.RxPreInterruptPrioty,Config.RxSubInterruptPrioty);
		}
		
    rcu_periph_clock_enable(Uart3TxIOBank); //时能IOBANK时钟
    rcu_periph_clock_enable(Uart3RxIOBank); //时能IOBANK时钟
		rcu_periph_clock_enable(RCU_AF); //时能IOBANK时钟		
		rcu_periph_clock_enable(RCU_UART3); 
		
		
		gpio_init(Uart3TxPort,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,Uart3TxPin);
		gpio_init(Uart3RxPort,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,Uart3RxPin);
	}
	
#ifdef GD32F10X_HD
	else if(Com==DRI_UART4)
	{
		if(Config.RxInterruptEnable==UART_RX_INTERRUPT_EN)
		{
			//注册中断函数
			IntVectSet(UART4_IRQn,Config.UartRxhandle);
			
			//中断使能
			nvic_irq_enable(UART4_IRQn,Config.RxPreInterruptPrioty,Config.RxSubInterruptPrioty);
		}
		
    rcu_periph_clock_enable(Uart4TxIOBank); //时能IOBANK时钟
    rcu_periph_clock_enable(Uart4RxIOBank); //时能IOBANK时钟
		rcu_periph_clock_enable(RCU_AF); //时能IOBANK时钟		
		rcu_periph_clock_enable(RCU_UART4); 
		
		gpio_init(Uart4TxPort,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,Uart4TxPin);
		gpio_init(Uart4RxPort,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,Uart4RxPin); 
	}
#endif
	if(Com==DRI_UART0)
	{
		//配置串口参数
    usart_baudrate_set(USART0, Config.BandSpeed);
		usart_word_length_set(USART0,USART_WL_8BIT);
		usart_stop_bit_set(USART0,USART_STB_1BIT);
    usart_parity_config(USART0,USART_PM_NONE);
		usart_transmit_config(USART0,USART_TRANSMIT_ENABLE);
		usart_receive_config(USART0,USART_RECEIVE_ENABLE);
		
    //串口时钟配置
		usart_synchronous_clock_disable(USART0);
		usart_synchronous_clock_config(USART0,USART_CLEN_EN,USART_CPH_2CK,USART_CPL_LOW);

		if(Config.RxInterruptEnable==UART_RX_INTERRUPT_EN) //配置中断
			usart_interrupt_enable(USART0, USART_INT_RBNE);
		//Enable Uart
    usart_enable(USART0); 
	}
	else if(Com==DRI_UART1)
	{
		//配置串口参数
    usart_baudrate_set(USART1, Config.BandSpeed);
		usart_word_length_set(USART1,USART_WL_8BIT);
		usart_stop_bit_set(USART1,USART_STB_1BIT);
    usart_parity_config(USART1,USART_PM_NONE);
		usart_transmit_config(USART1,USART_TRANSMIT_ENABLE);
		usart_receive_config(USART1,USART_RECEIVE_ENABLE);
		
    //串口时钟配置
		usart_synchronous_clock_disable(USART1);
		usart_synchronous_clock_config(USART1,USART_CLEN_EN,USART_CPH_2CK,USART_CPL_LOW);

		if(Config.RxInterruptEnable==UART_RX_INTERRUPT_EN) //配置中断
			usart_interrupt_enable(USART1, USART_INT_RBNE);
		//Enable Uart
    usart_enable(USART1); 
	}
	else if(Com==DRI_UART2)
	{
		//配置串口参数
    usart_baudrate_set(USART2, Config.BandSpeed);
		usart_word_length_set(USART2,USART_WL_8BIT);
		usart_stop_bit_set(USART2,USART_STB_1BIT);
    usart_parity_config(USART2,USART_PM_NONE);
		usart_transmit_config(USART2,USART_TRANSMIT_ENABLE);
		usart_receive_config(USART2,USART_RECEIVE_ENABLE);
		
    //串口时钟配置
		usart_synchronous_clock_disable(USART2);
		usart_synchronous_clock_config(USART2,USART_CLEN_EN,USART_CPH_2CK,USART_CPL_LOW);

		if(Config.RxInterruptEnable==UART_RX_INTERRUPT_EN) //配置中断
			usart_interrupt_enable(USART2, USART_INT_RBNE);
		//Enable Uart
    usart_enable(USART2); 
	}
	else if(Com==DRI_UART3)
	{
		//配置串口参数
    usart_baudrate_set(UART3, Config.BandSpeed);
		usart_word_length_set(UART3,USART_WL_8BIT);
		usart_stop_bit_set(UART3,USART_STB_1BIT);
    usart_parity_config(UART3,USART_PM_NONE);
		usart_transmit_config(UART3,USART_TRANSMIT_ENABLE);
		usart_receive_config(UART3,USART_RECEIVE_ENABLE);
		
    //串口时钟配置
		usart_synchronous_clock_disable(UART3);
		usart_synchronous_clock_config(UART3,USART_CLEN_EN,USART_CPH_2CK,USART_CPL_LOW);

		if(Config.RxInterruptEnable==UART_RX_INTERRUPT_EN) //配置中断
			usart_interrupt_enable(UART3, USART_INT_RBNE);
		//Enable Uart
    usart_enable(UART3); 
	}
	
#ifdef GD32F10X_HD
	if(Com==DRI_UART4)
	{
		//配置串口参数
    usart_baudrate_set(UART4, Config.BandSpeed);
		usart_word_length_set(UART4,USART_WL_8BIT);
		usart_stop_bit_set(UART4,USART_STB_1BIT);
    usart_parity_config(UART4,USART_PM_NONE);
		usart_transmit_config(UART4,USART_TRANSMIT_ENABLE);
		usart_receive_config(UART4,USART_RECEIVE_ENABLE);
		
    //串口时钟配置
		usart_synchronous_clock_disable(UART4);
		usart_synchronous_clock_config(UART4,USART_CLEN_EN,USART_CPH_2CK,USART_CPL_LOW);

		if(Config.RxInterruptEnable==UART_RX_INTERRUPT_EN) //配置中断
			usart_interrupt_enable(UART4, USART_INT_RBNE);
		//Enable Uart
    usart_enable(UART4);
	}
#endif
}


//***************************************************
//函数名  :  DriverUartTxChar
//功能说明： UART发送一个字符
//输入说明： Com：哪一个UART
//           Txchar:待发送的字符
//***************************************************
void DriverUartTxChar(DriverUartNum Com,char Txchar)
{
	char *p_Str = NULL;
	p_Str = &Txchar;
	if(Com==DRI_UART0)
	{
		while (SET != usart_flag_get(USART0,USART_FLAG_TC));

		usart_data_transmit(USART0,(*p_Str & 0x1FF));
		
		while (SET != usart_flag_get(USART0,USART_FLAG_TC));
	}
	else if(Com==DRI_UART1)
	{
		while (SET != usart_flag_get(USART1,USART_FLAG_TC));

		usart_data_transmit(USART1,(*p_Str & 0x1FF));
		
		while (SET != usart_flag_get(USART1,USART_FLAG_TC));
	}
	else if(Com==DRI_UART2)
	{
		while (SET != usart_flag_get(USART2,USART_FLAG_TC));

		usart_data_transmit(USART2,(*p_Str & 0x1FF));
		
		while (SET != usart_flag_get(USART2,USART_FLAG_TC));
	}
	else if(Com==DRI_UART3)
	{
		while (SET != usart_flag_get(UART3,USART_FLAG_TC));

		usart_data_transmit(UART3,(*p_Str & 0x1FF));
		
		while (SET != usart_flag_get(UART3,USART_FLAG_TC));
	}
#ifdef GD32F10X_HD
	else if(Com==DRI_UART4)
	{
		while (SET != usart_flag_get(UART4,USART_FLAG_TC));

		usart_data_transmit(UART4,(*p_Str & 0x1FF));
		
		while (SET != usart_flag_get(UART4,USART_FLAG_TC));
	}
#endif
}






