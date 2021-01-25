MAKEFLAGS+=--no-builtin-rules

SRCDIR=src
BINDIR=bin

CONFIG=Makefile.config
include ${CONFIG}

KERNEL_SOURCES_ASM+= \


KERNEL_SOURCES_C+= \
	$(SRCDIR)/kernel/irq.c \
	$(SRCDIR)/kernel/main.c \
	$(SRCDIR)/kernel/panic.c \
	$(SRCDIR)/kernel/syscall.c \
	$(SRCDIR)/kernel/tty.c \
	$(SRCDIR)/kernel/acpi/acpi.c \
	$(SRCDIR)/kernel/acpi/madt.c \
	$(SRCDIR)/kernel/acpi/rsdp.c \
	$(SRCDIR)/kernel/acpi/sdt.c \
	$(SRCDIR)/kernel/libk/stdio.c \
	$(SRCDIR)/kernel/libk/string.c

KERNEL_OBJECTS=$(KERNEL_SOURCES_ASM:%.asm=%.asm.o) $(KERNEL_SOURCES_C:%.c=%.c.o)

KERNEL_EXEC=$(BINDIR)/kernel/kernel.elf

QEMU=qemu-system-i386
QEMUFLAGS=-m 32 -serial stdio -d cpu_reset -net nic,model=rtl8139 -smp 2
ISO=anke-os.iso

all: $(KERNEL_EXEC)

$(KERNEL_EXEC): $(BINDIR)/kernel $(KERNEL_OBJECTS)
	$(KERNEL_LD) $(KERNEL_OBJECTS) $(KERNEL_LDFLAGS) -o $(KERNEL_EXEC)
	grub-file --is-x86-multiboot $@

$(BINDIR)/kernel: $(BINDIR)
	mkdir $(BINDIR)/kernel

$(BINDIR):
	mkdir $(BINDIR)

$(SRCDIR)/kernel/%.c.o: $(SRCDIR)/kernel/%.c
	$(KERNEL_CC) $(KERNEL_CFLAGS) $< -o $@

$(SRCDIR)/kernel/%.asm.o: $(SRCDIR)/kernel/%.asm
	$(KERNEL_ASM) $(KERNEL_ASMFLAGS) $< -o $@

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

qemu_kernel_debug: $(KERNEL_EXEC)
	$(QEMU) $(QEMUFLAGS) -kernel $(KERNEL_EXEC) -S -s

qemu_iso: $(BINDIR)/$(ISO)
	$(QEMU) $(QEMUFLAGS) -cdrom $(BINDIR)/$(ISO)

.PHONY: all clean qemu_kernel qemu_iso iso
