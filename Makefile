# Configuration
CROSS_COMPILER := x86_64-elf-

# Common commands
MKDIR ?= mkdir

# MBR configuration
MBR_ASM := nasm
MBR_ASMFLAGS := -Isrc

# Bootloader configuration
BOOTLOADER_ASM := nasm
BOOTLOADER_ASMFLAGS := -Isrc -f elf64
BOOTLOADER_CC := $(CROSS_COMPILER)gcc -c
BOOTLOADER_CFLAGS := -W -Wall -Wextra -std=gnu99 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pie -fno-pic -nostdlib -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mabi=sysv -mcmodel=small -march=x86-64 -m64 -Os -Isrc/boot/stage2
BOOTLOADER_OBJECTS := \
	obj/boot/stage2/bootstrap16.o \
	obj/boot/stage2/bootstrap32.o \
	obj/boot/stage2/bootstrap64.o \
	obj/boot/stage2/main.o \
	obj/boot/stage2/stdio.o
BOOTLOADER_LD := $(CROSS_COMPILER)ld
BOOTLOADER_LDFLAGS := -T src/boot/stage2/stage2.ld
BOOTLOADER_OBJCOPY := $(CROSS_COMPILER)objcopy

all: bin/anke-os.img

bin/anke-os.img: bin/boot/mbr/fat12.bin bin/boot/stage2.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	mformat -f 1440 -v ANKEOS86 -B bin/boot/mbr/fat12.bin -i $@
	mcopy -i $@ bin/boot/stage2.bin ::BOOT.BIN

bin/boot/%.bin: bin/boot/%.elf
	$(BOOTLOADER_OBJCOPY) -O binary $< $@

bin/boot/stage2.elf: $(BOOTLOADER_OBJECTS)
	$(BOOTLOADER_LD) $(BOOTLOADER_LDFLAGS) $(BOOTLOADER_OBJECTS) -o $@

obj/boot/stage2/%.o: src/boot/stage2/%.asm
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) -p $(dir $@); \
	fi
	$(BOOTLOADER_ASM) $(BOOTLOADER_ASMFLAGS) $< -o $@

obj/boot/stage2/%.o: src/boot/stage2/%.c
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) -p $(dir $@); \
	fi
	$(BOOTLOADER_CC) $(BOOTLOADER_CFLAGS) $< -o $@

bin/boot/mbr/%.bin: src/boot/mbr/%.asm
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) -p $(dir $@); \
	fi
	$(MBR_ASM) $(MBR_ASMFLAGS) -f bin $< -o $@

clean:
	$(RM) -r bin obj

.PHONY: all clean
