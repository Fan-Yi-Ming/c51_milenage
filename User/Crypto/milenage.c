#include "tools.h"
#include "aes.h"

code uint8_t r1 = 64;
code uint8_t r2 = 0;
code uint8_t r3 = 32;
code uint8_t r4 = 64;
code uint8_t r5 = 96;
code uint8_t c1[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
code uint8_t c2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
code uint8_t c3[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
code uint8_t c4[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
code uint8_t c5[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08};

// 用户密钥，128位，是函数 f1、f1、f2、f3、f4、f5 和 f5 的输入
uint8_t K[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
// 由 OP 和 K 推导得到的128位值，用于函数的计算过程中
uint8_t OPc[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
// 序列号，48位，是函数 f1 的输入
uint8_t SQN[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AMF[2];   // 认证管理字段，16位，是函数 f1 和 f1* 的输入
uint8_t RAND[16]; // 随机数，128位

uint8_t TEMP[16]; // 128位中间值，在函数的计算过程中使用

uint8_t AK[6];    // 匿名密钥，48位，由函数 f5 或 f5* 输出
uint8_t MAC_A[8]; // 网络认证码，64位，由函数 f1 输出
uint8_t MAC_S[8]; // 再同步认证码，64位，由函数 f1* 输出

uint8_t RES[8]; // 签名响应，64位，由函数 f2 输出
uint8_t CK[16]; // 保密性密钥，128位，由函数 f3 输出
uint8_t IK[16]; // 完整性密钥，128位，由函数 f4 输出

uint8_t AUTN[16]; // 128位网络认证令牌AUTN
uint8_t AUTS[14]; // 112位同步令牌AUTS

static void reverse_section(uint8_t *array, uint8_t start, uint8_t end)
{
    uint8_t temp;

    end--; // 转为最后一个元素的索引
    while (start < end)
    {
        temp = array[start];
        array[start] = array[end];
        array[end] = temp;
        start++;
        end--;
    }
}

void rot(uint8_t *array, uint8_t arrayLen, uint8_t nBytes)
{
    if (arrayLen == 0 || nBytes == 0)
    {
        return;
    }

    nBytes = nBytes % arrayLen;
    if (nBytes == 0)
    {
        return;
    }

    // 三次反转算法：
    // 1. 反转前nBytes个元素
    reverse_section(array, 0, nBytes);
    // 2. 反转剩余的元素
    reverse_section(array, nBytes, arrayLen);
    // 3. 反转整个数组
    reverse_section(array, 0, arrayLen);
}

void computeK()
{
    aes_keyexpansion(K);
}

void computeTEMP()
{
    uint8_t i;

    for (i = 0; i < 16; i++)
    {
        TEMP[i] = RAND[i] ^ OPc[i];
    }
    aes_encrypt(TEMP, TEMP);
}

void increaseSQN()
{
    uint8_t i = 6; // 6字节数组，索引0-5

    while (i > 0)
    {
        i--; // 第一次循环 i=5（最低字节）
        SQN[i]++;
        if (SQN[i] != 0)
        {
            break;
        }
    }
}

void f1(uint8_t *sqn)
{
    uint8_t i;
    uint8_t temp[16];

    for (i = 0; i < 6; i++)
    {
        temp[i] = sqn[i];
        temp[i + 8] = sqn[i];
    }
    for (i = 0; i < 2; i++)
    {
        temp[i + 6] = AMF[i];
        temp[i + 14] = AMF[i];
    }

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ OPc[i];
    }

    rot(temp, 16, r1 / 8);

    for (i = 0; i < 16; i++)
    {
        temp[i] = TEMP[i] ^ temp[i];
    }

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ c1[i];
    }

    aes_encrypt(temp, temp);

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ OPc[i];
    }

    for (i = 0; i < 8; i++)
    {
        MAC_A[i] = temp[i];
    }
}

uint8_t cmp48(const uint8_t a[6], const uint8_t b[6]) // 大端序比较函数
{
    uint8_t i;

    for (i = 0; i < 6; i++) // 从最高字节开始比较
    {
        if (a[i] != b[i])
        {
            return (a[i] < b[i]) ? 1 : 2;
        }
    }
    return 0;
}

uint8_t add48(const uint8_t a[6], const uint8_t b[6], uint8_t r[6]) // 大端序加法函数
{
    uint8_t i = 6; // 从6开始（数组长度）
    uint16_t sum;  // 临时存储和
    uint8_t c = 0; // 进位标志

    while (i > 0)
    {
        i--; // 先递减，第一次循环 i=5（最低字节）
        sum = (uint16_t)a[i] + (uint16_t)b[i] + c;
        r[i] = (uint8_t)sum;
        c = (sum > 255) ? 1 : 0; // 进位
    }
    return c; // 返回最终进位
}

uint8_t sub48(const uint8_t a[6], const uint8_t b[6], uint8_t r[6]) // 大端序减法函数
{
    uint8_t i = 6; // 从6开始（数组长度）
    int16_t diff;  // 临时存储差
    uint8_t c = 0; // 借位标志

    while (i > 0)
    {
        i--; // 先递减，第一次循环 i=5（最低字节）
        diff = (int16_t)a[i] - (int16_t)b[i] - c;

        if (diff < 0)
        {
            diff += 256; // 借位
            c = 1;
        }
        else
        {
            c = 0;
        }

        r[i] = (uint8_t)diff;
    }
    return c; // 返回最后的借位
}

uint8_t checkSQN(uint8_t *sqn)
{
    uint8_t distance[6];
    uint8_t window[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x40}; // 64
    uint8_t max_value[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t one[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    uint8_t temp[6];

    // 完全相等的情况
    if (cmp48(sqn, SQN) == 0)
    {
        return 1;
    }

    // 计算正向距离
    if (cmp48(sqn, SQN) == 2)
    { // sqn > SQN，直接相减
        sub48(sqn, SQN, distance);
    }
    else
    { // sqn < SQN，发生回绕
        sub48(max_value, SQN, temp);
        add48(temp, one, temp);
        add48(temp, sqn, distance);
    }

    // 检查 distance <= 64，使用三元运算符
    return (cmp48(distance, window) <= 1) ? 1 : 0;
}

uint8_t checkMAC_A(uint8_t *mac_a)
{
    uint8_t i;
    uint8_t result = 0;

    for (i = 0; i < 8; i++)
    {
        result |= MAC_A[i] ^ mac_a[i]; // 使用异或，不同时为非0
    }

    // 如果result为0，说明所有字节都相同
    return (result == 0) ? 1 : 0;
}

void f1star(uint8_t *sqn_ms, uint8_t *amf)
{
    uint8_t i;
    uint8_t temp[16];

    for (i = 0; i < 6; i++)
    {
        temp[i] = sqn_ms[i];
        temp[i + 8] = sqn_ms[i];
    }
    for (i = 0; i < 2; i++)
    {
        temp[i + 6] = amf[i];
        temp[i + 14] = amf[i];
    }

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ OPc[i];
    }

    rot(temp, 16, r1 / 8);

    for (i = 0; i < 16; i++)
    {
        temp[i] = TEMP[i] ^ temp[i];
    }

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ c1[i];
    }

    aes_encrypt(temp, temp);

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ OPc[i];
    }

    for (i = 0; i < 8; i++)
    {
        MAC_S[i] = temp[i + 8];
    }
}

void f5star()
{
    uint8_t i;
    uint8_t temp[16];

    // AK
    for (i = 0; i < 16; i++)
    {
        temp[i] = TEMP[i] ^ OPc[i];
    }

    rot(temp, 16, r5 / 8);

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ c5[i];
    }

    aes_encrypt(temp, temp);

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ OPc[i];
    }

    for (i = 0; i < 6; i++)
    {
        AK[i] = temp[i];
    }
}

void computeAUTS()
{
    uint8_t i;
    uint8_t auts_amf[2] = {0x00, 0x00};

    f5star();
    f1star(SQN, auts_amf);
    for (i = 0; i < 6; i++)
    {
        AUTS[i] = SQN[i] ^ AK[i];
    }
    for (i = 0; i < 8; i++)
    {
        AUTS[i + 6] = MAC_S[i];
    }
}

uint8_t kernel()
{
    uint8_t i;
    uint8_t temp[16];
    uint8_t sqn[6];
    uint8_t mac_a[8];
    uint8_t checkSQN_result;
    uint8_t checkMAC_A_result;

    // AK RES
    for (i = 0; i < 16; i++)
    {
        temp[i] = TEMP[i] ^ OPc[i];
    }

    rot(temp, 16, r2 / 8);

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ c2[i];
    }

    aes_encrypt(temp, temp);

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ OPc[i];
    }

    for (i = 0; i < 6; i++)
    {
        AK[i] = temp[i];
    }
    for (i = 0; i < 8; i++)
    {
        RES[i] = temp[i + 8];
    }

    // check SQN
    for (i = 0; i < 6; i++)
    {
        sqn[i] = AUTN[i] ^ AK[i];
    }
    checkSQN_result = checkSQN(sqn);

    // MAC_A
    for (i = 0; i < 2; i++)
    {
        AMF[i] = AUTN[i + 6];
    }
    f1(sqn);

    // check MAC_A
    for (i = 0; i < 8; i++)
    {
        mac_a[i] = AUTN[i + 8];
    }
    checkMAC_A_result = checkMAC_A(mac_a);

    // final check
    if (checkMAC_A_result == 0)
    {
        return 1; // MAC_A错误
    }
    if (checkSQN_result == 0)
    {
        computeAUTS();
        increaseSQN();
        return 2; // SQN错误
    }
    else
    {
        for (i = 0; i < 6; i++)
        {
            SQN[i] = sqn[i];
        }
    }

    // CK
    for (i = 0; i < 16; i++)
    {
        temp[i] = TEMP[i] ^ OPc[i];
    }

    rot(temp, 16, r3 / 8);

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ c3[i];
    }

    aes_encrypt(temp, temp);

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ OPc[i];
    }

    for (i = 0; i < 16; i++)
    {
        CK[i] = temp[i];
    }

    // IK
    for (i = 0; i < 16; i++)
    {
        temp[i] = TEMP[i] ^ OPc[i];
    }

    rot(temp, 16, r4 / 8);

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ c4[i];
    }

    aes_encrypt(temp, temp);

    for (i = 0; i < 16; i++)
    {
        temp[i] = temp[i] ^ OPc[i];
    }

    for (i = 0; i < 16; i++)
    {
        IK[i] = temp[i];
    }

    increaseSQN();
    return 0;
}

uint8_t execute_OPc()
{
    computeTEMP();
    return kernel();
}

void setRAND(uint8_t *rand)
{
    uint8_t i;

    for (i = 0; i < 16; i++)
    {
        RAND[i] = rand[i];
    }
}

void setAUTN(uint8_t *autn)
{
    uint8_t i;

    for (i = 0; i < 16; i++)
    {
        AUTN[i] = autn[i];
    }
}

uint8_t *getRES()
{
    return RES;
}

uint8_t *getCK()
{
    return CK;
}

uint8_t *getIK()
{
    return IK;
}

uint8_t *getAUTS()
{
    return AUTS;
}
