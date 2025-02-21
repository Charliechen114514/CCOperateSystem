DEMO = demo

$(DEMO).s:
	gcc -S -masm=intel demo.c -o demo_intel.s
	gcc -S -masm=att demo.c -o demo_att.s

.PHONY: clean

clean:
	rm -f *.s