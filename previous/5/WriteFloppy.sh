nasm mbr.S -o mbr.bin
dd if=mbr.bin of=a.img count=1 bs=512 conv=notrunc
bochs -f bashsrc
