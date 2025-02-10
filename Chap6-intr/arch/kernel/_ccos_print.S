%define TI_GDT          0
%define RPL0            0
%define SELECTOR_VIDEO  (0X0003 << 3) + TI_GDT + RPL0
%define BACKSPACE_ASCII_VAL 0x08
%define CR_ASCII_VAL        0x0d
%define LF_ASCII_VAL        0x0a

; console style 
%define CONSOLE_STYLE       0x07

; VGA Screen's property
%define CURSOR_END          2000
%define CARRY_CHAR_NUM      ((2000 - 80) * 2)
%define CARRY_CHAR_NUM_TIME (CARRY_CHAR_NUM / 4)
[bits 32]
section .data
__ccos_interger_display_buffer dq 0
section .text

; -----------------------------------
;   put the char into the cosole
; -----------------------------------
global __ccos_putchar
__ccos_putchar:
    pushad  ; push the 32 bits registers value
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    in  al, dx
    mov ah, al

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in  al, dx

    mov bx, ax
    mov ecx, [esp + 36]
    
    cmp cl, CR_ASCII_VAL
    jz  .is_carriage_return
    cmp cl, LF_ASCII_VAL
    jz  .is_line_feed
    cmp cl, BACKSPACE_ASCII_VAL
    jz  .is_backspace
    jmp .put_other

.is_backspace:
    dec bx
    shl bx, 1
    mov byte[gs:bx], 0x20
    inc bx
    mov byte[gs:bx], CONSOLE_STYLE
    shr bx, 1
    jmp .set_cursor

.put_other:
    shl bx, 1
    mov [gs:bx], cl
    inc bx
    mov byte[gs:bx], CONSOLE_STYLE
    shr bx, 1
    inc bx
    cmp bx, CURSOR_END
    jl .set_cursor

.is_line_feed:
.is_carriage_return:
    xor dx, dx
    mov ax, bx
    mov si, 80
    div si
    sub bx, dx
.carriage_return_end:
    add bx, 80
    cmp bx, CURSOR_END
.line_feed_end:
    jl .set_cursor

.roll_screen:
    cld
    mov ecx, CARRY_CHAR_NUM_TIME
    mov esi, 0xc00b80a0
    mov edi, 0xc00b8000
    rep movsd

    mov ebx, CARRY_CHAR_NUM
    mov ecx, 80
.cls:
    mov word [gs: ebx], 0x0720
    add ebx, 2
    loop .cls
    mov bx, CARRY_CHAR_NUM / 2
.set_cursor:
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al

.end_ccos_putchar:
    popad
    ret

global ccos_puts
ccos_puts:
    push ebx
    push ecx
    xor ecx, ecx
    mov ebx, [esp + 12]
.continue_fetch:
    mov cl, [ebx]
    cmp cl, 0
    jz .ccos_puts_done

    push ecx
    call __ccos_putchar
    add esp, 4
    inc ebx
    jmp .continue_fetch
.ccos_puts_done:
    pop ecx
    pop ebx
    ret

global __ccos_display_int
__ccos_display_int:
    pushad
    mov ebp, esp
    mov eax, [ebp + 36]
    mov edx, eax
    mov edi, 7
    mov ecx, 8
    mov ebx, __ccos_interger_display_buffer
.16based_4bits:
    and edx, 0x0000_000F
    cmp edx, 9
    jg .display_A2F
    add edx, '0'
    jmp .store
.display_A2F:
    sub edx, 10
    add edx, 'A'
.store:
    mov [ebx + edi], dl
    dec edi
    shr eax, 4
    mov edx, eax
    loop .16based_4bits

.ready_to_trans:
    inc edi
.skip_prefix_0:
    cmp edi, 8
    je .full0
.skip:
    mov cl, [__ccos_interger_display_buffer + edi]
    inc edi
    cmp cl, '0'
    je .skip_prefix_0
    dec edi
    jmp .print_each_num
.full0:
    mov cl, '0'
.print_each_num:
    push ecx 
    call __ccos_putchar
    add esp, 4
    inc edi 
    mov cl, [__ccos_interger_display_buffer + edi]
    cmp edi, 8
    jl .print_each_num  
    popad
    ret

; Export the console_setter
global __ccos_set_console_cursor
__ccos_set_console_cursor:
    pushad
    ; the bh is row
    mov ecx, [esp + 36]
    mov edx, [esp + 40]

    ; check the col's
    mov ebx, ecx
    shl ebx, 8
    add bl, dl
    xor ecx, ecx
    xor edx, edx

    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al

    popad
    ret