
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "print.h"
#include "app_layer.h"
#include "bsp_layer.h"
#include "circular_queue.h"

/*
 * ============================================================================
 * Function	: main����
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
int main(void)
{
	uint32_t i = 0;
	uint8_t data[10] = {0};
	BaseType_t xReturn = pdPASS; /* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */

	mcu_init();

	nvic_init();
	
	app_init();

	/* ����AppTaskCreate���� */
	xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate,		 /* ������ں��� */
						  (const char *)"AppTaskCreate",		 /* �������� */
						  (uint16_t)512,						 /* ����ջ��С */
						  (void *)NULL,							 /* ������ں������� */
						  (UBaseType_t)1,						 /* ��������ȼ� */
						  (TaskHandle_t *)&AppTaskCreateHandle); /* ������ƿ�ָ�� */
	/* ����������� */
	if (pdPASS == xReturn)
		vTaskStartScheduler(); /* �������񣬿������� */
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
