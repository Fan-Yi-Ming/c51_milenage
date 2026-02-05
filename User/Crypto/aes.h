#ifndef __AES_H
#define __AES_H

#include "tools.h"

// 定义 AES 类型（128位、192位或256位）
#define AES_128 ///< 使用 AES-128 算法
// #define AES_192 ///< 使用 AES-192 算法
// #define AES_256 ///< 使用 AES-256 算法

/**
 * @brief 根据定义的 AES 类型设置相关参数
 *
 * 根据选择的 AES 密钥长度，定义状态矩阵的列数（Nb）、密钥字数（Nk）和轮数（Nr）。
 */
#ifdef AES_128
#define Nb 4  ///< 状态矩阵的列数（32位字），对于所有 AES 变体均为 4
#define Nk 4  ///< 密钥中的 32 位字数，AES-128 为 4
#define Nr 10 ///< 轮数，AES-128 为 10
#elif defined(AES_192)
#define Nb 4  ///< 状态矩阵的列数（32位字），对于所有 AES 变体均为 4
#define Nk 6  ///< 密钥中的 32 位字数，AES-192 为 6
#define Nr 12 ///< 轮数，AES-192 为 12
#elif defined(AES_256)
#define Nb 4  ///< 状态矩阵的列数（32位字），对于所有 AES 变体均为 4
#define Nk 8  ///< 密钥中的 32 位字数，AES-256 为 8
#define Nr 14 ///< 轮数，AES-256 为 14
#else
#error "No AES type defined" ///< 如果未定义任何 AES 类型，则触发编译错误
#endif

/**
 * @brief 生成轮密钥
 *
 * 根据初始密钥生成所有轮密钥，并存储在全局变量 w 中。
 *
 * @param key 初始密钥指针，长度为 Nk * 4 字节
 */
void aes_keyexpansion(uint8_t *key);

/**
 * @brief 对单个数据块执行 AES 加密
 *
 * 对输入的数据块进行 AES 加密，并将结果存储在输出缓冲区中。
 *
 * @param input 输入数据指针，长度为 16 字节
 * @param output 输出数据指针，长度为 16 字节
 */
void aes_encrypt(uint8_t *input, uint8_t *output);

/**
 * @brief 对单个数据块执行 AES 解密
 *
 * 对输入的数据块进行 AES 解密，并将结果存储在输出缓冲区中。
 *
 * @param input 输入数据指针，长度为 16 字节
 * @param output 输出数据指针，长度为 16 字节
 */
void aes_decrypt(uint8_t *input, uint8_t *output);

#endif /* __AES_H */
