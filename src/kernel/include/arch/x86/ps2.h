#ifndef __INCLUDE_ARCH_X86_PS2_H__
#define __INCLUDE_ARCH_X86_PS2_H__

#include <stdbool.h>
#include <stdint.h>

void ps2Init(void);
bool ps2CanRead(int p_port);
uint8_t ps2Read(int p_port);
void ps2Write(int p_port, uint8_t p_value);

#endif
