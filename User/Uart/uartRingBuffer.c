#include "uartRingBuffer_cfg.h"
#include "aes.h"
#include "milenage.h"

static _st_RingBuffer uartRingBuffer_Rx;
static uint8_t uartBuffer_Rx[UART_BUFFERSIZE_RX];

/**
 * @brief 初始化UART和接收缓冲区
 */
void uartRingBuffer_init(void)
{
    ringBuffer_init(&uartRingBuffer_Rx, uartBuffer_Rx, UART_BUFFERSIZE_RX);

    SCON = 0x50;
    AUXR |= 0x40;
    AUXR &= 0xFE;
    TMOD &= 0x0F;
    TL1 = 0xE8;
    TH1 = 0xFF;
    ET1 = 0;
    TR1 = 1;
}

/**
 * @brief UART1接收中断服务程序
 */
void Uart1_Isr() interrupt 4
{
    uint8_t byt;

    if (RI)
    {
        byt = SBUF;
        ringBuffer_writeByte(&uartRingBuffer_Rx, byt);
        RI = 0;
    }
}

/**
 * @brief 计算CRC-16校验值
 * @param dat 指向待校验数据缓冲区的指针
 * @param dataLen 待校验数据的长度（字节数）
 * @return 计算得到的16位CRC校验值，以主机字节序返回
 */
uint16_t crc16_ibm(uint8_t *dat, uint16_t dataLen)
{
    uint16_t i;
    uint8_t j;
    uint16_t crc16 = 0x0000;

    for (i = 0; i < dataLen; i++)
    {
        crc16 ^= dat[i];
        for (j = 0; j < 8; j++)
        {
            if (crc16 & 0x0001)
            {
                crc16 >>= 1;
                crc16 ^= 0x8005;
            }
            else
            {
                crc16 >>= 1;
            }
        }
    }
    return crc16;
}

/**
 * @brief 解析数据帧
 * @param byt 当前字节
 * @param pFramePhaser 解析状态机
 * @return 1:解析成功 0:解析中或失败
 */
uint8_t framePhase(uint8_t byt, _st_FramePhaser *pFramePhaser)
{
    uint16_t dataLen = 0;

    switch (pFramePhaser->step)
    {
    case 0:
        if (byt == 0xA5)
        {
            pFramePhaser->step++;
        }
        break;

    case 1:
        if (byt == 0x5A)
        {
            pFramePhaser->step++;
        }
        else
        {
            pFramePhaser->step = 0;
        }
        break;

    case 2:
        pFramePhaser->buffer[0] = byt;
        pFramePhaser->step++;
        break;

    case 3:
        pFramePhaser->buffer[1] = byt;
        pFramePhaser->step++;
        break;

    case 4:
        pFramePhaser->buffer[2] = byt;
        pFramePhaser->cnt = 0;
        dataLen = (pFramePhaser->buffer[1] << 8) | pFramePhaser->buffer[2];
        pFramePhaser->step++;
        if (dataLen == 0)
        {
            pFramePhaser->step++;
        }
        break;

    case 5:
        pFramePhaser->buffer[3 + pFramePhaser->cnt++] = byt;
        dataLen = (uint16_t)(pFramePhaser->buffer[1] << 8) | (uint16_t)pFramePhaser->buffer[2];
        if (pFramePhaser->cnt >= dataLen)
        {
            pFramePhaser->step++;
        }
        break;

    case 6:
        pFramePhaser->crc16 = byt;
        pFramePhaser->step++;
        break;

    case 7:
        pFramePhaser->crc16 <<= 8;
        pFramePhaser->crc16 += byt;
        dataLen = (uint16_t)(pFramePhaser->buffer[1] << 8) | (uint16_t)pFramePhaser->buffer[2];
        if (pFramePhaser->crc16 == crc16_ibm(pFramePhaser->buffer, dataLen + 3))
        {
            pFramePhaser->step++;
        }
        else
        {
            pFramePhaser->step = 0;
        }
        break;

    case 8:
        pFramePhaser->step = 0;
        if (byt == 0xFF)
        {
            return 1;
        }
        break;

    default:
        pFramePhaser->step = 0;
        break;
    }
    return 0;
}

/**
 * @brief 构造数据帧
 * @param cmd     命令字节
 * @param dataLen 数据长度（字节数）
 * @param dat     载荷数据指针（可为NULL）
 * @param frameBuffer 帧输出缓冲区（需预留足够空间）
 */
void frameMake(uint8_t cmd, uint16_t dataLen, uint8_t *dat, uint8_t *frameBuffer)
{
    uint16_t crc16;

    frameBuffer[0] = 0xA5;
    frameBuffer[1] = 0x5A;
    frameBuffer[2] = cmd;
    frameBuffer[3] = (uint8_t)(dataLen >> 8);
    frameBuffer[4] = (uint8_t)(dataLen & 0xFF);

    if (dataLen > 0 && dat != NULL)
    {
        memcpy(&frameBuffer[5], dat, dataLen);
    }

    crc16 = crc16_ibm(&frameBuffer[2], dataLen + 3);
    frameBuffer[5 + dataLen] = (uint8_t)(crc16 >> 8);
    frameBuffer[6 + dataLen] = (uint8_t)(crc16 & 0xFF);
    frameBuffer[7 + dataLen] = 0xFF;
}

/**
 * @brief 发送单字节数据
 * @param byt 要发送的字节
 */
void uartRingBuffer_sendByte(uint8_t byt)
{
    SBUF = byt;
    while (!TI)
    {
        ;
    }
    TI = 0;
}

/**
 * @brief 发送数据缓冲区
 * @param dat 数据缓冲区
 * @param dataLen 数据长度
 */
void uartRingBuffer_sendBuffer(uint8_t *dat, uint16_t dataLen)
{
    uint16_t i;

    for (i = 0; i < dataLen; i++)
    {
        uartRingBuffer_sendByte(dat[i]);
    }
}

/**
 * @brief 发送完整数据帧
 * @param cmd 命令字节
 * @param dataLen 载荷数据长度（字节数）
 * @param dat 载荷数据指针（可为NULL）
 *
 * @note 帧总长度 = dataLen + 8（固定开销）
 */
void uartRingBuffer_sendFrame(uint8_t cmd, uint16_t dataLen, uint8_t *dat)
{
    uint8_t uartBuffer_Tx[UART_BUFFERSIZE_TX];

    frameMake(cmd, dataLen, dat, uartBuffer_Tx);
    uartRingBuffer_sendBuffer(uartBuffer_Tx, dataLen + 8);
}

/**
 * @brief 执行接收到的命令
 * @param frame_phase 解析后的帧数据
 */
static void uartRingBuffer_execute(uint8_t *buffer)
{
    uint8_t cmd = buffer[0];
    uint16_t dataLen = (uint16_t)(((uint16_t)buffer[1] << 8) | (uint16_t)buffer[2]);
    uint8_t *dat = &buffer[3];

    uint8_t aes_result[16];

    uint8_t milenage_result;
    uint8_t milenage_data[40];

    switch (cmd)
    {
    case 0x00:
        uartRingBuffer_sendFrame(cmd, dataLen, dat);
        break;

    case 0x01:
        aes_keyexpansion(dat);
        uartRingBuffer_sendFrame(cmd, 0, NULL);
        break;

    case 0x02:
        P45 = 1;
        aes_encrypt(dat, aes_result);
        P45 = 0;
        uartRingBuffer_sendFrame(cmd, 16, aes_result);
        break;

    case 0x03:
        P45 = 1;
        aes_decrypt(dat, aes_result);
        P45 = 0;
        uartRingBuffer_sendFrame(cmd, 16, aes_result);
        break;

    case 0x04:
        computeK(); // milenage算法初始化
        uartRingBuffer_sendFrame(cmd, 0, NULL);
        break;

    case 0x05:
        setRAND(dat);
        setAUTN(dat + 16);

        P45 = 1;
        milenage_result = execute_OPc();
        P45 = 0;
        if (milenage_result == 0)
        {
            memcpy(milenage_data, getRES(), 8);
            memcpy(milenage_data + 8, getCK(), 16);
            memcpy(milenage_data + 24, getIK(), 16);
            uartRingBuffer_sendFrame(cmd + milenage_result, 40, milenage_data);
        }
        else if (milenage_result == 2)
        {
            memset(milenage_data, 0x00, 40);
            memcpy(milenage_data, getAUTS(), 14);
            uartRingBuffer_sendFrame(cmd + milenage_result, 40, milenage_data);
        }
        else if (milenage_result == 1)
        {
            memset(milenage_data, 0x00, 40);
            uartRingBuffer_sendFrame(cmd + milenage_result, 40, milenage_data);
        }
        break;

    default:
        break;
    }
}

/**
 * @brief 处理接收到的数据帧
 */
void uartRingBuffer_proc(void)
{
    uint8_t byt;
    static uint8_t buffer[UART_FRAME_DATA_MAX + 3];
    static _st_FramePhaser framePhaser = {0, 0, 0, buffer};

    while (1)
    {
        if (ringBuffer_readByte(&uartRingBuffer_Rx, &byt) == 1)
        {
            if (framePhase(byt, &framePhaser) == 1)
            {
                uartRingBuffer_execute(buffer);
            }
        }
        else
        {
            break;
        }
    }
}
