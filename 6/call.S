call near proc
jmp $
addr dd 4
proc:
	mov ax, 0x1234
	ret
