KERNEL_ASM := nasm
KERNEL_CC := x86_64-elf-gcc
KERNEL_LD := x86_64-elf-ld
RM := rm -rf
MKDIR := mkdir -p
XORRISO := xorriso
CP := cp

KERNEL_LIBS := 
KERNEL_ASMFLAGS := -f elf64
KERNEL_CFLAGS := \
	-c \
	-MMD -MP \
	-W -Wall -Wextra \
	-std=gnu99 -pedantic-errors \
	-g3 -O0 \
	-ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pie -fno-pic -nostdlib -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mabi=sysv -mcmodel=kernel -march=x86-64 -m64 \
	-Iinclude -Ilimine
KERNEL_LDFLAGS := \
	-nostdlib -static -m elf_x86_64 \
	-z max-page-size=0x1000 \
	-T src/kernel/kernel.ld \
	$(KERNEL_LIBS:%=-l%)

KERNEL_SOURCES_C := $(shell find src/kernel -name '*.c')
KERNEL_SOURCES_ASM := $(shell find src/kernel -name '*.asm')
KERNEL_OBJECTS_C := $(KERNEL_SOURCES_C:src/kernel/%.c=obj/kernel/%.c.o)
KERNEL_OBJECTS_ASM := $(KERNEL_SOURCES_ASM:src/kernel/%.asm=obj/kernel/%.asm.o)
KERNEL_OBJECTS := $(KERNEL_OBJECTS_C) $(KERNEL_OBJECTS_ASM)
KERNEL_DEPENDENCIES_C := $(KERNEL_OBJECTS_C:obj/kernel/%.c.o=obj/kernel/%.c.d)
KERNEL_DEPENDENCIES_ASM := $(KERNEL_OBJECTS_ASM:obj/kernel/%.asm.o=obj/kernel/%.asm.d)
KERNEL_DEPENDENCIES := $(KERNEL_DEPENDENCIES_C) $(KERNEL_DEPENDENCIES_ASM)

all: bin/anke-os.iso

bin/anke-os.iso: obj/iso/limine-uefi-cd.bin obj/iso/limine-bios-cd.bin obj/iso/limine-bios.sys obj/iso/limine.cfg limine/limine obj/iso/boot/kernel.elf
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(XORRISO) -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label obj/iso -o $@
	limine/limine bios-install $@

obj/iso/%: limine/%
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(CP) $< $@

obj/iso/%: iso/%
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(CP) $< $@

obj/iso/%: src/iso/%
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(CP) $< $@

obj/iso/boot/kernel.elf: bin/kernel.elf
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(CP) $< $@

bin/kernel.elf: $(KERNEL_OBJECTS)
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(KERNEL_LD) $^ -o $@ $(KERNEL_LDFLAGS)

obj/kernel/%.c.o: src/kernel/%.c
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(KERNEL_CC) $< -o $@ $(KERNEL_CFLAGS)

obj/kernel/%.asm.o: src/kernel/%.asm
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(KERNEL_ASM) $(KERNEL_ASMFLAGS) $< -o $@ -MD $(@:obj/kernel/%.asm.o=obj/kernel/%.asm.d)

clean:
	$(RM) obj bin

.PHONY: all clean

-include $(KERNEL_DEPENDENCIES)
