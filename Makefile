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

MODULES_KERNEL := ata cmos floppy framebuffer mbrfs pci pciide tty
MODULES_KERNEL_EXECUTABLES := $(foreach module,$(MODULES_KERNEL),bin/modules/$(module).elf)
MODULES_ISO :=
MODULES_ISO_EXECUTABLES := $(foreach module,$(MODULES_ISO),bin/modules/$(module).elf)
MODULES_ISO_TARGET := $(foreach module,$(MODULES_ISO),obj/iso/modules/$(module).elf)
MODULES := $(MODULES_KERNEL) $(MODULES_ISO)
MODULES_EXECUTABLES := $(MODULES_KERNEL_EXECUTABLES) $(MODULES_ISO_EXECUTABLES)

export MODULES_KERNEL

all: cdrom

kernel: $(TARGET_KERNEL)
libc: $(TARGET_LIBC)
cdrom: $(TARGET_CDROM)
modules: libc $(MODULES_EXECUTABLES)

$(TARGET_LIBC): dirs libc/Makefile
	$(MAKE) -C libc

$(TARGET_KERNEL): modules dirs $(TARGET_LIBC) kernel/Makefile
	$(MAKE) -C kernel

$(TARGET_CDROM): dirs obj/iso/limine-cd-efi.bin obj/iso/limine-cd.bin obj/iso/limine.sys obj/iso/limine.cfg limine-bootloader/limine-deploy obj/iso/kernel.elf $(MODULES_ISO_TARGET)
	xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-cd-efi.bin -efi-boot-part --efi-boot-image --protective-msdos-label obj/iso -o $@
	limine-bootloader/limine-deploy $@

obj/iso/%: limine-bootloader/%
	cp $< $@

obj/iso/%: iso/%
	cp $< $@

obj/iso/kernel.elf: bin/kernel/kernel.elf
	cp $< $@

obj/iso/modules/%: bin/modules/%
	cp $< $@

bin/modules/%.elf: modules/%/Makefile
	$(MAKE) -C $(dir $<)

limine-bootloader/limine-deploy:
	make -C limine-bootloader

clean:
	$(RM) bin obj

dirs:
	$(MKDIR) obj/iso bin/iso obj/iso/modules bin/modules obj/modules

.PHONY: all clean dirs cdrom libc kernel modules
