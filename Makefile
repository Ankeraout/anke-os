MKDIR := mkdir -p

MBR_ASM := nasm
MBR_ASMFLAGS := -Wall -f bin
VBR_ASM := nasm
VBR_ASMFLAGS := -Wall -f bin
BOOTLOADER_ASM := nasm
BOOTLOADER_ASMFLAGS := -Wall -f elf
BOOTLOADER_CC := i686-elf-gcc
BOOTLOADER_CFLAGS := -W -Wall -Wextra -Os -fno-builtin -nostdlib -ffreestanding -c -Iinclude
BOOTLOADER_LD := i686-elf-ld
BOOTLOADER_LDFLAGS := -T boot/loader/bootloader.ld
BOOTLOADER_OBJCOPY := i686-elf-objcopy

# Compiling with i686-elf-gcc-14.2.0 and linking with i686-elf-ld gives "cannot find -lgcc: file format not recognized".
# Compiling with i686-elf-gcc-14.2.0 and linking with it gives "collect2: fatal error: ld terminated with signal 11 [Segmentation fault]" (bug?)
# Compiling with i686-elf-gcc-14.2.0 and linking with i686-elf-ld with these flags works.
BOOTLOADER_LDFLAGS += -L$(HOME)/opt/cross/lib/gcc/i686-elf/14.2.0
BOOTLOADER_LIBS := -lgcc

MBR_SOURCES := boot/mbr/mbr.asm

BOOTLOADER_SOURCES_ASM := $(shell find boot/loader -name '*.asm')
BOOTLOADER_SOURCES_C := $(shell find boot/loader -name '*.c')
BOOTLOADER_OBJECTS_ASM := $(patsubst %.asm,obj/%.asm.o,$(BOOTLOADER_SOURCES_ASM))
BOOTLOADER_OBJECTS_C := $(patsubst %.c,obj/%.c.o,$(BOOTLOADER_SOURCES_C))
BOOTLOADER_OBJECTS := $(BOOTLOADER_OBJECTS_ASM) $(BOOTLOADER_OBJECTS_C)
BOOTLOADER_DEPENDENCIES := $(patsubst %.o,%.d,$(BOOTLOADER_OBJECTS))

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
	$(BOOTLOADER_CC) $(BOOTLOADER_CFLAGS) $< -o $@ -MMD

bin/boot/loader.bin: bin/boot/loader.elf
	$(BOOTLOADER_OBJCOPY) -O binary $< $@

bin/boot/loader.elf: $(BOOTLOADER_OBJECTS)
	@if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(BOOTLOADER_LD) $(BOOTLOADER_LDFLAGS) $^ -o $@ $(BOOTLOADER_LIBS)

bin/fdd.img: bin/boot/vbr/fat12.bin bin/boot/loader.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	mformat -i $@ -B bin/boot/vbr/fat12.bin -f 1440 ::
	mcopy -i $@ bin/boot/loader.bin ::/BOOT.BIN

clean:
	rm -rf bin obj

.PHONY: all clean
.SUFFIXES:

-include $(BOOTLOADER_DEPENDENCIES)
