

#include "main.h"
#include "bsp_layer.h"


void nvic_init(void)
{
	HAL_NVIC_SetPriority(USART1_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	HAL_NVIC_SetPriority(EXTI0_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void iwdg_refresh(void)
{
	HAL_IWDG_Refresh(&hiwdg);
}

// hard fault handler in C,
// with stack frame location as input parameter
void hard_fault_handler_c(unsigned int *hardfault_args)
{
	unsigned int stacked_r0;
	unsigned int stacked_r1;
	unsigned int stacked_r2;
	unsigned int stacked_r3;
	unsigned int stacked_r12;
	unsigned int stacked_lr;
	unsigned int stacked_pc;
	unsigned int stacked_psr;
	stacked_r0 = ((unsigned int)hardfault_args[0]);
	stacked_r1 = ((unsigned int)hardfault_args[1]);
	stacked_r2 = ((unsigned int)hardfault_args[2]);
	stacked_r3 = ((unsigned int)hardfault_args[3]);
	stacked_r12 = ((unsigned int)hardfault_args[4]);
	stacked_lr = ((unsigned int)hardfault_args[5]);
	stacked_pc = ((unsigned int)hardfault_args[6]);
	stacked_psr = ((unsigned int)hardfault_args[7]);
	printf("\n\n[Hard faulthandler - all numbers in hex]\n");
	printf("R0 = %x\n", stacked_r0);
	printf("R1 = %x\n", stacked_r1);
	printf("R2 = %x\n", stacked_r2);
	printf("R3 = %x\n", stacked_r3);
	printf("R12 = %x\n", stacked_r12);
	printf("LR [R14] = %x  subroutine call return address\n", stacked_lr);
	printf("PC [R15] = %x  program counter\n", stacked_pc);
	printf("PSR = %x\n", stacked_psr);
}
