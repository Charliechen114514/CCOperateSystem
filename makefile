OBJ_DIR = ./middlewares
BOOT_BIN = ./boot
BOOT_IMG = boot.img
ENTRY_POINT = 0xc0001500
DISASM_DIR = disasm
AS = nasm
CC = gcc
LD = ld
LIB = -I. -I lib/ -I lib/kernel/ -I lib/user/ -I kernel/ -I device/ -I thread/ -I ${USER_C_SRC_DIR}/programs/ -I fs/ -I shell/
CFLAGS = -Wall -m32 -fno-stack-protector $(LIB) -c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes
ASFLAGS = -f elf 
ASM_INC_DIR = include
ARCH_ASM_DIR= arch/asm
MBR 	= mbr
LOADER 	= loader
KERNEL 	= kernel
# Sources for the kernel level program
KERNEL_C_SRC_DIR 	= kernel
USER_C_SRC_DIR		= user

LDFLAGS =  -m elf_i386 -z noexecstack \
			-Ttext $(ENTRY_POINT) -e main -Map $(OBJ_DIR)/kernel.map

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
	$(LD) $(LDFLAGS) $^ -o $@

##############     c代码编译     ###############
$(OBJ_DIR)/main.o: ${KERNEL_C_SRC_DIR}/main.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/init.o: ${KERNEL_C_SRC_DIR}/global_init/init.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/interrupt.o: ${KERNEL_C_SRC_DIR}/library/interrupt.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/ccos_print.o: ${KERNEL_C_SRC_DIR}/library/ccos_print.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/timer.o: ${KERNEL_C_SRC_DIR}/device/timer.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/kernel_assert.o: ${KERNEL_C_SRC_DIR}/library/kernel_assert.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/string.o: ${KERNEL_C_SRC_DIR}/library/string.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/bitmap.o: ${KERNEL_C_SRC_DIR}/library/bitmap.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/memory.o: ${KERNEL_C_SRC_DIR}/memory/memory.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/thread.o: ${KERNEL_C_SRC_DIR}/kernel/thread.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/list.o: ${KERNEL_C_SRC_DIR}/library/list.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/console_tty.o: ${KERNEL_C_SRC_DIR}/device/console_tty.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/lock.o: ${KERNEL_C_SRC_DIR}/kernel/lock.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/keyboard.o: ${KERNEL_C_SRC_DIR}/device/keyboard.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/ioqueue.o: ${KERNEL_C_SRC_DIR}/device/ioqueue.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/tss.o: ${USER_C_SRC_DIR}/tss/tss.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/process.o: ${USER_C_SRC_DIR}/programs/process.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/syscall.o: ${USER_C_SRC_DIR}/syscall/syscall.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/syscall-init.o: ${USER_C_SRC_DIR}/programs/syscall-init.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/stdio.o: ${KERNEL_C_SRC_DIR}/FormatIO/stdio.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/ide.o: ${KERNEL_C_SRC_DIR}/device/ide.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/stdio-kernel.o: ${KERNEL_C_SRC_DIR}/FormatIO/stdio-kernel.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/fs.o: ${KERNEL_C_SRC_DIR}/filesystem/fs.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/inode.o: ${KERNEL_C_SRC_DIR}/filesystem/inode.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/file.o: ${KERNEL_C_SRC_DIR}/filesystem/file.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/dir.o: ${KERNEL_C_SRC_DIR}/filesystem/dir.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/fork.o: ${USER_C_SRC_DIR}/programs/fork.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/shell.o: ${USER_C_SRC_DIR}/shell/shell.c 
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/user_assert.o: ${USER_C_SRC_DIR}/library/user_assert.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/buildin_cmd.o: ${USER_C_SRC_DIR}/shell/buildin_cmd.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/exec.o: ${USER_C_SRC_DIR}/programs/exec.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/wait_exit.o: ${USER_C_SRC_DIR}/programs/wait_exit.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/pipe.o: ${USER_C_SRC_DIR}/shell/pipe.c 
	$(CC) $(CFLAGS) $< -o $@


##############    汇编代码编译    ###############
# MBR guidance asm
${OBJ_DIR}/${MBR}.bin:
	mkdir -p ${OBJ_DIR}
	nasm -o ${OBJ_DIR}/${MBR}.bin -w-orphan-labels		-I${ASM_INC_DIR} ${ARCH_ASM_DIR}/${MBR}.S

# Loader the kernel asm
${OBJ_DIR}/${LOADER}.bin:
	nasm -o ${OBJ_DIR}/${LOADER}.bin -w-orphan-labels	-I${ASM_INC_DIR} ${ARCH_ASM_DIR}/${LOADER}.S


$(OBJ_DIR)/kernel.o: arch/kernel/kernel.S
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/_ccos_print.o: arch/kernel/_ccos_print.S
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/switch_thread.o: arch/kernel/switch_thread.S
	$(AS) $(ASFLAGS) $< -o $@


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

all:
	make clean;make;make upload