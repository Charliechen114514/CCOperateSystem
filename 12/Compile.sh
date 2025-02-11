nasm -I boot/include/ boot/loader.S -o boot/loader.bin
gcc -m32 -c -o kernel/main.o kernel/main.c
ld -m elf_i386 kernel/main.o -Ttext 0xc0001500 -e main -o kernel/kernel.bin
