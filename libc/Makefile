MAKEFLAGS += --no-builtin-rules

MKDIR := mkdir -p
RM := rm -rf
CC := x86_64-elf-gcc -c
LD := x86_64-elf-ld
AR := x86_64-elf-ar
CFLAGS := -I../include
CFLAGS += -MMD -MP
CFLAGS += -W -Wall -Wextra
CFLAGS += -std=gnu99 -pedantic-errors
CFLAGS += -g3 -O2
CFLAGS += -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pie -fno-pic -nostdlib -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mabi=sysv -mcmodel=kernel -march=x86-64 -m64

TARGET=../bin/libc/libc.a

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SOURCES := $(call rwildcard, ., *.c)
OBJECTS := $(patsubst ./%.c, ../obj/libc/%.c.o, $(SOURCES))
DIRECTORIES := $(patsubst ./%, ../obj/kernel/%, $(dir $(SOURCES)))
DEPENDENCIES := $(patsubst ./%.c, ../obj/libc/%.c.d, $(SOURCES))

all: $(TARGET)

dirs:
	$(MKDIR) ../bin/libc ../obj/libc

$(TARGET): dirs $(OBJECTS)
	$(AR) -crs $@ $(OBJECTS)

../obj/libc/%.c.o: ./%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) ../obj/libc ../bin/libc

-include $(DEPENDENCIES)

.PHONY: all dirs clean
