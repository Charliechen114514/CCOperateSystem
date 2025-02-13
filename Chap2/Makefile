MBR-OBJ 	= mbr.bin
# Same in bochsrc, if u gonna switch the name, do also modified the bochsrc
BOOT_IMG	= boot.img
MBR-OBJ:
	nasm -o ${MBR-OBJ} mbr.S

.PHONY: clean upload all

all:
	make clean;make;make upload

clean:
	rm -f *.bin *.out *.img

upload:
	rm -f ${BOOT_IMG}
	bximage -func=create -hd=60M -q ${BOOT_IMG}
	dd if=mbr.bin of=${BOOT_IMG} bs=512 count=1 conv=notrunc
	bochs -f bochsrc
