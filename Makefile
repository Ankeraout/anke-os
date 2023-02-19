MAKEFLAGS += --no-builtin-rules
XORRISO := xorriso
GRUB_MKRESCUE := grub-mkrescue
MKDIR := mkdir -p
RM := rm -rf
CP := cp

KERNEL_EXECUTABLE=kernel/bin/kernel.elf

all: cdrom

$(KERNEL_EXECUTABLE): kernel/Makefile
	$(MAKE) -C kernel

obj/iso/%: limine-bootloader/%
	cp $< $@

obj/iso/%: iso/%
	cp $< $@

obj/iso/%: kernel/bin/%
	cp $< $@

limine-bootloader/limine-deploy:
	make -C limine-bootloader

bin/anke-os.iso: obj/iso/limine-cd-efi.bin obj/iso/limine-cd.bin obj/iso/limine.sys obj/iso/limine.cfg obj/iso/kernel.elf limine-bootloader/limine-deploy
	xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-cd-efi.bin -efi-boot-part --efi-boot-image --protective-msdos-label obj/iso -o $@
	limine-bootloader/limine-deploy $@

cdrom: dirs bin/anke-os.iso

clean:
	$(RM) bin obj
	$(MAKE) -C kernel clean

dirs:
	$(MKDIR) bin obj/iso

.PHONY: all clean dirs cdrom
