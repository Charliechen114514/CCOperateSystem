EXE_FILE = demo

# -no-pie promissed the available write for n
$(EXE_FILE):
	gcc -no-pie -m32 demo.c -o $(EXE_FILE)

.PHONY: clean
clean:
	rm -f $(EXE_FILE)