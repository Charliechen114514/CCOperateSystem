#	----------- Disk Name --------------------
#	Currently, if you modified the image name
# 	U should also modify the bochsrc file
#	For currect loadins
# 	------------------------------------------
BOOT_IMG = boot.img

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
CC = gcc
LD = ld
# 	------------ Tools settings ---------------

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
# -------------- Kernel main Compile Flags -------------



#	------------ LEVEL BASE SRC DIR -------------
# 	Sources for the kernel level program
KERNEL_C_SRC_DIR 	= kernel
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
      	$(OBJ_DIR)/timer.o $(OBJ_DIR)/kernel.o\
		${OBJ_DIR}/ccos_print.o ${OBJ_DIR}/_ccos_print.o \
      	$(OBJ_DIR)/kernel_assert.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/bitmap.o \
      	$(OBJ_DIR)/string.o $(OBJ_DIR)/thread.o $(OBJ_DIR)/list.o \
      	$(OBJ_DIR)/switch_thread.o $(OBJ_DIR)/console_tty.o $(OBJ_DIR)/lock.o \
      	$(OBJ_DIR)/keyboard.o $(OBJ_DIR)/ioqueue.o $(OBJ_DIR)/tss.o \
      	$(OBJ_DIR)/process.o $(OBJ_DIR)/syscall.o $(OBJ_DIR)/syscall-init.o \
      	$(OBJ_DIR)/stdio.o $(OBJ_DIR)/ide.o $(OBJ_DIR)/stdio-kernel.o $(OBJ_DIR)/fs.o \
      	$(OBJ_DIR)/inode.o $(OBJ_DIR)/file.o $(OBJ_DIR)/dir.o $(OBJ_DIR)/fork.o \
      	$(OBJ_DIR)/shell.o $(OBJ_DIR)/user_assert.o  $(OBJ_DIR)/buildin_cmd.o \
      	$(OBJ_DIR)/exec.o $(OBJ_DIR)/wait_exit.o $(OBJ_DIR)/pipe.o

# boot images requires the followings
${BOOT_IMG}: \
	${OBJ_DIR}/${MBR}.bin ${OBJ_DIR}/${LOADER}.bin ${OBJ_DIR}/${KERNEL}.bin

# Provide the os binarys
$(OBJ_DIR)/${KERNEL}.bin: $(KERNEL_MAIN_OBJ)
	$(LD) $(KERNEL_MAIN_LD_FLAG) $^ -o $@

# kernel make policy
${KERNEL_MAIN_OBJ}: 
	${AS} ${KERMAIN_ASFLAGS} ${KERDIR_ASM_DIR}/kernel.S -o $(OBJ_DIR)/kernel.o
	${AS} ${KERMAIN_ASFLAGS} ${KERDIR_ASM_DIR}/_ccos_print.S -o $(OBJ_DIR)/_ccos_print.o
	${AS} ${KERMAIN_ASFLAGS} ${KERDIR_ASM_DIR}/switch_thread.S -o $(OBJ_DIR)/switch_thread.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/main.c -o $(OBJ_DIR)/main.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/global_init/init.c -o $(OBJ_DIR)/init.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/library/interrupt.c -o $(OBJ_DIR)/interrupt.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/library/ccos_print.c -o $(OBJ_DIR)/ccos_print.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/device/timer.c -o $(OBJ_DIR)/timer.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/library/kernel_assert.c -o $(OBJ_DIR)/kernel_assert.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/library/string.c -o $(OBJ_DIR)/string.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/library/bitmap.c -o $(OBJ_DIR)/bitmap.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/memory/memory.c -o $(OBJ_DIR)/memory.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/kernel/thread.c -o $(OBJ_DIR)/thread.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/library/list.c -o $(OBJ_DIR)/list.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/device/console_tty.c -o $(OBJ_DIR)/console_tty.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/kernel/lock.c -o $(OBJ_DIR)/lock.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/device/keyboard.c -o $(OBJ_DIR)/keyboard.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/device/ioqueue.c -o $(OBJ_DIR)/ioqueue.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/tss/tss.c -o $(OBJ_DIR)/tss.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/programs/process.c -o $(OBJ_DIR)/process.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/syscall/syscall.c -o $(OBJ_DIR)/syscall.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/programs/syscall-init.c -o $(OBJ_DIR)/syscall-init.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/FormatIO/stdio.c -o $(OBJ_DIR)/stdio.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/device/ide.c -o $(OBJ_DIR)/ide.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/FormatIO/stdio-kernel.c -o $(OBJ_DIR)/stdio-kernel.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/filesystem/fs.c -o $(OBJ_DIR)/fs.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/filesystem/inode.c -o $(OBJ_DIR)/inode.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/filesystem/file.c -o $(OBJ_DIR)/file.o
	${CC} ${KERMAIN_CFLAGS} ${KERNEL_C_SRC_DIR}/filesystem/dir.c -o $(OBJ_DIR)/dir.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/programs/fork.c -o $(OBJ_DIR)/fork.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/shell/shell.c -o $(OBJ_DIR)/shell.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/library/user_assert.c -o $(OBJ_DIR)/user_assert.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/shell/buildin_cmd.c -o $(OBJ_DIR)/buildin_cmd.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/programs/exec.c -o $(OBJ_DIR)/exec.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/programs/wait_exit.c -o $(OBJ_DIR)/wait_exit.o
	${CC} ${KERMAIN_CFLAGS} ${USER_C_SRC_DIR}/shell/pipe.c -o $(OBJ_DIR)/pipe.o


# MBR guidance asm
${OBJ_DIR}/${MBR}.bin:
	mkdir -p ${OBJ_DIR}
	nasm -o ${OBJ_DIR}/${MBR}.bin ${LOADER_ASFLAGS}	-I${ASM_INC_DIR} ${ARCH_ASM_DIR}/${MBR}.S

# Loader the kernel asm
${OBJ_DIR}/${LOADER}.bin:
	nasm -o ${OBJ_DIR}/${LOADER}.bin ${LOADER_ASFLAGS}	-I${ASM_INC_DIR} ${ARCH_ASM_DIR}/${LOADER}.S

# Phony's are the scrpits usage
.PHONY: help clean upload disasm all rebuild

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
	rm -rf ${DISASM_DIR}/* ${DISASM_DIR}
# Clean the compile time folder	
	rm -rf ${OBJ_DIR}/* ${OBJ_DIR}
# clean the bochs running log and image
	rm -f *.out ${BOOT_IMG}
	

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

rebuild:
	make clean;make

all:
	make clean;make;make upload