#include "TypesDef.h"
#include "Driver_Sleep.h"

#include "stm32h7xx_hal.h"
#include "ota.h"

#include "App_Usart.h"
#include "App_Link.h"
#include "App_TurnToApp.h"



uint16 g_Rs485Add = 0;
uint16 GetBoardAddress(void);				


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
/**************************************************
功能:主函数，不需调用
输入:无
输出:无
***************************************************/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 12;
  PeriphClkInitStruct.PLL2.PLL2P = 1;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x38000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

int main(void)
{
    SleepMs(200);//等待上电完成
	
	/* MPU Configuration--------------------------------------------------------*/
	MPU_Config();

	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

//		/* Enable D-Cache---------------------------------------------------------*/
//		SCB_EnableDCache();

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Configure the peripherals common clocks */
	PeriphCommonClock_Config();

	SystemInit();
//指令周期 12*1/72M=0.17us
	
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	//接收ID地址
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Pin = A0_Pin;
	HAL_GPIO_Init(A0_GPIO_Port, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = A1_Pin;
	HAL_GPIO_Init(A1_GPIO_Port, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = A2_Pin;
	HAL_GPIO_Init(A2_GPIO_Port, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = A3_Pin;
	HAL_GPIO_Init(A3_GPIO_Port, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = A4_Pin;
	HAL_GPIO_Init(A4_GPIO_Port, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = A5_Pin;
	HAL_GPIO_Init(A5_GPIO_Port, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = A6_Pin;
	HAL_GPIO_Init(A6_GPIO_Port, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = A7_Pin;
	HAL_GPIO_Init(A7_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Pin = RS485_CONTROL_PIN;
	HAL_GPIO_Init(RS485_CONTROL_PORT, &GPIO_InitStruct);
	RS485_READ_HANDLE

	g_Rs485Add = GetBoardAddress();
    //******************使用注意***************//
    //*1.首先根据自己的原理图，在 McUart.h 文件里 [配置UART的引脚映射]
    //*2.然后调用 App_Usart_Init 函数
    App_Usart_Init(115200,0);
    
    while(1)
    {
        App_Link_BinFileDownLoad();
        App_Usart_Reset();
			
		//OTA 代码升级转移写入
		OTA_CheckAndSwap_Early();
			
        iap_load_app(PROGRAM_BASE_ADDR);
        while(1);
    }
    
    //return 0;
}

uint16 GetBoardAddress(void)
{
    uint16_t addr = 0;
	
	addr = ((uint16_t)HAL_GPIO_ReadPin(A7_GPIO_Port,A7_Pin)<<7|
			(uint16_t)HAL_GPIO_ReadPin(A6_GPIO_Port,A6_Pin)<<6|
			(uint16_t)HAL_GPIO_ReadPin(A5_GPIO_Port,A5_Pin)<<5|
			(uint16_t)HAL_GPIO_ReadPin(A4_GPIO_Port,A4_Pin)<<4|
			(uint16_t)HAL_GPIO_ReadPin(A3_GPIO_Port,A3_Pin)<<3|
			(uint16_t)HAL_GPIO_ReadPin(A2_GPIO_Port,A2_Pin)<<2|
			(uint16_t)HAL_GPIO_ReadPin(A1_GPIO_Port,A1_Pin)<<1|
			(uint16_t)HAL_GPIO_ReadPin(A0_GPIO_Port,A0_Pin));
	
	addr = addr + 1;
	
    return addr;
}


void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */

  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}


