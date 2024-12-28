nasm -f elf ./lib/kernel/myprint.S -o ./lib/kernel/print.o
gcc -m32 -I ./lib/kernel/ -c -o ./kernel/main.o ./kernel/main.c
ld -m elf_i386 ./kernel/main.o ./lib/kernel/print.o  -Ttext 0xc0001500 -e main -o ./kernel/kernel.bin

