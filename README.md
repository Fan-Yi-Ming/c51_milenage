Milenage算法库 - 使用说明
概述
这是一个专为USIM卡端设计的Milenage算法实现库（因此不通过OP去计算OPc），用于3G/4G/5G移动通信网络认证。本实现采用纯C语言编写，完全兼容51单片机等资源受限的嵌入式平台（无64位整数运算的单片机）。算法基于AES-128加密算法，用于USIM卡和网络之间的双向认证。

注意：本库是USIM卡端的实现。对于网络侧（AuC/HSS）的Milenage算法，我将提供对应的Python实现。

功能特性
✅ 完整的Milenage算法函数 (f1, f1, f2, f3, f4, f5, f5) 实现

✅ 序列号(SQN)检查与同步机制

✅ AKA认证流程核心功能

✅ 可配置的旋转值和常量参数

✅ 内存高效的设计 - 适合嵌入式系统

✅ 纯C语言实现 - 无外部依赖

✅ 51单片机兼容 - 适合资源受限的MCU

✅ 固定大小内存分配 - 无动态内存分配


<img width="2648" height="1224" alt="image" src="https://github.com/user-attachments/assets/0ecfb6bc-7151-457b-8765-48ba35b938e7" />

请在milenage.c文件内修改 K OPc SQN 等参数。

串口帧协议解析：
A5 5A （帧头）+ XX（command字节）+ XX XX（数据包长度）+ XX XX XX (数据) + XX XX (crc16_ibm 校验字节) + 0xFF (帧尾)

串口测试用例如下：（每次测试要断电重启，因为SQN每次认证成功会加1）


初始化milenage算法（执行aes密钥扩展函数）


--->: A5 5A 04 00 00 C3 29 FF

<---: A5 5A 04 00 00 C3 29 FF


第一组： SQN不在窗口内，所以要激活AUTS

--->: A5 5A 05 00 20 19 85 16 70 36 EB 58 F5 62 1C 6D 8B 26 90 DC E1 08 CA 62 66 BB C3 80 00 EE A2 78 EA CB EF 40 9D AE 5B FF

<---: A5 5A 05 00 02 00 0E 70 68 FF

<---: A5 5A 07 00 0E D9 98 66 D6 D6 D5 84 25 71 C3 D2 C6 D2 B7 8B C5 FF


第一组： SQN在窗口内，返回RES(8字节) CK（16字节） IK（16字节）

--->: A5 5A 05 00 20 A7 09 F1 AC 88 E8 FE FB 87 AE 24 D9 1A 30 58 62 97 DA 40 EC 4E 86 80 00 88 B4 89 B8 A5 93 73 EF C9 5B FF

<---: A5 5A 05 00 02 00 28 E2 6D FF

<---: A5 5A 05 00 28 CC 53 A9 30 B6 99 08 48 48 7E BE 03 54 25 E8 98 EA 48 6E 54 AF FE 53 AE E4 53 3E 11 C7 A4 CD F2 01 B0 F7 A6 B1 45 C9 CF D9 50 FF
