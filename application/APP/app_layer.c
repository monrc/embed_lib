#include "app_layer.h"
#include "bsp_layer.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "main.h"
#include "key.h"
#include "lib_port.h"



/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void Led_Task(void *pvParameters); 		/* LED_Task����ʵ�� */
static void Uart_Task(void *pvParameters); 		/* ���ڵ�������ʵ�� */
static void Test_Task(void *parameter);
static void Test1_Task(void *parameter);
static void led_timer_callback(xTimerHandle timer);
static void creat_key_task(void);


/**************************** ������ ********************************/
TaskHandle_t AppTaskCreateHandle = NULL; /* ���������� */

TaskHandle_t eepromTaskHandle = NULL;

TaskHandle_t keyTask[4];
KeyTaskParamter_t keyPara[4];

TaskHandle_t TestTaskHandle = NULL;
TaskHandle_t Test1TaskHandle = NULL;

TimerHandle_t LedTimerHandle = NULL;	//���ڶ�ʱ�����


/*
 * ============================================================================
 * Function	: Ӧ�ó�ʼ��
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void app_init(void)
{
	key_init();
	debug_init();
}


 /*********************************************************
* Name		: AppTaskCreate
* Function	: Ϊ�˷���������е����񴴽����������������������
* Input		: None
* Output	: None
* Return	: None
*********************************************************/
void AppTaskCreate(void *pvParameters)
{
	BaseType_t xReturn = pdPASS; /* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */

	taskENTER_CRITICAL(); //�����ٽ���

	// ����LED�ƿ�����
	xTaskCreate(led_task, "leds", 256, NULL, 8, NULL);

	// ������������
	creat_key_task();

	// �������Դ�������
	xTaskCreate(terminal_task, "terminal", 1024, NULL, 3, NULL);

	// �������Դ�������
	xTaskCreate(eeprom_task, "eeprom", 512, NULL, 16, &eepromTaskHandle);
	
	/* ���� Test_Task ���� */
	xReturn = xTaskCreate((TaskFunction_t)Test_Task,	/* ������ں��� */
						  (const char *)"Test_Task",	/* �������� */
						  (uint16_t)512,					 /* ����ջ��С */
						  (void *)NULL,						 /* ������ں������� */
						  (UBaseType_t)1,					 /* ��������ȼ� */
						  (TaskHandle_t *)&TestTaskHandle); /* ������ƿ�ָ�� */
	if (pdPASS == xReturn)
	{
		debug("creat Test_Task succeed!\r\n");
	}

	// /* ���� Test1_Task ���� */
	// xReturn = xTaskCreate((TaskFunction_t)Test1_Task,	/* ������ں��� */
	// 					  (const char *)"Test1_Task",	/* �������� */
	// 					  (uint16_t)512,					 /* ����ջ��С */
	// 					  (void *)NULL,						 /* ������ں������� */
	// 					  (UBaseType_t)1,					 /* ��������ȼ� */
	// 					  (TaskHandle_t *)&Test1TaskHandle); /* ������ƿ�ָ�� */
	// if (pdPASS == xReturn)
	// {
	// 	debug("creat Test1_Task succeed!\r\n");
	// }


	//����������ڶ�ʱ��
	LedTimerHandle = xTimerCreate((const char *)"LedTimer",
										  (TickType_t)500,
										  (UBaseType_t)pdTRUE,
										  (void *)1,
										  (TimerCallbackFunction_t)led_timer_callback); //���ڶ�ʱ��������500ms(500��ʱ�ӽ���)������ģʽ
	xTimerStart(LedTimerHandle, 100);
	
	vTaskDelete(AppTaskCreateHandle); //ɾ��AppTaskCreate����

	taskEXIT_CRITICAL(); //�˳��ٽ���
}

/*********************************************************
* Name		: Led_Task
* Function	: LED��˸ģ��
*********************************************************/
static void Led_Task(void *parameter)
{
	while (1)
	{
		vTaskDelay(500); /* ��ʱ500��tick */
		HAL_GPIO_TogglePin(LED1_DS1_GPIO_Port, LED1_DS1_Pin);
		iwdg_refresh();
	}
}

/*********************************************************
* Name		: Uart_Task
* Function	: �ն�ģ�������ʼ��
*********************************************************/
static void Uart_Task(void *parameter)
{
    uint8_t uartInput;

    // terminal_init(uart2_output);
	// init_print();
   
    // gUartQueueHandle = xQueueCreate(10, 1); //������ϢMessage_Queue,��������Ǵ��ڽ��ջ���������
	
	// while (1)
	// {
	// 	if (xQueueReceive(gUartQueueHandle, &uartInput, portMAX_DELAY))
	// 	{
    //         terminal_input_predeal(uartInput);
	// 	}
        
    //     terminal_handler();
	// }
}



static void creat_key_task(void)
{
	uint8_t i;
	char name[] = "key ";

	for (i = 0; i < 4; i++)
	{
		keyPara[i].id = i;
		keyPara[i].scanTime = 30;
		keyPara[i].timeOut = 300;
		keyPara[i].longPress = 2000;
		name[3] = i + '0';

		xTaskCreate(key_task, name, 512, &keyPara[i], 3, &keyTask[i]);
	}
}

/*
* ============================================================
* Name		: Test_Task
* Function	: ���Ե�������
* Input		: None
* Output	: None
* Return	: None
* ============================================================
*/
static void Test_Task(void *parameter)
{
	while (1)
	{
		vTaskDelay(500); /* ��ʱ500��tick */
		iwdg_refresh();
	}
}

/*
* ============================================================
* Name		: Test1_Task
* Function	: ���Ե�������
* Input		: None
* Output	: None
* Return	: None
* ============================================================
*/
static void Test1_Task(void *parameter)
{
	while (1)
	{
		taskYIELD();
	}
}

/*
* ============================================================
* Name		: led_timer_callback
* Function	: ��ʱ�����ڻص�����
* Input		: None
* Output	: None
* Return	: None
* ============================================================
*/
static void led_timer_callback(xTimerHandle timer)
{
	static uint8_t ledState = 0;

	if (ledState)
	{
		//led_control(GREEN_LED, LED_ON);
		ledState = 0;
	}
	else
	{
		//led_control(GREEN_LED, LED_OFF);
		ledState = 1;
	}
}

#if 0
void input_key_deal(uint8_t key, uint8_t type)
{
	switch (key)
	{
	case 0:
		key0_process(type);
		break;
	case 1:
		key1_process(type);
		break;
	case 2:
		key2_process(type);
		break;
	case 3:
		keyUp_process(type);
		break;
	default:
		break;
	}
}


static void key0_process(uint8_t type)
{
	TickType_t period = 0;
	switch (type)
	{
	case SHORT_CLICK:
		period = xTimerGetPeriod(LedTimerHandle);
		xTimerChangePeriod(LedTimerHandle, period + 100, 0);
		break;
	case DOUBLE_CLICK:
		period = xTimerGetPeriod(LedTimerHandle);
		xTimerChangePeriod(LedTimerHandle, period - 100, 0);
		break;
	case THRICE_CLICK:
		xTimerStart(LedTimerHandle, 0);	//�������ڶ�ʱ��
		break;
	case LONG_PRESS:
		xTimerStop(LedTimerHandle, 0);
		break;
	default:
		break;
	}
}

static void key1_process(uint8_t type)
{
	switch (type)
	{
	case SHORT_CLICK:

		break;
	case DOUBLE_CLICK:

		break;
	case THRICE_CLICK:

		break;
	case LONG_PRESS:

		break;	
	default:
		break;
	}	
}

static void key2_process(uint8_t type)
{
	switch (type)
	{
	case SHORT_CLICK:

		break;
	case DOUBLE_CLICK:

		break;
	case THRICE_CLICK:

		break;
	case LONG_PRESS:

		break;	
	default:
		break;
	}	
}

static void keyUp_process(uint8_t type)
{
	switch (type)
	{
	case SHORT_CLICK:

		break;
	case DOUBLE_CLICK:

		break;
	case THRICE_CLICK:

		break;
	case LONG_PRESS:

		break;	
	default:
		break;
	}	
}
#endif

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	print_wait("Task %s stack overflow\r\n", pcTaskName);
}
