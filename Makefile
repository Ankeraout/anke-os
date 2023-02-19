MAKEFLAGS += --no-builtin-rules

XORRISO := xorriso
GRUB_MKRESCUE := grub-mkrescue
MKDIR := mkdir -p
RM := rm -rf
CP := cp

TARGET_KERNEL := bin/kernel/kernel.elf
TARGET_LIBC := bin/libc/libc.a
TARGET_CDROM := bin/iso/anke-os.iso
DEPS_CDROM := obj/iso/limine.cfg obj/iso/boot/kernel.elf

all: cdrom

kernel: $(TARGET_KERNEL)
libc: $(TARGET_LIBC)
cdrom: $(TARGET_CDROM)

$(TARGET_LIBC): dirs libc/Makefile
	$(MAKE) -C libc

$(TARGET_KERNEL): dirs $(TARGET_LIBC) kernel/Makefile
	$(MAKE) -C kernel

$(TARGET_CDROM): dirs obj/iso/limine-cd-efi.bin obj/iso/limine-cd.bin obj/iso/limine.sys obj/iso/limine.cfg obj/iso/kernel.elf limine-bootloader/limine-deploy
	xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-cd-efi.bin -efi-boot-part --efi-boot-image --protective-msdos-label obj/iso -o $@
	limine-bootloader/limine-deploy $@

obj/iso/%: limine-bootloader/%
	cp $< $@

obj/iso/%: iso/%
	cp $< $@

obj/iso/kernel.elf: bin/kernel/kernel.elf
	cp $< $@

limine-bootloader/limine-deploy:
	make -C limine-bootloader

clean:
	$(RM) bin obj

dirs:
	$(MKDIR) obj/iso bin/iso

.PHONY: all clean dirs cdrom libc kernel
