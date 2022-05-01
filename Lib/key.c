
#include <stdint.h>
#include <string.h>
#include "key.h"
#include "print.h"

#include "lib_port.h"

/*
 * ============================================================
 *	配置项
 * ============================================================
 */
#define KEY_NUM			 4
#define TAP_KEY_NUM		 4
#define CLICK_KEY_NUM	 4
#define KEY_SETTLE_TICKS 10 //消抖，按键电平稳定持续时间 10ms
#define KEY_SETTLE_TIME	 30 //按键状态稳定时间 30ms

#define MAX_WAIT_TIME		0xffffffff
#define KEY_LONG_PRESS_TIME 2000 //长按持续时间	2s
#define KEY_TIME_OUT		200	 //短按、双击、三击的间隔时间，超时认为是两次按键

#define TIME_TO_COUNT(time) ((time) / KEY_SETTLE_TICKS)
#define KEY_CLICK_INDEX		0
#define KEY_TAP_INDEX		1
#define KEY_INVALID_STATUS	2

#define is_enable(id, key)	 (sClickKey[id].enFlag & (0x0001 << (key)))
#define set_enable(id, key)	 (sClickKey[id].enFlag |= (0x0001 << (key)))
#define set_disable(id, key) (sClickKey[id].enFlag &= ~(0x01 << (key)))

typedef enum
{
	KEY_IDLE,
	KEY_WAIT_RELEASE1,
	KEY_WAIT_PRESS1,
	KEY_WAIT_RELEASE2,
	KEY_WAIT_PRESS2,
	KEY_WAIT_IDLE,
	KEY_STATE_SIZE,
} ClickState_t;

typedef union
{
	struct
	{
		uint8_t oldState : 2; //历史的稳定状态
		uint8_t newState : 2; //最新的临时状态
		uint8_t settle : 1;	  //稳定的时候
		uint8_t fun1 : 1;	  // true fun1功能使能
		uint8_t fun2 : 1;	  // true fun2功能使能
		uint8_t fun3 : 1;	  // true fun3功能使能
	};

	uint8_t value;
} Flag_t;

// 单机，双击
typedef struct
{
	uint8_t irqCount; //中断计数
	uint8_t idx[2];	  //指向功能索引
} Key_t;

typedef struct
{
	uint8_t irqCountBak; //处理时记录irqCount值
	ClickState_t state;	 //状态
	Flag_t flag;		 //标志位
	uint8_t id;			 //物理按键编号
	uint8_t keyType;	 //识别出来的按键类型

	uint8_t count;	  //电平改变之后的采样次数，用于消抖
	uint8_t cfgCount; //设置的消抖次数，

	uint16_t timeOut;	//单击、双击、三击按键的超时时间
	uint16_t longPress; //长按的超时时间
} KeyClick_t;

typedef struct
{
	uint8_t irqCountBak; //处理时记录irqCount值
	uint8_t count;		 //电平改变之后的采样次数，用于消抖
	Flag_t flag;		 //标志位
	uint8_t id;			 //物理按键编号

	uint16_t press;		// 按下的计数值
	uint16_t release;	// 松开的时间
	uint16_t period;	// 按下周期返回结果的时间
	uint32_t pressTime; // 按键按下的时刻
	uint16_t cfgPress;
	uint16_t cfgRelease;
	uint16_t cfgPeiod;
} KeyTap_t;

// 0 IO输入状态改变	 1 超时
const ClickState_t sClickJumpTab[KEY_STATE_SIZE][2] = {
	{KEY_WAIT_RELEASE1, KEY_IDLE},	  // KEY_IDLE
	{KEY_WAIT_PRESS1, KEY_WAIT_IDLE}, // KEY_WAIT_RELEASE1
	{KEY_WAIT_RELEASE2, KEY_IDLE},	  // KEY_WAIT_PRESS1
	{KEY_WAIT_PRESS2, KEY_WAIT_IDLE}, // KEY_WAIT_RELEASE2
	{KEY_WAIT_IDLE, KEY_IDLE},		  // KEY_WAIT_PRESS2
	{KEY_IDLE, KEY_WAIT_IDLE},		  // KEY_WAIT_IDLE
};

// 0 IO输入状态改变	 1 超时
const uint8_t sKeyClickTab[KEY_STATE_SIZE][2] = {
	{KEY_SETTLE, KEY_SETTLE},	  // KEY_IDLE
	{KEY_SETTLE, LONG_CLICK},	  // KEY_WAIT_RELEASE1
	{KEY_SETTLE, SHORT_CLICK},	  // KEY_WAIT_PRESS1
	{KEY_SETTLE, DOUBLE_CLICK},	  // KEY_WAIT_RELEASE2
	{THRICE_CLICK, DOUBLE_CLICK}, // KEY_WAIT_PRESS2
	{KEY_SETTLE, KEY_SETTLE},	  // KEY_WAIT_IDLE
};

static Key_t sKey[KEY_NUM];
static KeyClick_t sClickKey[CLICK_KEY_NUM];
static KeyTap_t sTapKey[TAP_KEY_NUM];

/*
 * ============================================================
 *	内部函数声明
 * ============================================================
 */
static uint8_t deal_key_status(uint8_t index, uint8_t event, uint32_t *pWaitTime);

/*
 * ============================================================================
 * Function	: 按键初始化
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void key_init(void)
{
	uint8_t i;

	memset(sKey, 0, sizeof(sKey));
	memset(sClickKey, 0, sizeof(sClickKey));
	memset(sTapKey, 0, sizeof(sTapKey));

	for (i = 0; i < KEY_NUM; i++)
	{
		sKey[i].idx[KEY_CLICK_INDEX] = 0xff;
		sKey[i].idx[KEY_TAP_INDEX] = 0xff;
	}
}

 /*
 * ============================================================================
 * Function	: 设置点击按键
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void set_click_key(uint8_t id, uint16_t time, uint16_t timeout, uint16_t longpress)
{
	uint8_t cid;

	for (cid = 0; cid < CLICK_KEY_NUM; cid++)
	{
		if (sClickKey[cid].id != 0xff)
		{
			break;
		}
	}

	if (cid < CLICK_KEY_NUM)
	{
		sKey[id].idx[KEY_CLICK_INDEX] = cid;

		sClickKey[cid].flag.newState = KEY_INVALID_STATUS;
		sClickKey[cid].flag.oldState = KEY_INVALID_STATUS;
		sClickKey[cid].flag.fun1 = longpress ? true : false;

		sClickKey[cid].id = id;

		sClickKey[cid].cfgCount = TIME_TO_COUNT(time);
		sClickKey[cid].timeOut = timeout;
		sClickKey[cid].longPress = longpress;
	}
	else
	{
		printf("click key init fail\r\n");
	}
}

/*
 * ============================================================
 * Name		: key_init
 * Function	: 按键初始化
 * Input		: None
 * Output	: None
 * Return	: None
 * ============================================================
 */
void set_tap_key(uint8_t id, uint16_t press, uint16_t release, uint16_t period)
{
	uint8_t idx;

	for (idx = 0; idx < TAP_KEY_NUM; idx++)
	{
		if (sTapKey[idx].id != 0xff)
		{
			break;
		}
	}

	if (idx < TAP_KEY_NUM)
	{
		sKey[id].idx[KEY_TAP_INDEX] = idx;

		sTapKey[idx].id = id;

		sTapKey[idx].cfgPress = press;
		sTapKey[idx].cfgRelease = release;
		sTapKey[idx].cfgPeiod = period;

		sTapKey[idx].flag.value = 0;
		sTapKey[idx].flag.newState = KEY_INVALID_STATUS;
		sTapKey[idx].flag.oldState = KEY_INVALID_STATUS;

		if (press || release)
		{
			sTapKey[idx].flag.fun1 = true;
			sTapKey[idx].flag.fun2 = true;
		}

		if (period)
		{
			sTapKey[idx].flag.fun3 = true;
		}
	}
	else
	{
		printf("tap key init fail\r\n");
	}
}

/*
 * ============================================================
 * Name		: increase_count
 * Function	: 依据中断信号，增加中断计数值
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================
 */
void increase_count(uint8_t id)
{
	sKey[id].irqCount++;
}



/*
* ============================================================
* Name		: deal_key_status
* Function	: 处理按键产生的事件
* Input		: uint8_t index 按键索引
			  uint8_t event	事件类型 	0 IO输入状态改变	1 超时
* Output	: uint32_t *pWaitTime	下次处理等待时间
* Return	: 按键类型
* ============================================================
*/
static uint8_t deal_key_status(uint8_t id, uint8_t event, uint32_t *pWaitTime)
{
	uint8_t keyType;
	uint8_t cid = sKey[id].idx[KEY_CLICK_INDEX];
	ClickState_t oldState = sClickKey[cid].state;

	
	keyType = sKeyClickTab[oldState][event];
	if (KEY_SETTLE != keyType)
	{
		sClickKey[cid].keyType = keyType;
	}

	sClickKey[cid].state = sClickJumpTab[oldState][event];
	if (KEY_WAIT_RELEASE1 == sClickKey[cid].state)
	{
		*pWaitTime = sClickKey[cid].longPress;
	}
	else
	{
		*pWaitTime = sClickKey[cid].timeOut;
	}

	if (KEY_IDLE == sClickKey[cid].state)
	{
		sClickKey[cid].count = sClickKey[cid].irqCountBak;
		*pWaitTime = MAX_WAIT_TIME;

		input_key_deal(id, sClickKey[cid].keyType);
		print(1, 1, "key %u type %u %u\r\n", id, sClickKey[cid].keyType, *pWaitTime);
		sClickKey[cid].keyType = KEY_SETTLE;
	}

	return keyType;
}

 /*
 * ============================================================================
 * Function	: 操作型按键输入处理，单击、双击、三击，长按
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void click_key_deal(uint8_t id, uint32_t *pTime)
{
	uint8_t state;
	uint8_t irqCount; //中断计数值缓存
	uint8_t cid;

	state = get_key_pressed(id); //读取按键状态
	cid = sKey[id].idx[KEY_CLICK_INDEX];
	irqCount = sKey[id].irqCount; //先缓存计数值，防止中断产生之后前后数据不一致

	// 消抖阶段,中断计数值发生变化，更新计数值
	if (irqCount != sClickKey[cid].irqCountBak)
	{
		sClickKey[cid].irqCountBak = irqCount;
		sClickKey[cid].flag.newState = KEY_INVALID_STATUS; //强制进行一次刷新
	}

	*pTime = KEY_SETTLE_TICKS;

	//检测电平是否发生转变
	if (state != sClickKey[cid].flag.newState)
	{
		sClickKey[cid].count = sClickKey[cid].cfgCount; //更新消抖计数值
		sClickKey[cid].flag.newState = state;
	}
	else if (sClickKey[cid].count)
	{
		sClickKey[cid].count--;
		if (0 == sClickKey[cid].count) //满足输入状态的稳定持续时间
		{
			if (sClickKey[cid].flag.oldState != sClickKey[cid].flag.newState)
			{
				deal_key_status(id, 0, pTime);
				sClickKey[cid].flag.oldState = sClickKey[cid].flag.newState;
			}
		}
	}
	else
	{
		deal_key_status(id, 1, pTime);
	}
}


 /*
 * ============================================================================
 * Function	: 状态型按键输入处理，按下、弹开
 * Input	: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void tap_key_deal(uint8_t id, uint32_t *pTime)
{
	uint8_t state;
	uint8_t irqCount; //中断计数值缓存
	uint8_t cid;

	state = get_key_pressed(id); //读取按键状态
	cid = sKey[id].idx[KEY_CLICK_INDEX];
	irqCount = sKey[id].irqCount; //先缓存计数值，防止中断产生之后前后数据不一致

	// 消抖阶段,中断计数值发生变化，更新计数值
	if (irqCount != sClickKey[cid].irqCountBak)
	{
		sClickKey[cid].irqCountBak = irqCount;
		sClickKey[cid].flag.newState = KEY_INVALID_STATUS; //强制进行一次刷新
	}

	*pTime = KEY_SETTLE_TICKS;

	//检测电平是否发生转变
	if (state != sClickKey[cid].flag.newState)
	{
		sClickKey[cid].count = sClickKey[cid].cfgCount; //更新消抖计数值
		sClickKey[cid].flag.newState = state;
	}
	else if (sClickKey[cid].count)
	{
		sClickKey[cid].count--;
		if (0 == sClickKey[cid].count) //满足输入状态的稳定持续时间
		{
			if (sClickKey[cid].flag.oldState != sClickKey[cid].flag.newState)
			{

				sClickKey[cid].flag.oldState = sClickKey[cid].flag.newState;
			}
		}
	}
	else
	{
		
	}
}


#if 0


/*
 * ============================================================
 * Name		: gpio_irq_handle
 * Function	: GPIO中断函数处理
 * Input		: None
 * Output	: None
 * Return	: None
 * ============================================================
 */
void gpio_irq_handle(uint32_t index)
{
	BaseType_t xHigherPriorityTaskWoken;

	if (KeyTaskHandle != NULL)
	{
		// index = 0x01 << index;
		xTaskNotifyFromISR(KeyTaskHandle, index, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //如果需要的话进行一次任务切换
	}
}



static void Key_Task(void *parameter)
{
	uint32_t NotifyValue;
	uint32_t waitTime = KEY_SCAN_TICKS;

	key_init();

	while (1)
	{
		//进入函数的时候不清除任务bit
		//退出函数的时候清除所有的bit
		//保存任务通知值
		if (xTaskNotifyWait(0, ULONG_MAX, &NotifyValue, waitTime))
		{
			increase_count(NotifyValue);
			input_change_deal(&waitTime);
		}
		else
		{
			time_out_deal(&waitTime);
		}
	}
}

#endif