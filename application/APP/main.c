
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "print.h"
#include "app_layer.h"
#include "bsp_layer.h"
#include "circular_queue.h"

/*
 * ============================================================================
 * Function	: main函数
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
int main(void)
{
	uint32_t i = 0;
	uint8_t data[10] = {0};
	BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */

	mcu_init();

	nvic_init();
	
	app_init();

	/* 创建AppTaskCreate任务 */
	xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate,		 /* 任务入口函数 */
						  (const char *)"AppTaskCreate",		 /* 任务名字 */
						  (uint16_t)512,						 /* 任务栈大小 */
						  (void *)NULL,							 /* 任务入口函数参数 */
						  (UBaseType_t)1,						 /* 任务的优先级 */
						  (TaskHandle_t *)&AppTaskCreateHandle); /* 任务控制块指针 */
	/* 启动任务调度 */
	if (pdPASS == xReturn)
		vTaskStartScheduler(); /* 启动任务，开启调度 */
	else
		return -1;

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
