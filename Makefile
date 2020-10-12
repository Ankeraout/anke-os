MAKEFLAGS+=--no-builtin-rules

SRCDIR=src
BINDIR=bin

KERNEL_CC=i686-elf-gcc -c
KERNEL_LD=i686-elf-ld
KERNEL_LDFLAGS=
KERNEL_CFLAGS=-W -Wall -Wextra -std=gnu11 -fno-builtin -nostdlib -g -O0 -ffreestanding
KERNEL_AS=nasm
KERNEL_ASFLAGS=-f elf

KERNEL_SOURCES_ASM=$(SRCDIR)/kernel/arch/i686/multiboot.asm
KERNEL_SOURCES_C=$(SRCDIR)/kernel/main.c
KERNEL_OBJECTS=$(KERNEL_SOURCES_ASM:%.asm=%.o) $(KERNEL_SOURCES_C:%.c=%.o)

KERNEL_EXEC=$(BINDIR)/kernel/kernel.elf

QEMU=qemu-system-x86_64
QEMUFLAGS=-m 64 -serial stdio -d cpu_reset
ISO=anke-os.iso

all: $(KERNEL_EXEC)

$(KERNEL_EXEC): $(BINDIR)/kernel $(KERNEL_OBJECTS)
	$(KERNEL_LD) $(KERNEL_LDFLAGS) $(KERNEL_OBJECTS) -o $(KERNEL_EXEC) -T $(SRCDIR)/kernel/arch/i686/linker.ld

$(BINDIR)/kernel: $(BINDIR)
	mkdir $(BINDIR)/kernel

$(BINDIR):
	mkdir $(BINDIR)

# Generic rule for compiling C code
$(SRCDIR)/kernel/%.o: $(SRCDIR)/kernel/%.c
	$(KERNEL_CC) $(KERNEL_CFLAGS) $< -o $@

$(SRCDIR)/kernel/%.o: $(SRCDIR)/kernel/%.asm
	$(KERNEL_AS) $(KERNEL_ASFLAGS) $< -o $@

iso: $(BINDIR)/$(ISO)

$(BINDIR)/$(ISO): $(KERNEL_EXEC) $(BINDIR)
	rm -rf $(BINDIR)/iso
	mkdir $(BINDIR)/iso
	mkdir $(BINDIR)/iso/boot
	mkdir $(BINDIR)/iso/boot/grub
	cp $(KERNEL_EXEC) $(BINDIR)/iso/boot
	cp $(SRCDIR)/grub/grub.cfg $(BINDIR)/iso/boot/grub
	grub-mkrescue $(BINDIR)/iso -o $@

clean:
	rm -rf $(BINDIR)
	rm -rf $(KERNEL_OBJECTS)
	rm -rf $(BINDIR)/$(ISO)

qemu_kernel: $(KERNEL_EXEC)
	$(QEMU) $(QEMUFLAGS) -kernel $(KERNEL_EXEC)

qemu_iso: $(BINDIR)/$(ISO)
	$(QEMU) $(QEMUFLAGS) -cdrom $(BINDIR)/$(ISO)

.PHONY: all clean qemu_kernel qemu_iso iso
