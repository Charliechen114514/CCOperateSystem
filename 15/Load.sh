dd if=./build/mbr.bin of=a.img count=1 bs=512 seek=0 conv=notrunc
dd if=./build/loader.bin of=a.img count=4 bs=512 seek=2 conv=notrunc
dd if=./build/kernel.bin of=a.img count=200 bs=512 seek=9 conv=notrunc
bochs -f bashsrc.ab
