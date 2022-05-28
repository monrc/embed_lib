

#include <stdio.h>
#include <string.h>
#include "circular_queue.h"

#define CRITICAL_PROTECT 2 // 0 ���ٽ籣���� 1 �����ٽ籣���� 2 �жϼ��ٽ籣��

#if (CRITICAL_PROTECT == 2)
#include "FreeRTOS.h"
#define enter_critical() uint32_t interrupt = portSET_INTERRUPT_MASK_FROM_ISR();
#define exit_critical()	 portCLEAR_INTERRUPT_MASK_FROM_ISR(interrupt)
#elif (CRITICAL_PROTECT == 1)
#include "FreeRTOS.h"
#define enter_critical() portENTER_CRITICAL()
#define exit_critical()	 portEXIT_CRITICAL()
#else
#define enter_critical()
#define exit_critical()
#endif

#ifdef debug
	#define printf debug
#endif
/*
 * ============================================================================
 * Function	: ���г�ʼ��
 * Input		: None
 * Output	: None
 * Return	: None
 * ============================================================================
 */
#if (1 == QUEUE_DEBUG)
void init_queue(CircleQueue_t *queue, uint8_t itemSize, void *buff, uint16_t buffSize, char *pName)
#else
void init_queue(CircleQueue_t *queue, uint8_t itemSize, void *buff, uint16_t buffSize)
#endif
{
	queue->front = 0;
	queue->rear = 0;

	queue->buff = buff;
	queue->buffSize = (buffSize / itemSize) * itemSize; //���ȶ���
	queue->itemSize = itemSize;

#if (1 == QUEUE_DEBUG)
	queue->pName = pName + 1; // +1 ���� & ����
#endif
}

/*
 * ============================================================================
 * Function	: ��ȡ�����л�������������
 * Input	: CircleQueue_t *queue ����ָ��
 * Output	: None
 * Return	: �����л�������������
 * ============================================================================
 */
uint16_t get_queue_size(CircleQueue_t *queue)
{
	uint16_t contains;

	enter_critical();
	contains = (queue->rear + queue->buffSize - queue->front) % queue->buffSize; //��ǰ�����Ѿ�ռ�õĴ洢�ռ�
	contains /= queue->itemSize;
	exit_critical();

	return contains;
}

/*
 * ============================================================================
 * Function	: ��ȡ���л��������Դ洢������ĸ���
 * Input	: CircleQueue_t *queue ����ָ��
 * Output	: None
 * Return	: ʣ����Դ洢������ĸ���
 * ============================================================================
 */
uint16_t get_remain_num(CircleQueue_t *queue)
{
	uint16_t itemNum;
	uint16_t contains;

	enter_critical();
	contains = (queue->rear + queue->buffSize - queue->front) % queue->buffSize; //��ǰ�����Ѿ�ռ�õĴ洢�ռ�
	exit_critical();

	itemNum = (queue->buffSize - contains) / queue->itemSize; //�����п��е����ݴ洢���ĸ���

	return itemNum - 1; //��һ��ȥ����������Ҫ��һ���ռ���
}

/*
 * ============================================================================
 * Function	: ��һ�����������
 * Input	: CircleQueue_t *queue ����ָ��
			  void *pData ������ָ��
 * Output	: None
 * Return	: true �ɹ�		false ʧ��
 * ============================================================================
 */

#if (1 == QUEUE_DEBUG)
bool queue_input(CircleQueue_t *queue, const void *pData, char *pFile, int line)
#else
bool queue_input(CircleQueue_t *queue, void *pData)
#endif
{
	uint16_t rear;
	bool ret = true;

	enter_critical();

	rear = (queue->rear + queue->itemSize) % queue->buffSize;
	if (rear == queue->front)
	{
#if (1 == QUEUE_DEBUG)
		printf("%s full at %s %u", queue->pName, pFile, line);
#endif
		ret = false;
	}
	else
	{
		memcpy(queue->buff + queue->rear, pData, queue->itemSize);
		queue->rear = rear; //�޸Ķ�β����
	}

	exit_critical();

	return ret;
}

/*
 * ============================================================================
 * Function	: �Ӷ�������ȡһ��������
 * Input	: CircleQueue_t *queue ����ָ��
 * Output	: void *pData ������ָ��
 * Return	: true �ɹ�		false ʧ��
 * ============================================================================
 */
bool de_queue(CircleQueue_t *queue, void *pData)
{
	bool ret = false;

	enter_critical();

	if (queue->front != queue->rear)
	{
		memcpy(pData, queue->buff + queue->front, queue->itemSize);
		queue->front = (queue->front + queue->itemSize) % queue->buffSize;
		ret = true;
	}

	exit_critical();

	return ret;
}

/*
 * ============================================================================
 * Function	: ������������
 * Input	: CircleQueue_t *queue ����ָ��
			  void *pData ����ָ��
			  uint16_t size ���ݳ���
 * Output	: None
 * Return	: true �ɹ�		false ʧ��
 * ============================================================================
 */
#if (1 == QUEUE_DEBUG)
bool queue_input_bytes(CircleQueue_t *queue, void *pData, uint16_t size, char *pFile, int line)
#else
bool queue_input_bytes(CircleQueue_t *queue, void *pData, uint16_t size)
#endif
{
	uint16_t rear;
	uint16_t firstLen = 0;
	uint16_t idleSize;
	uint16_t contains;
	bool ret = true;

	enter_critical();

	contains = (queue->rear + queue->buffSize - queue->front) % queue->buffSize; //��ǰ�����Ѿ�ռ�õĴ洢�ռ�
	idleSize = queue->buffSize - contains - 1; //�����п��е����ݴ洢���ĸ���, ��һ��ȥ����������Ҫ��һ���ռ���

	if (idleSize < size)
	{
#if (1 == QUEUE_DEBUG)
		printf("%s full at %s %u", queue->pName, pFile, line);
#endif
		ret = false;
	}
	else
	{
		rear = queue->rear + size;
		if (rear < queue->buffSize) //��ַδ���
		{
			memcpy(queue->buff + queue->rear, pData, size);
			queue->rear = rear;
		}
		else //��ַ�����������д�뻺��
		{
			firstLen = queue->buffSize - queue->rear;
			memcpy(queue->buff + queue->rear, pData, firstLen);
			memcpy(queue->buff, (uint8_t *)pData + firstLen, size - firstLen);
			queue->rear = rear % queue->buffSize;
		}
	}

	exit_critical();

	return ret;
}

/*
 * ============================================================================
 * Function	: �Ӷ����л�ȡһ������
 * Input	: CircleQueue_t *queue ����ָ��
			  uint16_t *pSize 	Ҫ��ȡ�����ݳ��ȣ�0��ȫ��������
 * Output	: void *pData		��Ϣ����
			  uint16_t *pSize 	���ص����ݳ���
 * Return	: true �ɹ�		false ʧ��
 * ============================================================================
 */
bool de_queue_bytes(CircleQueue_t *queue, void *pData, uint16_t *pSize)
{
	uint16_t contains;
	uint16_t firstLen = 0;
	uint16_t front;
	bool ret = false;

	enter_critical();

	contains = (queue->rear + queue->buffSize - queue->front) % queue->buffSize; //��ǰ�����Ѿ�ռ�õĴ洢�ռ�

	if (*pSize == 0) //����Ϊ0��ֱ�ӻ�ȡ����������������
	{
		*pSize = contains;
	}

	if (contains >= *pSize) //���������ݸ�����������
	{
		front = queue->front + *pSize;
		if (front < queue->buffSize) //��ַδ���
		{
			memcpy(pData, queue->buff + queue->front, *pSize);
			queue->front = front;
		}
		else //��ַ�������������ȡ����
		{
			firstLen = queue->buffSize - queue->front;
			memcpy(pData, queue->buff + queue->front, firstLen);
			memcpy((uint8_t *)pData + firstLen, queue->buff, *pSize - firstLen);
			queue->front = front % queue->buffSize;
		}
		ret = true;
	}

	exit_critical();

	return ret;
}

/*
 * ============================================================================
 * Function	: ��һ����Ϣ�������
 * Input	: CircleQueue_t *queue ����ָ��
			  void *pData ��Ϣָ��
			  uint16_t size ��Ϣ����
 * Output	: None
 * Return	: true �ɹ�		false ʧ��
 * ============================================================================
 */
#if (1 == QUEUE_DEBUG)
bool queue_input_msg(CircleQueue_t *queue, void *pData, uint16_t size, char *pFile, int line)
#else
bool queue_input_msg(CircleQueue_t *queue, void *pData, uint16_t size)
#endif
{
	uint16_t rear;
	uint16_t firstLen = 0;
	uint16_t itemNum;
	uint16_t contains;
	bool ret = true;

	enter_critical();
	contains = (queue->rear + queue->buffSize - queue->front) % queue->buffSize; //��ǰ�����Ѿ�ռ�õĴ洢�ռ�
	itemNum = queue->buffSize - contains - 1; //�����п��е����ݴ洢���ĸ���, ��һ��ȥ����������Ҫ��һ���ռ���

	if (itemNum < size + 1)
	{
#if (1 == QUEUE_DEBUG)
		printf("%s full at %s %u", queue->pName, pFile, line);
#endif
		ret = false;
	}
	else
	{
		queue->buff[queue->rear] = size; //�洢��Ϣ����
		rear = queue->rear + size + 1;
		if (rear < queue->buffSize) //��ַδ���
		{
			memcpy(queue->buff + queue->rear + 1, pData, size); // �洢����
			queue->rear = rear;
		}
		else //��ַ�����������д�뻺��
		{
			queue->rear++; //�洢����֮��βָ�����
			firstLen = queue->buffSize - queue->rear;
			memcpy(queue->buff + queue->rear, pData, firstLen);				   // �洢���ݶ�1
			memcpy(queue->buff, (uint8_t *)pData + firstLen, size - firstLen); // �洢���ݶ�2
			queue->rear = rear % queue->buffSize;
		}
	}

	exit_critical();

	return ret;
}

/*
 * ============================================================================
 * Function	: �Ӷ����л�ȡһ����Ϣ
 * Input	: CircleQueue_t *queue ����ָ��
 * Output	: void *pData		��Ϣ����
			  uint16_t *pSize 	��Ϣ����
 * Return	: true �ɹ�		false ʧ��
 * ============================================================================
 */
bool de_queue_msg(CircleQueue_t *queue, void *pData, uint16_t *pSize)
{
	uint16_t front;
	uint16_t firstLen;
	bool ret = false;

	enter_critical();

	if (queue->front != queue->rear) //���в�Ϊ��
	{
		*pSize = queue->buff[queue->front]; //��ȡ��Ϣ����
		front = queue->front + *pSize + 1;
		if (front < queue->buffSize) //��ַδ���
		{
			memcpy(pData, queue->buff + queue->front + 1, *pSize);
			queue->front = front;
		}
		else //��ַ�������������ȡ����
		{
			queue->front++; //��ȡ����֮��ͷָ�����
			firstLen = queue->buffSize - queue->front;
			memcpy(pData, queue->buff + queue->front, firstLen);
			memcpy((uint8_t *)pData + firstLen, queue->buff, *pSize - firstLen);
			queue->front = front % queue->buffSize;
		}
		ret = true;
	}

	exit_critical();

	return ret;
}

/*
 * ============================================================================
 * Function	: ����ģ����Թ����Ƿ�����
 * ============================================================================
 */
void queue_test(void)
{
#define MSG_SIZE 7
	
	uint8_t testBuff[QUEUE_SIZE(1, 41)];
	CircleQueue_t queue;

	uint8_t i;
	uint8_t read;
	uint8_t data[MSG_SIZE];
	uint32_t times = 100;

	creat_queue(&queue, 1, testBuff);

	for (i = 0; i < MSG_SIZE; i++)
	{
		data[i] = i;
	}

	while (times--)
	{
		en_queue_bytes(&queue, data, MSG_SIZE);
		for (i = 0; i < MSG_SIZE; i++)
		{
			if (de_queue(&queue, &read))
			{
				if (read != i)
				{
					printf("error %u, %u", read, i);
				}
			}
			else
			{
				printf("lost %u\r\n", i);
			}
		}
	}
}