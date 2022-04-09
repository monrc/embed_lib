#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

#include "print.h"

#define QUEUE_DEBUG		 1


#define QUEUE_SIZE(itemSize, itemNum) ((itemSize) * ((itemNum) + 1))

typedef struct
{

#if (1 == QUEUE_DEBUG)
	char *pName;
#endif
	uint8_t *buff; //数据存储区指针

	uint16_t front; //队列头部
	uint16_t rear;	//队列尾部

	uint16_t buffSize; //队列个数
	uint8_t itemSize;  //队列元素大小
} CircleQueue_t;

#if (1 == QUEUE_DEBUG)
#define creat_queue(queue, itemSize, buff) init_queue(queue, itemSize, buff, sizeof(buff), #queue);
#define en_queue(queue, pData)			   queue_input(queue, pData, __FILE__, __LINE__);
#define en_queue_bytes(queue, pData, size) queue_input_bytes(queue, pData, size, __FILE__, __LINE__);

void init_queue(CircleQueue_t *queue, uint8_t itemSize, void *buff, uint16_t buffSize, char *pName);
bool queue_input(CircleQueue_t *queue, void *pData, char *pFile, int line);
bool queue_input_bytes(CircleQueue_t *queue, void *pData, uint16_t size, char *pFile, int line);
bool queue_input_msg(CircleQueue_t *queue, void *pData, uint16_t size, char *pFile, int line);
#else
#define creat_queue(queue, itemSize, buff) init_queue(queue, itemSize, buff, sizeof(buff));
#define en_queue(queue, pData)			   queue_input(queue, pData);
#define en_queue_bytes(queue, pData, size) queue_input_bytes(queue, pData, size);

void init_queue(CircleQueue_t *queue, uint8_t itemSize, void *buff, uint16_t buffSize);
bool queue_input(CircleQueue_t *queue, void *pData);
bool queue_input_bytes(CircleQueue_t *queue, void *pData, uint16_t size);
bool queue_input_msg(CircleQueue_t *queue, void *pData, uint16_t size);
#endif

uint16_t get_queue_size(CircleQueue_t *queue);
uint16_t get_remain_num(CircleQueue_t *queue);
bool de_queue(CircleQueue_t *queue, void *pData);
bool de_queue_bytes(CircleQueue_t *queue, void *pData, uint16_t *pSize);
bool de_queue_msg(CircleQueue_t *queue, void *pData, uint16_t *pSize);

#endif
