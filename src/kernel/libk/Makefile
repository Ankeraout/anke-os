MAKEFLAGS+=--no-builtin-rules

CONFIG=../../../Makefile.config
include ${CONFIG}

SOURCES=$(shell find `pwd` -name *.c)
OBJECTS=$(SOURCES:%.c=%.c.o)

all: libk.a

libk.a: $(OBJECTS)
	$(KERNEL_AR) rcs $@ $^

%.c.o: %.c
	$(KERNEL_CC) $(KERNEL_CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f libk.a

.PHONY: all clean
