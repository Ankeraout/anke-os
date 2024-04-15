# Configuration
CROSS_COMPILER := x86_64-elf-

# Common commands
MKDIR ?= mkdir

# Toolchain configuration
ASM := nasm
ASMFLAGS := -f elf64 -Iinclude
CC := $(CROSS_COMPILER)gcc -c
CFLAGS := -W -Wall -Wextra
CFLAGS += -std=gnu99 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pie -fno-pic -nostdlib -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mabi=sysv -mcmodel=small -march=x86-64 -m64
CFLAGS += -Os
CFLAGS += -Iinclude
LD := $(CROSS_COMPILER)ld
LDFLAGS := -T bootloader.ld
OBJCOPY := $(CROSS_COMPILER)objcopy

# Sources
SOURCES_ASM := $(shell find src -name '*.asm')
SOURCES_C := $(shell find src -name '*.c')

# Objects
OBJECTS_ASM := $(SOURCES_ASM:src/%.asm=obj/%.asm.o)
OBJECTS_C := $(SOURCES_C:src/%.c=obj/%.c.o)
OBJECTS := $(OBJECTS_ASM) $(OBJECTS_C)

all: bin/bootloader.bin

bin/bootloader.bin: bin/bootloader.elf
	$(OBJCOPY) -O binary $< $@

bin/bootloader.elf: $(OBJECTS)
	@if [ ! -d $(dir $@) ]; then \
		echo MKDIR $(dir $@); \
		$(MKDIR) -p $(dir $@); \
	fi
	@echo LD $@
	@$(LD) $(LDFLAGS) $(OBJECTS) -o $@

obj/%.asm.o: src/%.asm
	@if [ ! -d $(dir $@) ]; then \
		echo MKDIR $(dir $@); \
		$(MKDIR) -p $(dir $@); \
	fi
	@echo ASM $@
	@$(ASM) $(ASMFLAGS) $< -o $@

obj/%.c.o: src/%.c
	@if [ ! -d $(dir $@) ]; then \
		echo MKDIR $(dir $@); \
		$(MKDIR) -p $(dir $@); \
	fi
	@echo CC $@
	@$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) -rf bin obj
