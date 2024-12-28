nasm mbr.S -o mbr.bin
dd if=mymbr.bin of=floppy.img count=1 bs=512 conv=notrunc
bochs -f bashsrc.ab
