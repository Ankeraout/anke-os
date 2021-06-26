MAKEFLAGS+=--no-builtin-rules

CONFIG=../../../Makefile.config
include ${CONFIG}

all: crti.o crtn.o

%.o: %.asm
	$(KERNEL_AS) $(KERNEL_ASFLAGS) $< -o $@

clean:
	rm -f crti.o
	rm -f crtn.o

.PHONY: all clean
