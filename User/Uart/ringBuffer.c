#include "ringBuffer.h"

void ringBuffer_init(_st_RingBuffer *pRB, uint8_t *buffer, uint16_t bufferSize)
{
    pRB->buffer = buffer;
    pRB->bufferSize = bufferSize;
    pRB->head = 0;
    pRB->tail = 0;
}

uint8_t ringBuffer_writeByte(_st_RingBuffer *pRB, uint8_t byt)
{
    uint16_t next = (pRB->head + 1) % pRB->bufferSize;

    if (next == pRB->tail)
    {
        return 0; // 缓冲区已满
    }
    pRB->buffer[pRB->head] = byt;
    pRB->head = next;
    return 1;
}

uint8_t ringBuffer_readByte(_st_RingBuffer *pRB, uint8_t *byt)
{
    if (pRB->head == pRB->tail)
    {
        return 0; // 缓冲区为空
    }
    *byt = pRB->buffer[pRB->tail];
    pRB->tail = (pRB->tail + 1) % pRB->bufferSize;
    return 1;
}
