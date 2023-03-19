MAKEFLAGS += --no-builtin-rules

MKDIR := mkdir -p
RM := rm -rf
CC := x86_64-elf-gcc -c
LD := x86_64-elf-ld
ASM := nasm

ASMFLAGS := -f elf64 -g
CFLAGS += -D__BOOT_LIMINE
CFLAGS += -MMD -MP
CFLAGS += -W -Wall -Wextra
CFLAGS += -std=gnu99 -pedantic-errors
CFLAGS += -g3 -O0
CFLAGS += -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pie -fno-pic -nostdlib -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mabi=sysv -mcmodel=kernel -march=x86-64 -m64
CFLAGS += -I../../include
LDFLAGS += -nostdlib -static -m elf_x86_64
LDFLAGS += -z max-page-size=0x1000
LDFLAGS += -r

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SOURCES_C := $(call rwildcard, ., *.c)
SOURCES_ASM := $(call rwildcard, ., *.asm)
SOURCES := $(SOURCES_C) $(SOURCES_ASM)
OBJECTS_C := $(patsubst ./%.c, ../../obj/modules/floppy/%.c.o, $(SOURCES_C))
OBJECTS_ASM := $(patsubst ./%.asm, ../../obj/modules/floppy/%.asm.o, $(SOURCES_ASM))
OBJECTS := $(OBJECTS_C) $(OBJECTS_ASM)
DIRECTORIES := $(patsubst ./%, ../../obj/modules/floppy/%, $(dir $(SOURCES)))
EXECUTABLE := ../../bin/modules/floppy.elf
DEPENDENCIES := $(patsubst ./%.c, ../../obj/modules/floppy/%.c.d, $(SOURCES_C))

all: dirs $(EXECUTABLE)

../../obj/modules/floppy/%.c.o: ./%.c
	$(CC) $(CFLAGS) $< -o $@

../../obj/modules/floppy/%.asm.o: ./%.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)

-include $(DEPENDENCIES)

dirs:
	$(MKDIR) ../../bin/modules $(DIRECTORIES)

.PHONY: all clean dirs
