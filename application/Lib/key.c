
#include <stdint.h>
#include <string.h>
#include "key.h"
#include "print.h"

#include "lib_port.h"

/*
 * ============================================================
 *	������
 * ============================================================
 */
#define KEY_NUM			 4
#define TAP_KEY_NUM		 4
#define CLICK_KEY_NUM	 4
#define KEY_SETTLE_TICKS 10 //���������������ȶ�����ʱ�� 10ms
#define KEY_SETTLE_TIME	 30 //����״̬�ȶ�ʱ�� 30ms

#define MAX_WAIT_TIME		0xffffffff
#define KEY_LONG_PRESS_TIME 2000 //��������ʱ��	2s
#define KEY_TIME_OUT		200	 //�̰���˫���������ļ��ʱ�䣬��ʱ��Ϊ�����ΰ���

#define TIME_TO_COUNT(time) ((time) / KEY_SETTLE_TICKS)
#define KEY_CLICK_INDEX		0
#define KEY_TAP_INDEX		1
#define KEY_RELEASE			0
#define KEY_PRESS			1
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
		uint8_t oldState : 2; //��ʷ���ȶ�״̬
		uint8_t newState : 2; //���µ���ʱ״̬
		uint8_t fun1 : 1;	  //�ȶ���ʱ��
		uint8_t fun2 : 1;	  // true fun1����ʹ��
		uint8_t fun3 : 1;	  // true fun2����ʹ��
		uint8_t fun4 : 1;	  // true fun3����ʹ��
	};

	uint8_t value;
} Flag_t;

// ������˫��
typedef struct
{
	uint8_t irqCount; //�жϼ���
	uint8_t idx[2];	  //ָ��������
} Key_t;

typedef struct
{
	uint8_t irqCountBak; //����ʱ��¼irqCountֵ
	ClickState_t state;	 //״̬
	Flag_t flag;		 //��־λ
	uint8_t id;			 //���������
	uint8_t keyType;	 //ʶ������İ�������

	uint8_t count;	  //����ı�֮��Ĳ�����������������
	uint8_t cfgCount; //���õ�����������

	uint16_t timeOut;	//������˫�������������ĳ�ʱʱ��
	uint16_t longPress; //�����ĳ�ʱʱ��
} KeyClick_t;

typedef struct
{
	uint8_t irqCountBak; //����ʱ��¼irqCountֵ
	uint8_t count;		 //����ı�֮��Ĳ�����������������
	Flag_t flag;		 //��־λ
	uint8_t id;			 //���������

	uint32_t pressTime; // �������µ�ʱ��
	uint16_t cfgPress;
	uint16_t cfgRelease;
	uint16_t cfgPeiod;
} KeyTap_t;

// 0 IO����״̬�ı�	 1 ��ʱ
const ClickState_t sClickJumpTab[KEY_STATE_SIZE][2] = {
	{KEY_WAIT_RELEASE1, KEY_IDLE},	  // KEY_IDLE
	{KEY_WAIT_PRESS1, KEY_WAIT_IDLE}, // KEY_WAIT_RELEASE1
	{KEY_WAIT_RELEASE2, KEY_IDLE},	  // KEY_WAIT_PRESS1
	{KEY_WAIT_PRESS2, KEY_WAIT_IDLE}, // KEY_WAIT_RELEASE2
	{KEY_WAIT_IDLE, KEY_IDLE},		  // KEY_WAIT_PRESS2
	{KEY_IDLE, KEY_WAIT_IDLE},		  // KEY_WAIT_IDLE
};

// 0 IO����״̬�ı�	 1 ��ʱ
const uint8_t sTypeTab[KEY_STATE_SIZE][2] = {
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
 *	�ڲ���������
 * ============================================================
 */
static uint8_t deal_key_status(uint8_t index, uint8_t event, uint32_t *pWaitTime);

/*
 * ============================================================================
 * Function	: ������ʼ��
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

	for (i = 0; i < TAP_KEY_NUM; i++)
	{
		sTapKey[i].id = 0xff;
		sTapKey[i].flag.newState = KEY_INVALID_STATUS;
	}

	for (i = 0; i < CLICK_KEY_NUM; i++)
	{
		sClickKey[i].id = 0xff;
		sClickKey[i].flag.newState = KEY_INVALID_STATUS;
		sClickKey[i].state = KEY_IDLE;
	}
}

/*
 * ============================================================================
 * Function	: ���õ������
 * Input	: uint8_t id ����ID
			  uint16_t time ����ʱ��
			  uint16_t timeout ��ʱʱ��
			  uint16_t longpress ����ʱ��
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void set_click_key(uint8_t id, uint16_t time, uint16_t timeout, uint16_t longpress)
{
	uint8_t cid;

	for (cid = 0; cid < CLICK_KEY_NUM; cid++)
	{
		if (sClickKey[cid].id == 0xff)
		{
			break;
		}
	}

	if (cid < CLICK_KEY_NUM)
	{
		sKey[id].idx[KEY_CLICK_INDEX] = cid;

		sClickKey[cid].flag.newState = KEY_INVALID_STATUS;
		sClickKey[cid].flag.oldState = KEY_RELEASE;
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
 * ============================================================================
 * Function	: ����״̬����
 * Input	: uint8_t id ����ID
			  uint16_t press ���µ�����ʱ��
			  uint16_t release �ɿ�������ʱ��
			  uint16_t period ����֮��ʱ���ؽ��������
 * Output	: None
 * Return	: None
 * ============================================================================
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
		sTapKey[idx].flag.oldState = KEY_RELEASE;

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
 * ============================================================================
 * Function	: �����ж��źţ������жϼ���ֵ
 * Input	: uint8_t id ����ID
 * Output	: None
 * Return	: None
 * ============================================================================
 */
void increase_count(uint8_t id)
{
	sKey[id].irqCount++;
}

/*
 * ============================================================================
 * Function	: �������������¼�
 * Input	: uint8_t id ����ID
			  uint8_t event �¼���Դ��0 ����״̬�ı� 1 ��ʱ
 * Output	: uint32_t *pWaitTime �´γ�ʱ�����ʱ�䣬0xFFFFFFFF��ʾ���̽���
 * Return	: None
 * ============================================================================
 */
static uint8_t deal_key_status(uint8_t id, uint8_t event, uint32_t *pWaitTime)
{
	uint8_t keyType;
	uint8_t cid = sKey[id].idx[KEY_CLICK_INDEX];
	ClickState_t oldState = sClickKey[cid].state;

	keyType = sTypeTab[oldState][event];
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
		debug("key %u type %u %u\r\n", id, sClickKey[cid].keyType, *pWaitTime);
		sClickKey[cid].keyType = KEY_SETTLE;
	}

	return keyType;
}

/*
 * ============================================================================
 * Function	: �����Ͱ������봦��������˫��������������
 * Input	: uint8_t id ����ID
 * Output	: uint32_t *pTime �´δ����ʱ��
 * Return	: None
 * ============================================================================
 */
void click_key_deal(uint8_t id, uint32_t *pTime)
{
	uint8_t state;
	uint8_t irqCount; //�жϼ���ֵ����
	uint8_t cid;

	state = get_key_pressed(id); //��ȡ�����Ƿ���
	cid = sKey[id].idx[KEY_CLICK_INDEX];
	irqCount = sKey[id].irqCount; //�Ȼ������ֵ����ֹ�жϲ���֮��ǰ�����ݲ�һ��

	if (irqCount != sClickKey[cid].irqCountBak) //�жϼ���ֵ�����仯��������������ֵ
	{
		sClickKey[cid].irqCountBak = irqCount;
		sClickKey[cid].flag.newState = KEY_INVALID_STATUS; //ǿ�ƽ���һ��ˢ��
	}

	*pTime = KEY_SETTLE_TICKS; //Ĭ������Ϊ����ɨ����ʱ�䣬��������������Ҫ�����Ϊ��ʱʱ��

	if (state != sClickKey[cid].flag.newState)	//�����׶Σ�����״̬�ı䣬��������״̬
	{
		sClickKey[cid].count = sClickKey[cid].cfgCount; //������������ֵ
		sClickKey[cid].flag.newState = state;
	}
	else if (sClickKey[cid].count)
	{
		sClickKey[cid].count--;
		if (0 == sClickKey[cid].count) //������������ȡ����ǰ������ȶ�״̬
		{
			if (sClickKey[cid].flag.oldState != sClickKey[cid].flag.newState) //״̬��һ�£������ı�
			{
				deal_key_status(id, 0, pTime);
				sClickKey[cid].flag.oldState = sClickKey[cid].flag.newState;
			}
		}
	}
	else //�������׶Σ��ȴ���ʱ������״̬��
	{
		deal_key_status(id, 1, pTime);
	}
}

/*
 * ============================================================================
 * Function	: ״̬�Ͱ������봦�����¡�����
 * Input	: uint8_t id ����ID
 * Output	: uint32_t *pTime �´δ����ʱ��
 * Return	: None
 * ============================================================================
 */
void tap_key_deal(uint8_t id, uint32_t *pTime)
{
	uint8_t state;
	uint8_t irqCount; //�жϼ���ֵ����
	uint8_t tid;
	uint8_t keyType;

	state = get_key_pressed(id); //��ȡ�����Ƿ���
	tid = sKey[id].idx[KEY_CLICK_INDEX];
	irqCount = sKey[id].irqCount; //�Ȼ������ֵ����ֹ�жϲ���֮��ǰ�����ݲ�һ��

	if (irqCount != sTapKey[tid].irqCountBak) //�жϼ���ֵ�����仯��������������ֵ
	{
		sTapKey[tid].irqCountBak = irqCount;
		sTapKey[tid].flag.newState = KEY_INVALID_STATUS; //ǿ�ƽ���һ��ˢ��
	}

	*pTime = KEY_SETTLE_TICKS;	//Ĭ������Ϊ����ɨ����ʱ�䣬��������������Ҫ�����Ϊ��ʱʱ��

	if (state != sTapKey[tid].flag.newState) //�����׶Σ�����״̬�ı䣬��������״̬
	{
		//���ݰ���״̬�����¶�Ӧ����������ֵ
		sTapKey[tid].count = state ? sTapKey[tid].cfgPress : sTapKey[tid].cfgRelease;
		sTapKey[tid].flag.newState = state;
	}
	else if (sTapKey[tid].count)
	{
		sTapKey[tid].count--;
		if (0 == sTapKey[tid].count) //������������ȡ����ǰ������ȶ�״̬
		{
			if (sTapKey[tid].flag.oldState != sTapKey[tid].flag.newState)
			{
				if (state)
				{
					keyType = TAP_PRESS;
					*pTime = sTapKey[tid].cfgPeiod;
				}
				else
				{
					keyType = TAP_RELEASE;
				}

				input_key_deal(id, keyType);
				sTapKey[tid].flag.oldState = sTapKey[tid].flag.newState;
			}
		}
	}
	else
	{
		*pTime = MAX_WAIT_TIME;
		if (state)
		{
			*pTime = sTapKey[tid].cfgPeiod;
			input_key_deal(id, TAP_PRESST);
		}
	}
}