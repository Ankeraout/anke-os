MAKEFLAGS+=--no-builtin-rules

CONFIG=../../../../Makefile.config
include ${CONFIG}

all: bootstrap.a

bootstrap.a: boot.o
	$(KERNEL_AR) rcs $@ $<

%.o: %.asm
	$(KERNEL_AS) $(KERNEL_ASFLAGS) $< -o $@

clean:
	rm -f boot.o
	rm -f bootstrap.a

.PHONY: all clean
