MKDIR := mkdir

all: bin/anke-os.img

bin/anke-os.img: mbr/bin/fat12.bin bootloader/bin/bootloader.bin
	@if [ ! -d $(dir $@) ]; then \
		echo MKDIR $(dir $@); \
		$(MKDIR) -p $(dir $@); \
	fi

	dd if=/dev/zero of=$@ bs=512 count=2880
	mformat -f 1440 -v ANKEOS86 -B mbr/bin/fat12.bin -i $@
	mcopy -i $@ bootloader/bin/bootloader.bin ::BOOT.BIN

mbr/%:
	$(MAKE) -C mbr $($@:mbr/%=%)

bootloader/%:
	$(MAKE) -C bootloader $($@:bootloader/%=%)

clean:
	$(RM) -r bin obj
	$(MAKE) -C bootloader clean
	$(MAKE) -C mbr clean

.PHONY: all clean
