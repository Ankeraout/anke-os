MAKEFLAGS+=--no-builtin-rules

BINDIR=bin
SRCDIR=src

EXEC=anke-os.img
KERNEL=kernel.elf

MBR_ASM=nasm
MBR_ASMFLAGS=-i $(SRCDIR)/mbr
MBR_EXEC=$(BINDIR)/mbr/floppy144_nofs.o

BOOTLOADER_ASM=nasm
BOOTLOADER_ASMFLAGS=-i $(SRCDIR)/bootloader
BOOTLOADER_EXEC=$(BINDIR)/bootloader/main.bin

all: $(EXEC)

$(EXEC): $(MBR_EXEC) $(BOOTLOADER_EXEC)
	cat $^ /dev/zero | dd of=$@ bs=512 count=2880
	
$(BINDIR):
	mkdir $@
	
$(BINDIR)/kernel: $(BINDIR)
	mkdir $@

$(BINDIR)/mbr: $(BINDIR)
	mkdir $@

$(BINDIR)/bootloader: $(BINDIR)
	mkdir $@

$(BINDIR)/mbr/%.o: $(SRCDIR)/mbr/%.asm $(BINDIR)/mbr
	$(MBR_ASM) $(MBR_ASMFLAGS) $< -o $@

$(BOOTLOADER_EXEC): $(SRCDIR)/bootloader/main.asm $(BINDIR)/bootloader
	$(BOOTLOADER_ASM) $(BOOTLOADER_ASMFLAGS) $< -o $@
	
clean:
	rm -rf $(BINDIR) $(KERNEL_OBJECTS)
