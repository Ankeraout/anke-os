MAKEFLAGS += --no-builtin-rules
GRUB_MKRESCUE := grub-mkrescue
MKDIR := mkdir -p
RM := rm -rf
CP := cp

all: iso

src/kernel/bin/kernel: src/kernel/Makefile
	$(MAKE) -C src/kernel

obj/iso/boot/grub/grub.cfg: src/iso/boot/grub/grub.cfg
	cp $< $@

obj/iso/boot/kernel: src/kernel/bin/kernel
	cp $< $@

bin/anke-os.iso: obj/iso/boot/grub/grub.cfg obj/iso/boot/kernel
	grub-mkrescue obj/iso -o $@

iso: dirs bin/anke-os.iso

clean:
	$(RM) bin obj
	$(MAKE) -C src/kernel clean

dirs:
	$(MKDIR) bin obj/iso/boot/grub

.PHONY: all clean dirs iso
