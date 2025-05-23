MBR 	= mbr
LOADER	= loader
KERNEL	= kernel
# Same in bochsrc, if u gonna switch the name, do also modified the bochsrc
BOOT_IMG	= boot.img

# disasm and middlewares
OBJ_DIR		= middleware
DISASM_DIR  = disasm

# MBR And Loader

ASM_INC_DIR = include
ARCH_ASM_DIR= arch/asm

# Kernel Mains
# ASM Issue
KERDIR_ASM_DIR	= arch/kernel
KERMAIN_ASFLAGS = -f elf
KER_ASM_INCLUDE		= include

# C Issue
DEBUG_FLAG	:= -DCCOS_DEBUG
KER_C_DIR	= kernel

KERMAIN_CFLAGS 	= 	-I. -Wall -m32 -fno-stack-protector \
					-c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes \
					${DEBUG_FLAG}

# Linkages
ENTRY_POINT		= 0xc0001500
# Specify the i386 cpu link mode and
# Specify the non-execution stack mode 
# To set up, we need to call at front end, with impl at backend
KERNEL_MAIN_LD_FLAG =  	\
					-m elf_i386 -z noexecstack \
					-Ttext $(ENTRY_POINT) -e main -Map $(OBJ_DIR)/kernel.map

# Test Dirent
TEST_DIR	= test
TEST_OBJ_DIR= test_middlewares
TEST_SRC	= test/source
TEST_OUT_KERNEL = n
REQ_CHECK_LIB := -DREQ_CHECK_LIB

OLD_ASSERT_INC = \#include \"include/library/kernel_assert.h
NEW_ASSERT_INC = \#include \"test/include/test.h

# Modify the GCC for the usage
AS 			= nasm
GCC			= gcc
LD			= ld
# Some dependencies macro
KERNEL_MAIN_OBJ = 	${OBJ_DIR}/main.o ${OBJ_DIR}/init.o $(OBJ_DIR)/ccos_print.o \
					${OBJ_DIR}/_ccos_print.o ${OBJ_DIR}/_interrupt_asm.o \
					${OBJ_DIR}/pci.o ${OBJ_DIR}/interrupt.o ${OBJ_DIR}/timer.o \
					${OBJ_DIR}/kernel_assert.o ${OBJ_DIR}/string.o


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
	${AS} ${KERMAIN_ASFLAGS} ${KERDIR_ASM_DIR}/_ccos_print.S -I${KER_ASM_INCLUDE} -o $(OBJ_DIR)/_ccos_print.o
	${AS} ${KERMAIN_ASFLAGS} ${KERDIR_ASM_DIR}/interrupt.S -I${KER_ASM_INCLUDE} -o $(OBJ_DIR)/_interrupt_asm.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/library/ccos_print.c -o $(OBJ_DIR)/ccos_print.o
	${GCC} ${KERMAIN_CFLAGS} -c ${KER_C_DIR}/kernel/interrupt.c -o ${OBJ_DIR}/interrupt.o
	${GCC} ${KERMAIN_CFLAGS} -c ${KER_C_DIR}/kernel/init.c -o ${OBJ_DIR}/init.o
	${GCC} ${KERMAIN_CFLAGS} -c ${KER_C_DIR}/device/pci.c -o ${OBJ_DIR}/pci.o
	${GCC} ${KERMAIN_CFLAGS} -c ${KER_C_DIR}/main.c -o ${OBJ_DIR}/main.o
	${GCC} ${KERMAIN_CFLAGS} -c ${KER_C_DIR}/device/timer.c -o ${OBJ_DIR}/timer.o
	${GCC} ${KERMAIN_CFLAGS} -c ${KER_C_DIR}/library/kernel_assert.c -o ${OBJ_DIR}/kernel_assert.o
	${GCC} ${KERMAIN_CFLAGS} -c ${KER_C_DIR}/library/string.c -o ${OBJ_DIR}/string.o

# Phony's are the scrpits usage
.PHONY: clean upload disasm all rebuild test

# clean the builds
clean:
# Clean the disasm folder if using make disasm
	rm -rf ${DISASM_DIR}
# Clean the compile time folder	
	rm -rf ${OBJ_DIR}
# Clean possible test folder
	rm -rf ${TEST_OBJ_DIR} 
	rm -rf ${TEST_SRC}
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

rebuild:
	make clean;make

test:
	mkdir -p ${TEST_OBJ_DIR}
	mkdir -p ${TEST_SRC}
	cp ${KER_C_DIR}/library/string.c ${TEST_SRC}/string.c
	cp ${KER_C_DIR}/library/bitmap.c ${TEST_SRC}/bitmap.c
	cp ${KER_C_DIR}/library/list.c ${TEST_SRC}/list.c
	perl -pi -e 's/\bKERNEL_ASSERT\b/TEST_ASSERT/g' ${TEST_SRC}/string.c
	perl -pi -e 's|$(OLD_ASSERT_INC)|$(NEW_ASSERT_INC)|g' ${TEST_SRC}/string.c
	perl -pi -e 's/\bKERNEL_ASSERT\b/TEST_ASSERT/g' ${TEST_SRC}/bitmap.c
	perl -pi -e 's|$(OLD_ASSERT_INC)|$(NEW_ASSERT_INC)|g' ${TEST_SRC}/bitmap.c
	perl test/tools/modify.pl ${TEST_SRC}/list.c
	${GCC} -I. -g\
		${TEST_DIR}/test_library_functionalities/test_string.c \
		${TEST_DIR}/test_library_functionalities/test_bitmap.c \
		${TEST_DIR}/test_library_functionalities/test_list.c \
		${TEST_SRC}/string.c ${TEST_SRC}/bitmap.c ${TEST_SRC}/list.c\
		${TEST_DIR}/involk_test.c \
		${TEST_DIR}/test_main.c -o ${TEST_OBJ_DIR}/test_program
	./${TEST_OBJ_DIR}/test_program

all:
	make clean;make;make upload