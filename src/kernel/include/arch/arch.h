#ifndef __INCLUDE_ARCH_ARCH_H__
#define __INCLUDE_ARCH_ARCH_H__

#include "boot/boot.h"

int archPreinit(struct ts_boot *p_boot);
int archInit(void);
void archInterruptsEnable(void);
void archInterruptsDisable(void);
void archHalt(void);
void archHaltAndCatchFire(void);

#endif
