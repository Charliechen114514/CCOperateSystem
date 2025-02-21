MBR 	= mbr
LOADER	= loader
KERNEL	= kernel
# Same in bochsrc, if u gonna switch the name, do also modified the bochsrc
BOOT_IMG	= boot.img
ASM_INC_DIR = include
ARCH_ASM_DIR= arch/asm
KER_C_DIR	= kernel
OBJ_DIR		= middleware
DISASM_DIR  = disasm


# ASM Issue
KERDIR_ASM_DIR	= arch/kernel
KERMAIN_ASFLAGS = -f elf

# C Issue
KERMAIN_CFLAGS 	= 	-I. -Wall -m32 -fno-stack-protector \
					-c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes

# Linkages
ENTRY_POINT		= 0xc0001500
# Specify the i386 cpu link mode and
# Specify the non-execution stack mode 
# To set up, we need to call at front end, with impl at backend
KERNEL_MAIN_LD_FLAG =  	\
					-m elf_i386 -z noexecstack \
					-Ttext $(ENTRY_POINT) -e main -Map $(OBJ_DIR)/kernel.map


DEBUG_FLAG	:=
# Modify the GCC for the usage
AS 			= nasm
GCC			= gcc
LD			= ld
# Some dependencies macro
KERNEL_MAIN_OBJ = 	${OBJ_DIR}/main.o $(OBJ_DIR)/ccos_print.o \
					${OBJ_DIR}/_ccos_print.o

${BOOT_IMG}: \
	${OBJ_DIR}/${MBR}.bin ${OBJ_DIR}/${LOADER}.bin $(OBJ_DIR)/${KERNEL}.bin

$(OBJ_DIR)/${KERNEL}.bin: $(KERNEL_MAIN_OBJ)
	$(LD) $(KERNEL_MAIN_LD_FLAG) $^ -o $@

# MBR guidance asm
${OBJ_DIR}/${MBR}.bin:
	mkdir -p ${OBJ_DIR}
	nasm -o ${OBJ_DIR}/${MBR}.bin 	-w-orphan-labels	-I${ASM_INC_DIR} ${ARCH_ASM_DIR}/${MBR}.S

# Loader the kernel asm
${OBJ_DIR}/${LOADER}.bin:
	nasm -o ${OBJ_DIR}/${LOADER}.bin -w-orphan-labels	-I${ASM_INC_DIR} ${ARCH_ASM_DIR}/${LOADER}.S

# Main Kernels
${KERNEL_MAIN_OBJ}:
	${AS} ${KERMAIN_ASFLAGS} ${KERDIR_ASM_DIR}/_ccos_print.S -o $(OBJ_DIR)/_ccos_print.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/library/ccos_print.c -o $(OBJ_DIR)/ccos_print.o
	${GCC} -m32 -I. ${DEBUG_FLAG} -c ${KER_C_DIR}/main.c -o ${OBJ_DIR}/main.o

# Phony's are the scrpits usage
.PHONY: clean upload disasm all

# clean the builds
clean:
# Clean the disasm folder if using make disasm
	rm -rf ${DISASM_DIR}/* ${DISASM_DIR}
# Clean the compile time folder	
	rm -rf ${OBJ_DIR}/* ${OBJ_DIR}
# clean the bochs running log and image
	rm -f *.out *.img
	

disasm:
# create the disasm folder
	mkdir -p ${DISASM_DIR}
	ndisasm -o 0x0900 	${OBJ_DIR}/${LOADER}.bin > ${DISASM_DIR}/dis${LOADER}.asm
	ndisasm -o 0x7c00	${OBJ_DIR}/${MBR}.bin > ${DISASM_DIR}/dis${MBR}.asm
	ndisasm -o 0x0000	${OBJ_DIR}/print.o > ${DISASM_DIR}/disprint.asm
	objdump -d --adjust-vma=0xC0001500 ${OBJ_DIR}/main.o  > ${DISASM_DIR}/dismain.asm

# upload the issue
# for bochs 2.8+, you need to specify the creation in -func=create
# on the other hand, below this, use -mode=create
# regards from bochs documenations :)
upload:
	rm -f ${BOOT_IMG}
	bximage -func=create -hd=60M -q ${BOOT_IMG}
	dd if=${OBJ_DIR}/${MBR}.bin of=${BOOT_IMG} bs=512 count=1 conv=notrunc
	dd if=${OBJ_DIR}/${LOADER}.bin of=${BOOT_IMG} bs=512 count=5 seek=2 conv=notrunc
	dd if=${OBJ_DIR}/${KERNEL}.bin of=${BOOT_IMG} bs=512 count=200 seek=9 conv=notrunc
	bochs -f bochsrc

all:
	make clean;make;make upload