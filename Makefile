all: bin/fdd.img

bin:
	mkdir -p $@

obj:
	mkdir -p $@

bin/boot: bin
	mkdir -p $@

bin/boot/vbr: bin/boot
	mkdir -p $@

bin/boot/mbr.bin: bin/boot boot/mbr/mbr.asm
	nasm -f bin boot/mbr/mbr.asm -o $@

bin/boot/vbr/fat12.bin: bin/boot/vbr boot/vbr/fat/fat12.asm boot/vbr/fat/header_common.inc boot/vbr/fat/header_fat1216.inc
	nasm -f bin boot/vbr/fat/fat12.asm -o $@

bin/boot/loader.bin: bin/boot boot/loader/bootstrap16.asm
	nasm -f bin boot/loader/bootstrap16.asm -o $@

bin/fdd.img: bin bin/boot/vbr/fat12.bin bin/boot/loader.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	mformat -i $@ -B bin/boot/vbr/fat12.bin -f 1440 ::
	mcopy -i $@ bin/boot/loader.bin ::/BOOT.BIN

clean:
	rm -rf bin obj

.PHONY: all clean
