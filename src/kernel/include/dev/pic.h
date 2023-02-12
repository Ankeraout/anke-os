#ifndef __INCLUDE_DEV_PIC_H__
#define __INCLUDE_DEV_PIC_H__

#include <stdbool.h>

void picInit(void);
void picEndOfInterrupt(bool l_slaveInterrupt);

#endif
