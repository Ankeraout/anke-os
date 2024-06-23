MKDIR := mkdir

ASM_MBR := nasm
ASMFLAGS_MBR := -f bin
ASM_LOADER := nasm
ASMFLAGS_LOADER := -f elf32 -Iboot/loader
CC_LOADER := i686-elf-gcc -c
LD_LOADER := i686-elf-ld
OBJCOPY := i686-elf-objcopy
CFLAGS_LOADER := -W -Wall -Wextra -Os -ffreestanding -Iboot/loader
LDFLAGS_LOADER :=
SOURCES_ASM_LOADER := \
	boot/loader/arch/x86/bootstrap16/main.asm \
	boot/loader/arch/x86/bioscall.asm \
	boot/loader/arch/x86/isr.asm
OBJECTS_ASM_LOADER := $(SOURCES_ASM_LOADER:boot/loader/%.asm=obj/boot/loader/%.asm.o)
SOURCES_C_LOADER := \
	boot/loader/main.c \
	boot/loader/libc/stdio.c \
	boot/loader/libc/string.c \
	boot/loader/arch/x86/idt.c \
	boot/loader/arch/x86/irq.c \
	boot/loader/drivers/irq/i8259.c
OBJECTS_C_LOADER := $(SOURCES_C_LOADER:boot/loader/%.c=obj/boot/loader/%.c.o)
OBJECTS_LOADER := $(OBJECTS_ASM_LOADER) $(OBJECTS_C_LOADER)
DEPENDENCIES_LOADER := $(OBJECTS_LOADER:%.o=%.d)

all: bin/anke-os.img

bin/anke-os.img: bin/boot/mbr/fat12.bin bin/boot/loader.bin
	@if [ ! -d $(dir $@) ]; then \
		echo MKDIR $(dir $@); \
		$(MKDIR) -p $(dir $@); \
	fi

	dd if=/dev/zero of=$@ bs=512 count=2880
	mformat -f 1440 -v ANKEOS86 -B bin/boot/mbr/fat12.bin -i $@
	mcopy -i $@ bin/boot/loader.bin ::BOOT.BIN

bin/boot/mbr/%.bin: boot/mbr/%.asm
	@if [ ! -d $(dir $@) ]; then \
		echo MKDIR $(dir $@); \
		$(MKDIR) -p $(dir $@); \
	fi

	$(ASM_MBR) $(ASMFLAGS_MBR) $< -o $@ -Iboot/mbr

bin/boot/loader.bin: bin/boot/loader.elf
	$(OBJCOPY) -O binary $< $@

bin/boot/loader.elf: $(OBJECTS_LOADER) boot/loader/loader.ld
	@if [ ! -d $(dir $@) ]; then \
		echo MKDIR $(dir $@); \
		$(MKDIR) -p $(dir $@); \
	fi

	$(LD_LOADER) $(OBJECTS_LOADER) -o $@ -T boot/loader/loader.ld

obj/boot/loader/%.asm.o: boot/loader/%.asm
	@if [ ! -d $(dir $@) ]; then \
		echo MKDIR $(dir $@); \
		$(MKDIR) -p $(dir $@); \
	fi

	$(ASM_LOADER) $(ASMFLAGS_LOADER) $< -o $@

obj/boot/loader/%.c.o: boot/loader/%.c
	@if [ ! -d $(dir $@) ]; then \
		echo MKDIR $(dir $@); \
		$(MKDIR) -p $(dir $@); \
	fi

	$(CC_LOADER) $(CFLAGS_LOADER) $< -o $@

clean:
	$(RM) -r bin obj

.PHONY: all clean

-include $(DEPENDENCIES_LOADER)
