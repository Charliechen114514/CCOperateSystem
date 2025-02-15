MBR 	= mbr
LOADER	= loader
UTILS   = utils
# Same in bochsrc, if u gonna switch the name, do also modified the bochsrc
BOOT_IMG	= boot.img
MBR-OBJ:
	nasm -o ${MBR}.bin 		${MBR}.S
	nasm -o ${LOADER}.bin 	${LOADER}.S

.PHONY: clean upload all

clean:
	rm -rf *.bin *.out *.img

upload:
	rm -rf ${BOOT_IMG}
	bximage -func=create -hd=60M -q ${BOOT_IMG}
	dd if=${MBR}.bin of=${BOOT_IMG} bs=512 count=1 conv=notrunc
	dd if=${LOADER}.bin of=${BOOT_IMG} bs=512 count=1 seek=2 conv=notrunc
	bochs -f bochsrc
all:
	make clean;make;make upload