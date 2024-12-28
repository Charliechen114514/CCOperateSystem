MBR 	= mbr
LOADER	= loader
UTILS   = utils
KERNEL	= kernel
# Same in bochsrc, if u gonna switch the name, do also modified the bochsrc
BOOT_IMG	= boot.img
ASM_INC_DIR = include/asm
KER_C_DIR	= kernel
OBJ_DIR		= middleware

GCC			= gcc4

OBJS:
	nasm -o ${MBR}.bin 		-I${ASM_INC_DIR} ${MBR}.S
	nasm -o ${LOADER}.bin 	-I${ASM_INC_DIR} ${LOADER}.S
	${GCC} -m32 -c ${KER_C_DIR}/main.c -o ${OBJ_DIR}/main.o
	ld -m elf_i386 ${OBJ_DIR}/main.o -Ttext 0xc0001500 -e main -o ${KERNEL}.bin


.PHONY: clean upload disasm

# clean the builds
clean:
	rm -rf middleware/*
	rm *.bin *.out *.img *.asm
	

disasm:
	ndisasm -o 0x900 	${LOADER}.bin > dis${LOADER}.asm
	ndisasm -o 0x7c00	${MBR}.bin > dis${MBR}.asm

upload:
	bximage -func=create -hd=60M -q ${BOOT_IMG}
	dd if=${MBR}.bin of=${BOOT_IMG} bs=512 count=1 conv=notrunc
	dd if=${LOADER}.bin of=${BOOT_IMG} bs=512 count=5 seek=2 conv=notrunc
	dd if=${KERNEL}.bin of=${BOOT_IMG} bs=512 count=200 seek=9 conv=notrunc
	bochs -f bochsrc
