mov ax, $$
mov ds, ax
mov ax, [var]
label: 
	mov ax, $
jmp label
var dw 0x99 
