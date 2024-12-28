dd if=mbr2.bin of=a.img count=1 bs=512 seek=0 conv=notrunc
dd if=loader2.bin of=a.img count=2 bs=512 seek=2 conv=notrunc
bochs -f bashsrc.ab
