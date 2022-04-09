
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "print.h"
#include "bsp_layer.h"
#include "circular_queue.h"

CircleQueue_t test;
uint8_t testBuff[QUEUE_SIZE(1, 100)];

int main(void)
{
	uint32_t i = 0;
	uint8_t data[10];
	uint8_t byte;
	mcu_init();
	nvic_init();

	debug_init();

	creat_queue(&test, 1, testBuff);

	for (i = 0; i < 10; i++)
	{
		data[i] = i;
	}

	while (0)
	{
		iwdg_refresh();
		HAL_GPIO_TogglePin(LED1_DS1_GPIO_Port, LED1_DS1_Pin);
		en_queue_bytes(&test, data, 10);

		for (i = 0; i < 10; i++)
		{
			if (de_queue(&test, &byte))
			{
				if (byte != i)
				{
					debug("error %u, %u", byte, i);
				}
			}
			else
			{
				debug("lost %u\r\n", i);
			}
		}
		//HAL_Delay(500);
	}

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