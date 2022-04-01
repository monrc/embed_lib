
#include "mcu_init.h"
#include "stm32f1xx_it.h"

extern void timer_irq_callback(void);

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
	/* USER CODE BEGIN NonMaskableInt_IRQn 0 */
	printf("NMI_Handler\r\n");
	/* USER CODE END NonMaskableInt_IRQn 0 */
	HAL_RCC_NMI_IRQHandler();
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
	/* USER CODE BEGIN HardFault_IRQn 0 */
	printf("HardFault_Handler\r\n");
	/* USER CODE END HardFault_IRQn 0 */
	while (1)
	{
		/* USER CODE BEGIN W1_HardFault_IRQn 0 */
		/* USER CODE END W1_HardFault_IRQn 0 */
	}
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
	/* USER CODE BEGIN MemoryManagement_IRQn 0 */
	printf("MemManage_Handler\r\n");
	/* USER CODE END MemoryManagement_IRQn 0 */
	while (1)
	{
	}
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
	/* USER CODE BEGIN BusFault_IRQn 0 */
	printf("UsageFault_Handler\r\n");
	/* USER CODE END BusFault_IRQn 0 */
	while (1)
	{
	}
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
	/* USER CODE BEGIN UsageFault_IRQn 0 */
	printf("UsageFault_Handler\r\n");
	/* USER CODE END UsageFault_IRQn 0 */
	while (1)
	{
	}
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
	
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
	
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
	
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/
/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
	uint8_t res;
	//HAL_UART_IRQHandler(&huart1);
	//res = USART1->DR & (uint16_t)0x01FF;
	//uart1_irq_callback();
	
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
	uint8_t res;
	//HAL_UART_IRQHandler(&huart2);
	//res = USART2->DR & (uint16_t)0x01FF;
}

/**
  * @brief This function handles TIM6 global interrupt.
  */
void TIM6_IRQHandler(void)
{	
	if(READ_BIT(TIM6->SR, TIM_IT_UPDATE) != TIM_IT_UPDATE)
		return;
	//timer_irq_callback();
	
	CLEAR_BIT(TIM6->SR, TIM_IT_UPDATE);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
