nasm bitsDif.S -o bitsDif.bin
dd if=bitsDif.bin of=floppy.img count=1 bs=512 conv=notrunc
bochs -f bashsrc.ab
