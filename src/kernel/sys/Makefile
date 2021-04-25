MAKEFLAGS+=--no-builtin-rules

CONFIG=../../../Makefile.config
include ${CONFIG}

SOURCES=$(shell find `pwd` -name '*.c')
OBJECTS=$(SOURCES:%.c=%.c.o)

all: sys.a

sys.a: $(OBJECTS)
	$(KERNEL_AR) rcs $@ $^

%.c.o: %.c
	$(KERNEL_CC) $(KERNEL_CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f sys.a

.PHONY: all clean
