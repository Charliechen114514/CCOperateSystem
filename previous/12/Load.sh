dd if=./boot/mbr.bin of=a.img count=1 bs=512 seek=0 conv=notrunc
dd if=./boot/loader.bin of=a.img count=4 bs=512 seek=2 conv=notrunc
dd if=./kernel/kernel.bin of=a.img count=200 bs=512 seek=9 conv=notrunc
bochs -f bashsrc.ab
