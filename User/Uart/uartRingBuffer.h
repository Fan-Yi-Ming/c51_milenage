#ifndef UARTRINGBUFFER_H_
#define UARTRINGBUFFER_H_

#include "tools.h"

void uartRingBuffer_init(void);
void uartRingBuffer_proc(void);

void uartRingBuffer_sendFrame(uint8_t cmd, uint16_t dataLen, uint8_t *dat);

#endif /*UARTRINGBUFFER_H_*/
