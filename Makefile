ASM := nasm
ASMFLAGS := -Wall

all: bin/anke-os.img

bin/anke-os.img: obj/boot/vbr/fat12.bin obj/kernel/kernel.bin
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	dd if=/dev/zero of=$@ bs=512 count=2880
	mformat -i $@ -B obj/boot/vbr/fat12.bin -f 1440 ::
	mcopy -i $@ obj/kernel/kernel.bin ::/BOOT.BIN

obj/%.bin: %.asm
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(ASM) -f bin $< -o $@

clean:
	rm -rf obj bin

.PHONY: all clean
