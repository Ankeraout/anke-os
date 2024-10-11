MKDIR := mkdir -p

MBR_ASM := nasm
MBR_ASMFLAGS := -Wall -f bin
VBR_ASM := nasm
VBR_ASMFLAGS := -Wall -f bin
BOOTLOADER_ASM := nasm
BOOTLOADER_ASMFLAGS := -Wall -f elf
BOOTLOADER_CC := i686-elf-gcc
BOOTLOADER_CFLAGS := -W -Wall -Wextra -Os -fno-builtin -nostdlib -ffreestanding -c
BOOTLOADER_LD := i686-elf-ld
BOOTLOADER_LDFLAGS := -T boot/loader/bootloader.ld

MBR_SOURCES := boot/mbr/mbr.asm

BOOTLOADER_SOURCES_ASM := $(shell find boot/loader -name '*.asm')
BOOTLOADER_SOURCES_C := $(shell find boot/loader -name '*.c')
BOOTLOADER_OBJECTS_ASM := $(patsubst %.asm,obj/%.asm.o,$(BOOTLOADER_SOURCES_ASM))
BOOTLOADER_OBJECTS_C := $(patsubst %.c,obj/%.c.o,$(BOOTLOADER_SOURCES_C))
BOOTLOADER_OBJECTS := $(BOOTLOADER_OBJECTS_ASM) $(BOOTLOADER_OBJECTS_C)

all: bin/fdd.img

bin/boot/mbr.bin: $(MBR_SOURCES)
	@if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(MBR_ASM) $(MBR_ASMFLAGS) $^ -o $@

bin/boot/vbr/%.bin: boot/vbr/%.asm
	@if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(VBR_ASM) $(VBR_ASMFLAGS) $^ -o $@

obj/boot/loader/%.asm.o: boot/loader/%.asm
	@if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(BOOTLOADER_ASM) $(BOOTLOADER_ASMFLAGS) $< -o $@

obj/boot/loader/%.c.o: boot/loader/%.c
	@if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(BOOTLOADER_CC) $(BOOTLOADER_CFLAGS) $< -o $@

bin/boot/loader.bin: $(BOOTLOADER_OBJECTS)
	@if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(BOOTLOADER_LD) $(BOOTLOADER_LDFLAGS) $^ -o $@

bin/fdd.img: bin/boot/vbr/fat12.bin bin/boot/loader.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	mformat -i $@ -B bin/boot/vbr/fat12.bin -f 1440 ::
	mcopy -i $@ bin/boot/loader.bin ::/BOOT.BIN

clean:
	rm -rf bin obj

.PHONY: all clean
.SUFFIXES:
