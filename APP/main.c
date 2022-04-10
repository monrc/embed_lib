
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "print.h"
#include "bsp_layer.h"
#include "circular_queue.h"





int main(void)
{
	uint32_t i = 0;
	uint8_t data[10];
	uint8_t read;
	mcu_init();
	nvic_init();

	debug_init();

	while (1)
	{
		iwdg_refresh();
		HAL_Delay(500);
		HAL_GPIO_TogglePin(LED1_DS1_GPIO_Port, LED1_DS1_Pin);
		print_array(1, "abcdef", data, 10);

		debug_error("count %u\r\n", i++);
	}
	return 1;
}
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	printf("Task %s stack overflow\r\n", pcTaskName);
}