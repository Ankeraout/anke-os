MAKEFLAGS += --no-builtin-rules

MKDIR := mkdir -p
RM := rm -rf
CC := i686-elf-gcc -c
LD := i686-elf-gcc
ASM := nasm
ASMFLAGS := -f elf -g

CFLAGS += -MMD -MP
CFLAGS += -W -Wall -Wextra
CFLAGS += -std=gnu99 -pedantic-errors
CFLAGS += -g3 -O0
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -Isrc
LDFLAGS += -T src/kernel.ld
LDFLAGS += -g3 -O0
LDFLAGS += -ffreestanding -nostdlib

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SOURCES_C := $(call rwildcard, src, *.c)
SOURCES_ASM := $(call rwildcard, src, *.asm)
SOURCES := $(SOURCES_C) $(SOURCES_ASM)
OBJECTS_C := $(patsubst src/%.c, obj/%.c.o, $(SOURCES_C))
OBJECTS_ASM := $(patsubst src/%.asm, obj/%.asm.o, $(SOURCES_ASM))
OBJECTS := $(OBJECTS_C) $(OBJECTS_ASM)
DIRECTORIES := $(patsubst src/%, obj/%, $(dir $(SOURCES)))
EXECUTABLE := bin/kernel
DEPENDENCIES := $(patsubst src/%.c, obj/%.c.d, $(SOURCES_C))

all: dirs $(EXECUTABLE)

obj/%.c.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

obj/%.asm.o: src/%.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@ -lgcc

clean:
	$(RM) bin obj

-include $(DEPENDENCIES)

dirs:
	$(MKDIR) bin $(DIRECTORIES)

.PHONY: all clean dirs
