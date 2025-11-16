CROSS_COMPILE := x86_64-elf-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
ASM := nasm
MKDIR := mkdir -p
RM := rm -rf

ASMFLAGS := -f elf64 -Iinclude
CFLAGS := \
	-c \
	-MMD -MP \
	-W -Wall -Wextra \
	-std=gnu99 -pedantic-errors \
	-g3 -Og \
	-ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pie -fno-pic -nostdlib -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mabi=sysv -mcmodel=kernel -march=x86-64 -m64 \
	-Iinclude -I../limine \
	-DCONFIG_ARCH_AMD64 \
	-DCONFIG_AMD64_BOOTSTRAP_LIMINE \
	-DCONFIG_MAX_CORE_COUNT=1

LDFLAGS := \
	-nostdlib -static -m elf_x86_64 \
	-z max-page-size=0x1000 \
	-T kernel.ld \

SOURCES_C := $(shell find src -name '*.c')
SOURCES_ASM := $(shell find src -name '*.asm')
OBJECTS_C := $(SOURCES_C:src/%.c=obj/%.c.o)
OBJECTS_ASM := $(SOURCES_ASM:src/%.asm=obj/%.asm.o)
OBJECTS := $(OBJECTS_C) $(OBJECTS_ASM)
DEPENDENCIES_C := $(OBJECTS_C:obj/%.c.o=obj/%.c.d)
DEPENDENCIES_ASM := $(OBJECTS_ASM:obj/%.asm.o=obj/%.asm.d)
DEPENDENCIES := $(DEPENDENCIES_C) $(DEPENDENCIES_ASM)

all: bin/kernel.elf

bin/kernel.elf: $(OBJECTS)
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(LD) $(LDFLAGS) -o $@ $^

obj/%.c.o: src/%.c
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(CC) $(CFLAGS) -o $@ $<

obj/%.asm.o: src/%.asm
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(ASM) $(ASMFLAGS) -o $@ $<

clean:
	$(RM) bin obj

.PHONY: all clean

-include $(DEPENDENCIES)
