MAKEFLAGS+=--no-builtin-rules

CONFIG=../../../../Makefile.config
include ${CONFIG}

SOURCES_C=$(shell find `pwd` -name '*.c')
SOURCES_ASM=$(shell find `pwd` -name '*.asm')
OBJECTS_C=$(SOURCES_C:%.c=%.c.o)
OBJECTS_ASM=$(SOURCES_ASM:%.asm=%.asm.o)
OBJECTS=$(OBJECTS_C) $(OBJECTS_ASM)

arch.a: $(OBJECTS)
	$(KERNEL_AR) rcs $@ $^

%.c.o: %.c
	$(KERNEL_CC) $(KERNEL_CFLAGS) $< -o $@

%.asm.o: %.asm
	$(KERNEL_AS) $(KERNEL_ASFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) arch.a

.PHONY: clean
