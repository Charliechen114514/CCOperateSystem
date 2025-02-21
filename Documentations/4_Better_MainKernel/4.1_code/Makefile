EXE_FILE := union_call

$(EXE_FILE):
	nasm -f elf32 demo_asm.S -o demo_asm.o
	gcc -m32 -c demo_c.c -o demo_c.o
	ld -m elf_i386 -z noexecstack -s -o $(EXE_FILE) demo_c.o demo_asm.o

.PHONY:clean

clean:
	rm -f *.o $(EXE_FILE)