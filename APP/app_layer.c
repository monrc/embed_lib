#include "app_layer.h"
#include "bsp_layer.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "main.h"
#include "key.h"

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void Led_Task(void *pvParameters); 		/* LED_Task任务实现 */
static void Uart_Task(void *pvParameters); 		/* 串口调试任务实现 */
static void Key_Task(void *parameter);			/* 按键扫描实现 */
static void Test_Task(void *parameter);
static void Test1_Task(void *parameter);
static void led_timer_callback(xTimerHandle timer);


static void key0_process(uint8_t type);
static void key1_process(uint8_t type);
static void key2_process(uint8_t type);
static void keyUp_process(uint8_t type);


/**************************** 任务句柄 ********************************/
TaskHandle_t AppTaskCreateHandle = NULL; /* 创建任务句柄 */

TaskHandle_t LedTaskHandle = NULL;
TaskHandle_t KeyTaskHandle = NULL;
TaskHandle_t TerminalTaskHandle = NULL;
TaskHandle_t TestTaskHandle = NULL;
TaskHandle_t Test1TaskHandle = NULL;


TimerHandle_t LedTimerHandle = NULL;	//周期定时器句柄



/**************************** 队列句柄 ********************************/



void app_init(void)
{
	key_init();
	debug_init();
}


 /*********************************************************
* Name		: AppTaskCreate
* Function	: 为了方便管理，所有的任务创建函数都放在这个函数里面
* Input		: None
* Output	: None
* Return	: None
*********************************************************/
void AppTaskCreate(void *pvParameters)
{
	BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */

	taskENTER_CRITICAL(); //进入临界区

	/* 创建 Led_Task 任务 */
	// xReturn = xTaskCreate((TaskFunction_t)Led_Task,			 /* 任务入口函数 */
	// 					  (const char *)"Led_Task",			 /* 任务名字 */
	// 					  (uint16_t)512,					 /* 任务栈大小 */
	// 					  (void *)NULL,						 /* 任务入口函数参数 */
	// 					  (UBaseType_t)2,					 /* 任务的优先级 */
	// 					  (TaskHandle_t *)&LedTaskHandle); /* 任务控制块指针 */
	// if (pdPASS == xReturn)
	// {
	// 	debug("creat Led_Task succeed!\r\n");
	// }
	
	/* 创建 Key_Task 任务 */
	xReturn = xTaskCreate((TaskFunction_t)Key_Task,			 /* 任务入口函数 */
						  (const char *)"Key_Task",			 /* 任务名字 */
						  (uint16_t)512,					 /* 任务栈大小 */
						  (void *)NULL,						 /* 任务入口函数参数 */
						  (UBaseType_t)3,					 /* 任务的优先级 */
						  (TaskHandle_t *)&KeyTaskHandle); /* 任务控制块指针 */
	if (pdPASS == xReturn)
	{
		debug("creat KEY_Task succeed!\r\n");
	}
	
	// /* 创建 Uart_Task 任务 */
	// xReturn = xTaskCreate((TaskFunction_t)Uart_Task,	/* 任务入口函数 */
	// 					  (const char *)"Uart_Task",	/* 任务名字 */
	// 					  (uint16_t)512,					 /* 任务栈大小 */
	// 					  (void *)NULL,						 /* 任务入口函数参数 */
	// 					  (UBaseType_t)16,					 /* 任务的优先级 */
	// 					  (TaskHandle_t *)&TerminalTaskHandle); /* 任务控制块指针 */
	// if (pdPASS == xReturn)
	// {
	// 	debug("creat Uart_Task succeed!\r\n");
	// }

	/* 创建 Test_Task 任务 */
	xReturn = xTaskCreate((TaskFunction_t)Test_Task,	/* 任务入口函数 */
						  (const char *)"Test_Task",	/* 任务名字 */
						  (uint16_t)512,					 /* 任务栈大小 */
						  (void *)NULL,						 /* 任务入口函数参数 */
						  (UBaseType_t)1,					 /* 任务的优先级 */
						  (TaskHandle_t *)&TestTaskHandle); /* 任务控制块指针 */
	if (pdPASS == xReturn)
	{
		debug("creat Test_Task succeed!\r\n");
	}

	// /* 创建 Test1_Task 任务 */
	// xReturn = xTaskCreate((TaskFunction_t)Test1_Task,	/* 任务入口函数 */
	// 					  (const char *)"Test1_Task",	/* 任务名字 */
	// 					  (uint16_t)512,					 /* 任务栈大小 */
	// 					  (void *)NULL,						 /* 任务入口函数参数 */
	// 					  (UBaseType_t)1,					 /* 任务的优先级 */
	// 					  (TaskHandle_t *)&Test1TaskHandle); /* 任务控制块指针 */
	// if (pdPASS == xReturn)
	// {
	// 	debug("creat Test1_Task succeed!\r\n");
	// }


	//创建软件周期定时器
	LedTimerHandle = xTimerCreate((const char *)"LedTimer",
										  (TickType_t)500,
										  (UBaseType_t)pdTRUE,
										  (void *)1,
										  (TimerCallbackFunction_t)led_timer_callback); //周期定时器，周期500ms(500个时钟节拍)，周期模式
	xTimerStart(LedTimerHandle, 100);
	
	vTaskDelete(AppTaskCreateHandle); //删除AppTaskCreate任务

	taskEXIT_CRITICAL(); //退出临界区
}

/*********************************************************
* Name		: Led_Task
* Function	: LED闪烁模块
*********************************************************/
static void Led_Task(void *parameter)
{
	uint8_t ledState = 0;

	while (1)
	{
		vTaskDelay(500); /* 延时500个tick */
		HAL_GPIO_TogglePin(LED1_DS1_GPIO_Port, LED1_DS1_Pin);
		iwdg_refresh();
	}
}

/*********************************************************
* Name		: Uart_Task
* Function	: 终端模块参数初始化
*********************************************************/
static void Uart_Task(void *parameter)
{
    uint8_t uartInput;

    // terminal_init(uart2_output);
	// init_print();
   
    // gUartQueueHandle = xQueueCreate(10, 1); //创建消息Message_Queue,队列项长度是串口接收缓冲区长度
	
	// while (1)
	// {
	// 	if (xQueueReceive(gUartQueueHandle, &uartInput, portMAX_DELAY))
	// 	{
    //         terminal_input_predeal(uartInput);
	// 	}
        
    //     terminal_handler();
	// }
}

/*
* ============================================================
* Name		: Key_Task
* Function	: 按键任务函数
* Input		: None
* Output	: None
* Return	: None
* ============================================================
*/
static void Key_Task(void *parameter)
{
	uint32_t NotifyValue;
	uint32_t waitTime = 10;

	set_click_key(0, 30, 300, 2000);
	
	while (1)
	{
		//进入函数的时候不清除任务bit
		//退出函数的时候清除所有的bit
		//保存任务通知值
		if (xTaskNotifyWait(0, ULONG_MAX, &NotifyValue, waitTime))
		{
			increase_count(NotifyValue);
		}

		click_key_deal(0, &waitTime);
	}
}


/*
* ============================================================
* Name		: Test_Task
* Function	: 测试调试任务
* Input		: None
* Output	: None
* Return	: None
* ============================================================
*/
static void Test_Task(void *parameter)
{
	while (1)
	{
		vTaskDelay(500); /* 延时500个tick */
		debug("123456789\r\n");
		iwdg_refresh();
	}
}

/*
* ============================================================
* Name		: Test1_Task
* Function	: 测试调试任务
* Input		: None
* Output	: None
* Return	: None
* ============================================================
*/
static void Test1_Task(void *parameter)
{
	while (1)
	{
		debug("123456789\r\n");
		taskYIELD();
	}
}

/*
* ============================================================
* Name		: led_timer_callback
* Function	: 定时器周期回调函数
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
		xTimerStart(LedTimerHandle, 0);	//开启周期定时器
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
