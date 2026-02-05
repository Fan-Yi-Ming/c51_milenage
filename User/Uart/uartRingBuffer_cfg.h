#ifndef UARTRINGBUFFER_CFG_H_
#define UARTRINGBUFFER_CFG_H_

#include "tools.h"
#include "ringBuffer.h"
#include "string.h"

#define UART_FRAME_DATA_MAX 128                                // 最大数据长度
#define UART_BUFFERSIZE_TX (UART_FRAME_DATA_MAX + 8)           // 最大发送缓冲区大小
#define UART_BUFFERSIZE_RX (1 + (UART_FRAME_DATA_MAX + 8) * 2) // 环形缓冲区需要有一个字节冗余空间

/**
 * @brief 帧解析状态机结构体
 * @details 用于记录帧解析过程中的状态、计数、CRC校验值及数据缓冲区指针
 */
typedef struct
{
    uint8_t step;    ///< 当前解析步骤
    uint8_t cnt;     ///< 数据计数
    uint16_t crc16;  ///< CRC16 校验值
    uint8_t *buffer; ///< 数据缓冲区指针
} _st_FramePhaser;

#endif /*UARTRINGBUFFER_CFG_H_*/
