#ifndef __INCLUDE_ARCH_ARCH_H__
#define __INCLUDE_ARCH_ARCH_H__

void archInit(void);
void archInterruptsEnable(void);
void archInterruptsDisable(void);
void archHalt(void);
void archHaltAndCatchFire(void);

#endif
