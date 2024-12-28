;MainGuide Program
;
;LOADER_BASE equ 0x7c00
;LOADER_START_SECTOR equ 0x2
;----------------------------
SECTION MBR vstart=0x7c00
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov sp, 0x7c00
	mov ax, 0xb800
	mov gs, ax
	
;clear the screen
;using 0x06
;----------------------------
	mov ax, 0600h
	mov bx, 0700h
	mov cx, 0
	mov dx, 184fh
	int 10h

;
	mov byte [gs:0x00], 'H'
	mov byte [gs:0x01], 0xA4

	mov byte [gs:0x02], 'e'
	mov byte [gs:0x03], 0xA4
	
	mov byte [gs:0x04], 'l'
	mov byte [gs:0x05], 0xA4

	mov byte [gs:0x06], 'l'
	mov byte [gs:0x07], 0xA4

	mov byte [gs:0x08], 'o'
	mov byte [gs:0x09], 0xA4

	mov byte [gs:0x0A], ' '
	mov byte [gs:0x0B], 0xA4

	mov byte [gs:0x0C], 'W'
	mov byte [gs:0x0D], 0xA4

	mov byte [gs:0x0E], 'o'
	mov byte [gs:0x0F], 0xA4

	mov byte [gs:0x10], 'r'
	mov byte [gs:0x11], 0xA4

	mov byte [gs:0x12], 'l'
	mov byte [gs:0x13], 0xA4

	mov byte [gs:0x14], 'd'
	mov byte [gs:0x15], 0xA4

	jmp $
	times 510 - ($ - $$) db 0
	db 0x55 0xaa






