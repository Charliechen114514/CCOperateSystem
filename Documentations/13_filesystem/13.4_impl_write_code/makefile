#	----------- Disk Name --------------------
#	Currently, if you modified the image name
# 	U should also modify the bochsrc file
#	For currect loadins
# 	------------------------------------------
BOOT_IMG 	= boot.img
DISK_NAME	= disk.img
# 	------------ Binary file name ---------------
MBR 	= mbr
LOADER 	= loader
KERNEL 	= kernel
# 	------------ Binary file name ---------------

#	------------ Binary Build dir ---------------
OBJ_DIR = ./middlewares
DISASM_DIR = ./disasm
#	------------ Binary Build dir ---------------

# 	------------ Tools settings ---------------
AS = nasm
GCC = gcc
LD = ld
# 	------------ Tools settings ---------------

# -------------- Test issue --------------
# Test Dirent
TEST_DIR	= test
TEST_OBJ_DIR= test_middlewares
TEST_SRC	= test/source
TEST_OUT_KERNEL = n
REQ_CHECK_LIB := -DREQ_CHECK_LIB

OLD_ASSERT_INC = \#include \"include/library/kernel_assert.h
NEW_ASSERT_INC = \#include \"test/include/test.h
# ----------------------------------------------


# -------------------------------------------
#	Compile flags
# -------------------------------------------
# -------------- Loader main Compile Flags -------------
LOADER_ASFLAGS 	= -w-orphan-labels
# -------------- Loader main Compile Flags -------------
# -------------- Kernel main Compile Flags -------------
KERMAIN_CFLAGS 	= 	-I. -Wall -m32 -fno-stack-protector \
					-c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes
KERMAIN_ASFLAGS = -f elf 
KER_ASM_INCLUDE		= include
# -------------- Kernel main Compile Flags -------------



#	------------ LEVEL BASE SRC DIR -------------
# 	Sources for the kernel level program
KER_C_DIR 	= kernel
USER_C_SRC_DIR		= user
# ASM Compiles includes
ASM_INC_DIR 		= include
# ASM Src Dirent
ARCH_ASM_DIR		= arch/asm
# ASM Main Dirent
KERDIR_ASM_DIR		= arch/kernel
#	------------ LEVEL BASE SRC DIR -------------

#	------------ LEVEL BASE SRC DIR -------------
# 		Kernel OS Compile sections
ENTRY_POINT = 0xc0001500
KERNEL_MAIN_LD_FLAG =  -m elf_i386 -z noexecstack \
			-Ttext $(ENTRY_POINT) -e main -Map $(OBJ_DIR)/kernel.map
#	------------ LEVEL BASE SRC DIR -------------


KERNEL_MAIN_OBJ = \
		$(OBJ_DIR)/main.o $(OBJ_DIR)/init.o $(OBJ_DIR)/interrupt.o \
      	$(OBJ_DIR)/timer.o $(OBJ_DIR)/_interrupt_asm.o $(OBJ_DIR)/pci.o\
		${OBJ_DIR}/ccos_print.o ${OBJ_DIR}/_ccos_print.o \
      	$(OBJ_DIR)/kernel_assert.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/bitmap.o \
      	$(OBJ_DIR)/string.o $(OBJ_DIR)/thread.o $(OBJ_DIR)/list.o \
      	$(OBJ_DIR)/switch_thread.o $(OBJ_DIR)/console_tty.o $(OBJ_DIR)/lock.o \
      	$(OBJ_DIR)/keyboard.o $(OBJ_DIR)/ioqueue.o $(OBJ_DIR)/tss.o \
      	$(OBJ_DIR)/process.o $(OBJ_DIR)/syscall.o $(OBJ_DIR)/syscall-init.o \
      	$(OBJ_DIR)/stdio.o $(OBJ_DIR)/ide.o $(OBJ_DIR)/stdio-kernel.o \
		$(OBJ_DIR)/inode.o $(OBJ_DIR)/file.o $(OBJ_DIR)/dir.o $(OBJ_DIR)/filesystem.o\

		
# boot images requires the followings
${BOOT_IMG}: \
	${OBJ_DIR}/${MBR}.bin ${OBJ_DIR}/${LOADER}.bin ${OBJ_DIR}/${KERNEL}.bin

# Provide the os binarys
$(OBJ_DIR)/${KERNEL}.bin: $(KERNEL_MAIN_OBJ)
	$(LD) $(KERNEL_MAIN_LD_FLAG) $^ -o $@

# kernel make policy
${KERNEL_MAIN_OBJ}: 
	${AS} ${KERMAIN_ASFLAGS} ${KERDIR_ASM_DIR}/interrupt.S -I${KER_ASM_INCLUDE} -o $(OBJ_DIR)/_interrupt_asm.o
	${AS} ${KERMAIN_ASFLAGS} ${KERDIR_ASM_DIR}/_ccos_print.S -o $(OBJ_DIR)/_ccos_print.o
	${AS} ${KERMAIN_ASFLAGS} ${KERDIR_ASM_DIR}/switch_thread.S -o $(OBJ_DIR)/switch_thread.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/main.c -o $(OBJ_DIR)/main.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/global_init/init.c -o $(OBJ_DIR)/init.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/device/pci.c -o ${OBJ_DIR}/pci.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/library/interrupt.c -o $(OBJ_DIR)/interrupt.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/library/ccos_print.c -o $(OBJ_DIR)/ccos_print.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/device/timer.c -o $(OBJ_DIR)/timer.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/library/kernel_assert.c -o $(OBJ_DIR)/kernel_assert.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/library/string.c -o $(OBJ_DIR)/string.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/library/bitmap.c -o $(OBJ_DIR)/bitmap.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/memory/memory.c -o $(OBJ_DIR)/memory.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/thread/thread.c -o $(OBJ_DIR)/thread.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/library/list.c -o $(OBJ_DIR)/list.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/device/console_tty.c -o $(OBJ_DIR)/console_tty.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/kernel/lock.c -o $(OBJ_DIR)/lock.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/device/keyboard.c -o $(OBJ_DIR)/keyboard.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/io/ioqueue.c -o $(OBJ_DIR)/ioqueue.o
	${GCC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/tss/tss.c -o $(OBJ_DIR)/tss.o
	${GCC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/programs/process.c -o $(OBJ_DIR)/process.o
	${GCC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/syscall/syscall.c -o $(OBJ_DIR)/syscall.o
	${GCC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/programs/syscall-init.c -o $(OBJ_DIR)/syscall-init.o
	${GCC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/stdio/stdio.c -o $(OBJ_DIR)/stdio.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/device/ide.c -o $(OBJ_DIR)/ide.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/io/stdio-kernel.c -o $(OBJ_DIR)/stdio-kernel.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/filesystem/filesystem.c -o $(OBJ_DIR)/filesystem.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/filesystem/inode.c -o $(OBJ_DIR)/inode.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/filesystem/file.c -o $(OBJ_DIR)/file.o
	${GCC} ${KERMAIN_CFLAGS} ${KER_C_DIR}/filesystem/dir.c -o $(OBJ_DIR)/dir.o


# MBR guidance asm
${OBJ_DIR}/${MBR}.bin:
	mkdir -p ${OBJ_DIR}
	nasm -o ${OBJ_DIR}/${MBR}.bin ${LOADER_ASFLAGS}	-I${ASM_INC_DIR} ${ARCH_ASM_DIR}/${MBR}.S

# Loader the kernel asm
${OBJ_DIR}/${LOADER}.bin:
	nasm -o ${OBJ_DIR}/${LOADER}.bin ${LOADER_ASFLAGS}	-I${ASM_INC_DIR} ${ARCH_ASM_DIR}/${LOADER}.S

# Phony's are the scrpits usage
.PHONY: help clean upload disasm all rebuild test

help:
	@echo "------------ CCOperatingSystem Makefile Usage -------------------"
	@echo "Available targets:"
	@echo "  clean     - Clean build artifacts, object files, and logs."
	@echo "  disasm    - Generate disassembly of loader, MBR, and kernel."
	@echo "  upload    - Create and upload boot image using Bochs."
	@echo "  rebuild   - Clean and build the project."
	@echo "  all       - Clean, build, and run using Bochs."
	@echo "notice: run make all to setup and run os directly, then type "
	@echo "c and enter it into bochs terminal, the os will run!"
	@echo "------------ CCOperatingSystem Makefile Usage -------------------"

# clean the builds
clean:
# Clean the disasm folder if using make disasm
	rm -rf  ${DISASM_DIR}
# Clean the compile time folder	
	rm -rf ${OBJ_DIR}
# Clean possible test folder
	rm -rf ${TEST_OBJ_DIR} 
	rm -rf ${TEST_SRC}
# clean the bochs running log and image
	rm -f *.out ${BOOT_IMG} ${BOOT_IMG}.lock
	

# Add files you gonna dump speical!
disasm:
# create the disasm folder
	mkdir -p ${DISASM_DIR}
	ndisasm -o 0x0900 	${OBJ_DIR}/${LOADER}.bin > ${DISASM_DIR}/dis${LOADER}.asm
	ndisasm -o 0x7c00	${OBJ_DIR}/${MBR}.bin > ${DISASM_DIR}/dis${MBR}.asm
	ndisasm -o 0x0000	${OBJ_DIR}/_ccos_print.o > ${DISASM_DIR}/disprint.asm
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

rebuild:
	make clean;make

create_disk:
	bximage -func=create -hd=80M -q ${DISK_NAME}

rm_disk:
	rm -f ${DISK_NAME}

all:
	make clean;make;make upload