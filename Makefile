KERNEL_CC := x86_64-elf-gcc
KERNEL_LD := x86_64-elf-ld
RM := rm -rf
MKDIR := mkdir -p
XORRISO := xorriso
CP := cp

KERNEL_LIBS := gcc
KERNEL_CFLAGS := -c \
	-W -Wall -Wextra \
	-std=gnu17 -pedantic \
	-ffreestanding \
	-mcmodel=kernel \
	-mno-red-zone \
	-mno-mmx -mno-sse -mno-sse2
KERNEL_LDFLAGS := -ffreestanding -nostdlib $(KERNEL_LIBS:%=-l%)

all: bin/anke-os.iso

bin/anke-os.iso: obj/iso/limine-uefi-cd.bin obj/iso/limine-bios-cd.bin obj/iso/limine-bios.sys obj/iso/limine.cfg limine/limine
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
	$(CP) $< $@

obj/iso/%: src/iso/%
	$(CP) $< $@

clean:
	$(RM) obj bin

.PHONY: all clean
