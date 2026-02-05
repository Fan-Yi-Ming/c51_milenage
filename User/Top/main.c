#include "tools.h"
#include "uartRingBuffer.h"

void main(void)
{
    // 配置P4端口：
    // P4.5 设置为推挽输出
    P4M0 = 0x20;
    P4M1 = 0xDF;

    // 设置P4.5为低电平
    P45 = 0;

    uartRingBuffer_init();

    // 开启总中断
    EA = 1;

    // 开启串口中断
    ES = 1;

    // 主循环
    while (1)
    {
        uartRingBuffer_proc();
    }
}