#ifndef __MILENAGE_H
#define __MILENAGE_H

#include "tools.h"

#define AUTH_SUCCESS 0   // 认证成功
#define AUTH_MAC_ERROR 1 // MAC_A验证失败
#define AUTH_SQN_ERROR 2 // SQN验证失败（触发重同步

void computeK();

void setRAND(uint8_t *rand);
void setAUTN(uint8_t *autn);

uint8_t execute_OPc();

uint8_t *getRES();
uint8_t *getCK();
uint8_t *getIK();
uint8_t *getAUTS();

#endif /* __MILENAGE_H */