MBR 	= mbr
LOADER	= loader
UTILS   = utils
# Same in bochsrc, if u gonna switch the name, do also modified the bochsrc
BOOT_IMG	= boot.img
ASM_INC_DIR = include/asm

MBR-OBJ:
	nasm -o ${MBR}.bin 		-I${ASM_INC_DIR} ${MBR}.S
	nasm -o ${LOADER}.bin 	-I${ASM_INC_DIR} ${LOADER}.S

.PHONY: clean upload

# clean the builds
clean:
	rm *.bin *.out *.img

upload:
	bximage -func=create -hd=60M -q ${BOOT_IMG}
	dd if=${MBR}.bin of=${BOOT_IMG} bs=512 count=1 conv=notrunc
	dd if=${LOADER}.bin of=${BOOT_IMG} bs=512 count=4 seek=2 conv=notrunc
	bochs -f bochsrc
