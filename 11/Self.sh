dd if=mbr.bin of=a.img count=1 bs=512 seek=0 conv=notrunc
dd if=loader.bin of=a.img count=4 bs=512 seek=2 conv=notrunc
bochs -f bashsrc.ab

