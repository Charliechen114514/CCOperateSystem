%include "boot.inc"       ; Include boot section
%include "gpt.inc"        ; Include GPT section
%include "print.inc"      ; Include print section

; -------------------------------------------
;   Author Mentioned:
;       You can place a jmp here, but the base will
;       be offseted to 0x903, so remember this later
; -------------------------------------------
section loader vstart=LOADER_BASE_ADDR   ; Define loader section starting at LOADER_BASE_ADDR
    jmp loader_start                      ; Jump to loader_start

; gpt table here, see the gpt.inc for better 
GPT_TABLE

LOADER_STACK_TOP    equ LOADER_BASE_ADDR   ; Set stack top address for the loader
enter_loader        db "Enter the Loader Sections Success!", 0  ; Success message string

loader_start:
    mov si, enter_loader                 ; Load the address of the message into si
    mov cx, 4                            ; Set the number of characters to print
    call loader_print_strings            ; Call function to print strings
    ; open the A20
    in al, 0x92                          ; Read from port 0x92
    or al, 0000_0010b                    ; Set the A20 bit to enable it
    out 0x92, al                         ; Write the modified value back to port 0x92

    lgdt [GDT_PTR]                       ; Load the GDT pointer
    mov eax, cr0                         ; Read the control register CR0
    or eax, 0x0000_0001                  ; Set the PE bit to enable protected mode
    mov cr0, eax                         ; Write the updated value back to CR0
    jmp dword SELECTOR_CODE:protect_mode_start ; Jump to protected mode start

; ------------------------------------------
; loader_print_strings
; Function Name: loader_print_strings
; Parameters: si - string address
;            cl - line number
; ------------------------------------------
loader_print_strings:
    push di                              ; Save di register
    push ax                              ; Save ax register
    mov ax, 0xA0                         ; Load value 0xA0 into ax
    mov di, 0x00                         ; Set di to 0 (GS segment offset)
    mul cl                                ; Multiply ax by cl
    add di, ax                            ; Add the result to di

loader_store_loop:
    lodsb                                ; Load a byte from [si] into al and update si
    or al, al                            ; Check if the character is 0 (end of string)
    jz loader_done_print                 ; If al is 0, jump to end of print loop

    ; Store character into [gs:di]
    mov [gs:di], al                     ; Store the character in gs:di
    inc di                               ; Increment di to point to the next position

    ; Store 0xA4 into [gs:di]
    mov byte [gs:di], PRINT_STYLE       ; Store 0xA4 in gs:di (set style)
    inc di                               ; Increment di to the next position

    jmp loader_store_loop                ; Continue processing the next character

loader_done_print:
    pop ax                               ; Restore ax register
    pop di                               ; Restore di register
    ret                                  ; Return from function

; now this is 32bits mode 
[bits 32]                                ; Set 32-bit mode for the following code
protect_mode_start:
    mov ax, SELECTOR_DATA                ; Load the data selector into ax
    mov ds, ax                           ; Set ds to the value in ax

    mov es, ax                           ; Set es to the value in ax
    mov ss, ax                           ; Set ss to the value in ax

    mov esp, LOADER_STACK_TOP            ; Set the stack pointer to the loader stack top
    mov ax, SELECTOR_VIDEO               ; Load the video selector into ax
    mov gs, ax                           ; Set gs to the value in ax

    push di                               ; Save di register
    SET_PROTECT_PRINT_LINE 5             ; Set the line to print for the protection mode
    print_hello_os_protect_mode          ; Print the hello message for protected mode
    SET_PROTECT_PRINT_LINE 6             ; Set the next line to print
    print_protect_mode_state             ; Print the protected mode state

    pop di                                ; Restore di register
    Terminate                            ; Terminate the loader process
