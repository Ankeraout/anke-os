MAKEFLAGS += --no-builtin-rules
XORRISO := xorriso
GRUB_MKRESCUE := grub-mkrescue
MKDIR := mkdir -p
RM := rm -rf
CP := cp

all: iso

src/kernel/bin/kernel: src/kernel/Makefile
	$(MAKE) -C src/kernel

obj/iso/%: limine-bootloader/%
	cp $< $@

obj/iso/limine.cfg: src/iso/limine.cfg
	cp $< $@

obj/iso/kernel.elf: src/kernel/bin/kernel
	cp $< $@

limine-bootloader/limine-deploy:
	make -C limine-bootloader

bin/anke-os.iso: obj/iso/limine-cd-efi.bin obj/iso/limine-cd.bin obj/iso/limine.sys obj/iso/limine.cfg obj/iso/kernel.elf limine-bootloader/limine-deploy
	xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-cd-efi.bin -efi-boot-part --efi-boot-image --protective-msdos-label obj/iso -o $@
	limine-bootloader/limine-deploy $@

iso: dirs bin/anke-os.iso

clean:
	$(RM) bin obj
	$(MAKE) -C src/kernel clean

dirs:
	$(MKDIR) bin obj/iso

.PHONY: all clean dirs iso
