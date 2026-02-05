#ifndef CIRCULARQUEUE_H_
#define CIRCULARQUEUE_H_

#include "tools.h"

typedef struct
{
    uint8_t *buffer;        // 静态分配内存
    uint16_t bufferSize;    // 实际使用的缓冲区大小
    volatile uint16_t head; // 写指针
    volatile uint16_t tail; // 读指针
} _st_RingBuffer;           // 环形缓冲区结构体

void ringBuffer_init(_st_RingBuffer *pRB, uint8_t *buffer, uint16_t bufferSize);
uint8_t ringBuffer_writeByte(_st_RingBuffer *pRB, uint8_t byt);
uint8_t ringBuffer_readByte(_st_RingBuffer *pRB, uint8_t *byt);

#endif /*CIRCULARQUEUE_H_*/
