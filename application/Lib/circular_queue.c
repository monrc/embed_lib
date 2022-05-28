

#include <stdio.h>
#include <string.h>
#include "circular_queue.h"

#define CRITICAL_PROTECT 2 // 0 无临界保护、 1 任务级临界保护、 2 中断级临界保护

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
 * Function	: 队列初始化
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
	queue->buffSize = (buffSize / itemSize) * itemSize; //长度对齐
	queue->itemSize = itemSize;

#if (1 == QUEUE_DEBUG)
	queue->pName = pName + 1; // +1 过滤 & 符号
#endif
}

/*
 * ============================================================================
 * Function	: 获取队列中缓存的数据项个数
 * Input	: CircleQueue_t *queue 队列指针
 * Output	: None
 * Return	: 队列中缓存的数据项个数
 * ============================================================================
 */
uint16_t get_queue_size(CircleQueue_t *queue)
{
	uint16_t contains;

	enter_critical();
	contains = (queue->rear + queue->buffSize - queue->front) % queue->buffSize; //当前队列已经占用的存储空间
	contains /= queue->itemSize;
	exit_critical();

	return contains;
}

/*
 * ============================================================================
 * Function	: 获取队列缓冲区可以存储数据项的个数
 * Input	: CircleQueue_t *queue 队列指针
 * Output	: None
 * Return	: 剩余可以存储数据项的个数
 * ============================================================================
 */
uint16_t get_remain_num(CircleQueue_t *queue)
{
	uint16_t itemNum;
	uint16_t contains;

	enter_critical();
	contains = (queue->rear + queue->buffSize - queue->front) % queue->buffSize; //当前队列已经占用的存储空间
	exit_critical();

	itemNum = (queue->buffSize - contains) / queue->itemSize; //队列中空闲的数据存储区的个数

	return itemNum - 1; //减一，去除队列满需要的一个空间区
}

/*
 * ============================================================================
 * Function	: 将一个数据项入队
 * Input	: CircleQueue_t *queue 队列指针
			  void *pData 数据项指针
 * Output	: None
 * Return	: true 成功		false 失败
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
		queue->rear = rear; //修改队尾索引
	}

	exit_critical();

	return ret;
}

/*
 * ============================================================================
 * Function	: 从队列中提取一个数据项
 * Input	: CircleQueue_t *queue 队列指针
 * Output	: void *pData 数据项指针
 * Return	: true 成功		false 失败
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
 * Function	: 将多个数据入队
 * Input	: CircleQueue_t *queue 队列指针
			  void *pData 数据指针
			  uint16_t size 数据长度
 * Output	: None
 * Return	: true 成功		false 失败
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

	contains = (queue->rear + queue->buffSize - queue->front) % queue->buffSize; //当前队列已经占用的存储空间
	idleSize = queue->buffSize - contains - 1; //队列中空闲的数据存储区的个数, 减一，去除队列满需要的一个空间区

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
		if (rear < queue->buffSize) //地址未溢出
		{
			memcpy(queue->buff + queue->rear, pData, size);
			queue->rear = rear;
		}
		else //地址溢出，分两段写入缓存
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
 * Function	: 从队列中获取一段数据
 * Input	: CircleQueue_t *queue 队列指针
			  uint16_t *pSize 	要提取的数据长度，0：全部的数据
 * Output	: void *pData		消息数据
			  uint16_t *pSize 	返回的数据长度
 * Return	: true 成功		false 失败
 * ============================================================================
 */
bool de_queue_bytes(CircleQueue_t *queue, void *pData, uint16_t *pSize)
{
	uint16_t contains;
	uint16_t firstLen = 0;
	uint16_t front;
	bool ret = false;

	enter_critical();

	contains = (queue->rear + queue->buffSize - queue->front) % queue->buffSize; //当前队列已经占用的存储空间

	if (*pSize == 0) //长度为0，直接获取缓冲区的所有数据
	{
		*pSize = contains;
	}

	if (contains >= *pSize) //缓冲区数据个数满足需求
	{
		front = queue->front + *pSize;
		if (front < queue->buffSize) //地址未溢出
		{
			memcpy(pData, queue->buff + queue->front, *pSize);
			queue->front = front;
		}
		else //地址溢出，分两段提取数据
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
 * Function	: 将一条消息存入队列
 * Input	: CircleQueue_t *queue 队列指针
			  void *pData 消息指针
			  uint16_t size 消息长度
 * Output	: None
 * Return	: true 成功		false 失败
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
	contains = (queue->rear + queue->buffSize - queue->front) % queue->buffSize; //当前队列已经占用的存储空间
	itemNum = queue->buffSize - contains - 1; //队列中空闲的数据存储区的个数, 减一，去除队列满需要的一个空间区

	if (itemNum < size + 1)
	{
#if (1 == QUEUE_DEBUG)
		printf("%s full at %s %u", queue->pName, pFile, line);
#endif
		ret = false;
	}
	else
	{
		queue->buff[queue->rear] = size; //存储消息长度
		rear = queue->rear + size + 1;
		if (rear < queue->buffSize) //地址未溢出
		{
			memcpy(queue->buff + queue->rear + 1, pData, size); // 存储数据
			queue->rear = rear;
		}
		else //地址溢出，分两段写入缓存
		{
			queue->rear++; //存储长度之后，尾指针后移
			firstLen = queue->buffSize - queue->rear;
			memcpy(queue->buff + queue->rear, pData, firstLen);				   // 存储数据段1
			memcpy(queue->buff, (uint8_t *)pData + firstLen, size - firstLen); // 存储数据段2
			queue->rear = rear % queue->buffSize;
		}
	}

	exit_critical();

	return ret;
}

/*
 * ============================================================================
 * Function	: 从队列中获取一条消息
 * Input	: CircleQueue_t *queue 队列指针
 * Output	: void *pData		消息数据
			  uint16_t *pSize 	消息长度
 * Return	: true 成功		false 失败
 * ============================================================================
 */
bool de_queue_msg(CircleQueue_t *queue, void *pData, uint16_t *pSize)
{
	uint16_t front;
	uint16_t firstLen;
	bool ret = false;

	enter_critical();

	if (queue->front != queue->rear) //队列不为空
	{
		*pSize = queue->buff[queue->front]; //提取消息长度
		front = queue->front + *pSize + 1;
		if (front < queue->buffSize) //地址未溢出
		{
			memcpy(pData, queue->buff + queue->front + 1, *pSize);
			queue->front = front;
		}
		else //地址溢出，分两段提取数据
		{
			queue->front++; //提取长度之后，头指针后移
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
 * Function	: 队列模块测试功能是否正常
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