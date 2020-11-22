MAKEFLAGS+=--no-builtin-rules

SRCDIR=src
BINDIR=bin

KERNEL_CPP=i686-elf-g++ -c
KERNEL_LD=i686-elf-g++
KERNEL_LDFLAGS=-fno-builtin -nostdlib -ffreestanding -g -O0 -lgcc
KERNEL_CPPFLAGS=-W -Wall -Wextra -std=gnu++17 -fno-builtin -nostdlib -g -O0 -ffreestanding -Isrc/kernel -fno-exceptions -fno-rtti -mgeneral-regs-only
KERNEL_AS=nasm
KERNEL_ASFLAGS=-f elf
KERNEL_CRTBEGIN=$(shell $(KERNEL_CPP) $(KERNEL_CPPFLAGS) -print-file-name=crtbegin.o)
KERNEL_CRTEND=$(shell $(KERNEL_CPP) $(KERNEL_CPPFLAGS) -print-file-name=crtend.o)
KERNEL_CRTI=$(SRCDIR)/kernel/crti.o
KERNEL_CRTN=$(SRCDIR)/kernel/crtn.o

KERNEL_SOURCES_ASM=	\
	$(SRCDIR)/kernel/arch/i686/isr.asm \
	$(SRCDIR)/kernel/arch/i686/multiboot.asm \
	$(SRCDIR)/kernel/arch/i686/mutex.asm \
	$(SRCDIR)/kernel/arch/i686/ring3.asm \
	$(SRCDIR)/kernel/arch/i686/syscall.asm

KERNEL_SOURCES_CPP= \
	$(SRCDIR)/kernel/cpp.cpp \
	$(SRCDIR)/kernel/debug.cpp \
	$(SRCDIR)/kernel/main.cpp \
	$(SRCDIR)/kernel/panic.cpp \
	$(SRCDIR)/kernel/syscall.cpp \
	$(SRCDIR)/kernel/acpi/acpi.cpp \
	$(SRCDIR)/kernel/acpi/rsdp.cpp \
	$(SRCDIR)/kernel/arch/i686/idt.cpp \
	$(SRCDIR)/kernel/arch/i686/pic.cpp \
	$(SRCDIR)/kernel/arch/i686/tss.cpp \
	$(SRCDIR)/kernel/driver/eth.cpp \
	$(SRCDIR)/kernel/driver/pci.cpp \
	$(SRCDIR)/kernel/driver/rtl8139.cpp \
	$(SRCDIR)/kernel/libk/libk.cpp \
	$(SRCDIR)/kernel/mm/mm.cpp \
	$(SRCDIR)/kernel/mm/pmm.cpp \
	$(SRCDIR)/kernel/mm/vmm.cpp \
	$(SRCDIR)/kernel/tty/tty.cpp

KERNEL_OBJECTS=$(KERNEL_SOURCES_ASM:%.asm=%.o) $(KERNEL_SOURCES_C:%.c=%.o) $(KERNEL_SOURCES_CPP:%.cpp=%.o)

KERNEL_EXEC=$(BINDIR)/kernel/kernel.elf

QEMU=qemu-system-i386
QEMUFLAGS=-m 32 -serial stdio -d cpu_reset -net nic,model=rtl8139
ISO=anke-os.iso

all: $(KERNEL_EXEC)

$(KERNEL_EXEC): $(BINDIR)/kernel $(KERNEL_OBJECTS) $(KERNEL_CRTI) $(KERNEL_CRTN)
	$(KERNEL_LD) -T $(SRCDIR)/kernel/arch/i686/linker.ld $(KERNEL_CRTI) $(KERNEL_CRTBEGIN) $(KERNEL_OBJECTS) $(KERNEL_CRTEND) $(KERNEL_CRTN) $(KERNEL_LDFLAGS) -o $(KERNEL_EXEC)

$(BINDIR)/kernel: $(BINDIR)
	mkdir $(BINDIR)/kernel

$(BINDIR):
	mkdir $(BINDIR)

$(SRCDIR)/kernel/%.o: $(SRCDIR)/kernel/%.cpp
	if [ "$(KERNEL_SOURCES_CPP_INTHDLR)" == *"$<"* ]; then \
		$(KERNEL_CPP) $(KERNEL_CPPFLAGS) $(KERNEL_INTHLDR_CPPFLAGS) $< -o $@; \
	else \
		$(KERNEL_CPP) $(KERNEL_CPPFLAGS) $< -o $@; \
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

qemu_kernel_debug: $(KERNEL_EXEC)
	$(QEMU) $(QEMUFLAGS) -kernel $(KERNEL_EXEC) -S -s

qemu_iso: $(BINDIR)/$(ISO)
	$(QEMU) $(QEMUFLAGS) -cdrom $(BINDIR)/$(ISO)

.PHONY: all clean qemu_kernel qemu_iso iso
