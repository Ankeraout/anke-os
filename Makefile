ASM ?= nasm
ASMFLAGS += 
MKDIR ?= mkdir

all: bin/anke-os.img

bin/anke-os.img: bin/boot/mbr/fat12.bin bin/boot/stage2/stage2.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	mformat -f 1440 -v ANKEOS86 -B bin/boot/mbr/fat12.bin -i $@
	mcopy -i $@ bin/boot/stage2/stage2.bin ::BOOT.BIN

bin/%.bin: src/%.asm
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) -p $(dir $@); \
	fi
	if [ ! -d $(dir $(patsubst bin/%.bin,obj/%.d,$@)) ]; then \
		$(MKDIR) -p $(dir $(patsubst bin/%.bin,obj/%.d,$@)); \
	fi
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	$(RM) -r bin obj

.PHONY: all clean
