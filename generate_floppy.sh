./configure --bootstrap=aout
make clean
make all
nasm src/mbr/floppy_nofs.asm
cat src/mbr/floppy_nofs bin/kernel/kernel.elf /dev/zero | dd of=floppy.img bs=512 count=2880 iflag=fullblock
