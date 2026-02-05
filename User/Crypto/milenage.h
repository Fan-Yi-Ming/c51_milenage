#ifndef __MILENAGE_H
#define __MILENAGE_H

#include "tools.h"

void computeK();

void setRAND(uint8_t *rand);
void setAUTN(uint8_t *autn);

uint8_t execute_OPc();

uint8_t *getRES();
uint8_t *getCK();
uint8_t *getIK();
uint8_t *getAUTS();

#endif /* __MILENAGE_H */