MAKEFLAGS+=--no-builtin-rules

CONFIG=Makefile.config

include ${CONFIG}

all: kernel

kernel: src/kernel/kernel

src/kernel/kernel:
	$(MAKE) -C src/kernel

clean:
	$(MAKE) -C src/kernel clean
	rm -f anke-os.iso

iso: anke-os.iso

anke-os.iso: src/grub/grub.cfg src/kernel/kernel
	rm -rf /tmp/anke-os-iso
	mkdir -p /tmp/anke-os-iso/boot/grub
	cp src/grub/grub.cfg /tmp/anke-os-iso/boot/grub/grub.cfg
	cp src/kernel/kernel /tmp/anke-os-iso/boot/kernel
	grub-mkrescue -o anke-os.iso /tmp/anke-os-iso
	
qemu_kernel: src/kernel/kernel
	qemu-system-i386 -m 32 -kernel src/kernel/kernel

qemu_iso: iso
	qemu-system-i386 -m 32 -cdrom anke-os.iso

.PHONY: clean iso kernel qemu_kernel qemu_iso
