KERNEL_CC := x86_64-elf-gcc
KERNEL_LD := x86_64-elf-ld
RM := rm -rf
MKDIR := mkdir -p
XORRISO := xorriso
CP := cp

KERNEL_LIBS := 
KERNEL_CFLAGS := \
	-c \
	-MMD -MP \
	-W -Wall -Wextra \
	-std=gnu99 -pedantic-errors \
	-g3 -O0 \
	-ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pie -fno-pic -nostdlib -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mabi=sysv -mcmodel=kernel -march=x86-64 -m64 \
	-I../include -I../limine-bootloader

KERNEL_LDFLAGS := \
	-nostdlib -static -m elf_x86_64 \
	-z max-page-size=0x1000 \
	-T src/kernel/kernel.ld \
	$(KERNEL_LIBS:%=-l%)

KERNEL_SOURCES_C := $(shell find src/kernel -name '*.c')
KERNEL_OBJECTS := $(KERNEL_SOURCES_C:src/kernel/%.c=obj/kernel/%.c.o)
KERNEL_DEPENDENCIES := $(KERNEL_OBJECTS:obj/kernel/%.c.o=obj/kernel/%.c.d)

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

clean:
	$(RM) obj bin

.PHONY: all clean

-include $(KERNEL_DEPENDENCIES)
