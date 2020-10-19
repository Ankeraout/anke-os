MAKEFLAGS+=--no-builtin-rules

SRCDIR=src
BINDIR=bin

KERNEL_CC=i686-elf-gcc -c
KERNEL_LD=i686-elf-gcc
KERNEL_LDFLAGS=-fno-builtin -nostdlib -ffreestanding -g -O0 -lgcc
KERNEL_CFLAGS=-W -Wall -Wextra -std=gnu11 -fno-builtin -nostdlib -g -O0 -ffreestanding -Isrc/kernel
KERNEL_INTHLDR_CFLAGS=-mgeneral-regs-only
KERNEL_AS=nasm
KERNEL_ASFLAGS=-f elf

KERNEL_SOURCES_ASM=	\
	$(SRCDIR)/kernel/arch/i686/multiboot.asm \

KERNEL_SOURCES_C_INTHDLR=	\
	$(SRCDIR)/kernel/arch/i686/isr.c

KERNEL_SOURCES_C=	\
	$(SRCDIR)/kernel/debug.c \
	$(SRCDIR)/kernel/main.c \
	$(SRCDIR)/kernel/panic.c \
	$(SRCDIR)/kernel/acpi/rsdp.c \
	$(SRCDIR)/kernel/arch/i686/idt.c \
	$(SRCDIR)/kernel/arch/i686/pic.c \
	$(SRCDIR)/kernel/libk/libk.c \
	$(SRCDIR)/kernel/mm/mm.c \
	$(SRCDIR)/kernel/mm/pmm.c \
	$(SRCDIR)/kernel/mm/vmm.c \
	$(SRCDIR)/kernel/tty/tty.c \
	$(KERNEL_SOURCES_C_INTHDLR)

KERNEL_OBJECTS=$(KERNEL_SOURCES_ASM:%.asm=%.o) $(KERNEL_SOURCES_C:%.c=%.o)

KERNEL_EXEC=$(BINDIR)/kernel/kernel.elf

QEMU=qemu-system-x86_64
QEMUFLAGS=-m 2 -serial stdio -d cpu_reset --enable-kvm
ISO=anke-os.iso

all: $(KERNEL_EXEC)

$(KERNEL_EXEC): $(BINDIR)/kernel $(KERNEL_OBJECTS)
	$(KERNEL_LD) -T $(SRCDIR)/kernel/arch/i686/linker.ld $(KERNEL_OBJECTS) $(KERNEL_LDFLAGS) -o $(KERNEL_EXEC)

$(BINDIR)/kernel: $(BINDIR)
	mkdir $(BINDIR)/kernel

$(BINDIR):
	mkdir $(BINDIR)

# Generic rule for compiling C code
$(SRCDIR)/kernel/%.o: $(SRCDIR)/kernel/%.c
	if [ "$(KERNEL_SOURCES_C_INTHDLR)" == *"$<"* ]; then \
		$(KERNEL_CC) $(KERNEL_CFLAGS) $(KERNEL_INTHLDR_CFLAGS) $< -o $@; \
		echo "Compiled with interrupt handler flags"; \
	else \
		$(KERNEL_CC) $(KERNEL_CFLAGS) $< -o $@; \
	fi

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
