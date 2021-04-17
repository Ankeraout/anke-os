MAKEFLAGS+=--no-builtin-rules

CONFIG=Makefile.config

include ${CONFIG}

all: kernel

kernel:
	$(MAKE) -C src/kernel

clean:
	$(MAKE) -C src/kernel clean

.PHONY: clean kernel
