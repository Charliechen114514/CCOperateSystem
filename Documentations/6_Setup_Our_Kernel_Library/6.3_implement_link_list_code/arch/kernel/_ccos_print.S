%define TI_GDT              0
%define RPL0                0
%define SELECTOR_VIDEO      (0X0003 << 3) + TI_GDT + RPL0  ; Video memory segment selector
%define BACKSPACE_ASCII_VAL 0x08   ; ASCII value for Backspace
%define CR_ASCII_VAL        0x0d   ; ASCII value for Carriage Return (CR)
%define LF_ASCII_VAL        0x0a   ; ASCII value for Line Feed (LF)

; console style (light gray text on black background)
%define CONSOLE_STYLE       0x07

; VGA Screen's properties
%define CURSOR_END          2000  ; End position of the cursor (2000 = 80*25, for 80x25 screen)
%define CARRY_CHAR_NUM      ((2000 - 80) * 2)  ; Number of characters that will be scrolled when screen is full
%define CARRY_CHAR_NUM_TIME (CARRY_CHAR_NUM / 4)  ; Time to scroll the screen

; Macro to set the cursor position
%macro  SET_CURSOR_CONV  0
    mov dx, 0x03d4      ; VGA control port for cursor high byte (0x03d4)
    mov al, 0x0e        ; Select cursor high byte register
    out dx, al
    mov dx, 0x03d5      ; VGA control port for cursor high byte value (0x03d5)
    mov al, bh          ; Store row position (in bh register)
    out dx, al

    mov dx, 0x03d4      ; VGA control port for cursor low byte (0x03d4)
    mov al, 0x0f        ; Select cursor low byte register
    out dx, al
    mov dx, 0x03d5      ; VGA control port for cursor low byte value (0x03d5)
    mov al, bl          ; Store column position (in bl register)
    out dx, al
%endmacro

[bits 32]
section .data
global __ccos_internal_style_sheet
__ccos_interger_display_buffer  dq 0    ; Buffer for integer display
__ccos_internal_style_sheet     db CONSOLE_STYLE  ; Console style setting (text color)

section .text

; -----------------------------------
;   put the char into the console
; -----------------------------------
global __ccos_putchar
__ccos_putchar:
    pushad  ; Save the state of all registers (32-bit) to preserve them
    mov ax, SELECTOR_VIDEO
    mov gs, ax   ; Set gs register to point to video memory segment

    ; Read current cursor position (row and column)
    mov dx, 0x03d4      ; VGA control port for cursor high byte (0x03d4)
    mov al, 0x0e        ; Select cursor high byte register
    out dx, al
    mov dx, 0x03d5      ; VGA control port for cursor high byte value (0x03d5)
    in  al, dx
    mov ah, al          ; Store cursor row position in ah register

    mov dx, 0x03d4      ; VGA control port for cursor low byte (0x03d4)
    mov al, 0x0f        ; Select cursor low byte register
    out dx, al
    mov dx, 0x03d5      ; VGA control port for cursor low byte value (0x03d5)
    in  al, dx          ; Read current column position into al

    mov bx, ax          ; Combine row and column position into bx
    mov ecx, [esp + 36] ; Get the character to print from the function arguments
    
    ; Check if the character is a Carriage Return, Line Feed, or Backspace
    cmp cl, CR_ASCII_VAL
    jz  .is_carriage_return  ; If Carriage Return, jump to the Carriage Return handler
    cmp cl, LF_ASCII_VAL
    jz  .is_line_feed        ; If Line Feed, jump to the Line Feed handler
    cmp cl, BACKSPACE_ASCII_VAL
    jz  .is_backspace        ; If Backspace, jump to the Backspace handler
    jmp .put_other           ; Otherwise, handle the character normally

.is_backspace:
    dec bx                  ; Move cursor one position back (for backspace)
    shl bx, 1               ; Shift left to adjust for VGA memory addressing
    mov byte[gs:bx], 0x20   ; Write space character to clear the position
    inc bx                  ; Move cursor forward by one byte
    push ax
    mov byte al, [__ccos_internal_style_sheet]  ; Get the current text style
    mov byte [gs:bx], al    ; Set the style for the current position
    pop ax
    shr bx, 1               ; Shift back to adjust the position for a 16-bit word
    jmp .set_cursor          ; Set the cursor to the new position

.put_other:
    shl bx, 1               ; Shift left to adjust for VGA memory addressing
    mov [gs:bx], cl         ; Write the character at the current cursor position
    inc bx                  ; Move cursor forward by one byte
    push ax
    mov byte al, [__ccos_internal_style_sheet]  ; Get the current text style
    mov byte [gs:bx], al    ; Set the style for the current position
    pop ax
    shr bx, 1               ; Shift back to adjust the position for a 16-bit word
    inc bx                  ; Move cursor forward
    cmp bx, CURSOR_END      ; Check if the cursor has reached the end of the screen
    jl .set_cursor          ; If not, set the cursor to the new position

.is_line_feed:
.is_carriage_return:
    xor dx, dx              ; Clear dx register
    mov ax, bx              ; Copy the cursor position (row/column) to ax
    mov si, 80              ; Set divisor for screen width (80 columns)
    div si                  ; Divide to get the new position (row/column)
    sub bx, dx              ; Subtract the remainder from the position
.carriage_return_end:
    add bx, 80              ; Move to the next line (add 80 to column position)
    cmp bx, CURSOR_END      ; Check if the cursor has reached the end of the screen
.line_feed_end:
    jl .set_cursor          ; If not, set the cursor to the new position

.roll_screen:
    cld                     ; Clear the direction flag (for string operations)
    mov ecx, CARRY_CHAR_NUM_TIME  ; Number of times to scroll the screen
    mov esi, 0xc00b80a0     ; Source address for scrolling (start of video memory)
    mov edi, 0xc00b8000     ; Destination address for scrolling (start of video memory)
    rep movsd               ; Scroll the screen by moving words (4 bytes) from source to destination

    mov ebx, CARRY_CHAR_NUM
    mov ecx, 80
.cls:
    mov word [gs: ebx], 0x0720   ; Clear the current position (write space and style)
    add ebx, 2                   ; Move to the next word (2 bytes)
    loop .cls                    ; Repeat for the entire screen (80x25)

    mov bx, CARRY_CHAR_NUM / 2   ; Set the cursor position back to the start after scrolling
.set_cursor:
    SET_CURSOR_CONV              ; Call the macro to set the cursor to the new position

.end_ccos_putchar:
    popad  ; Restore the saved register values
    ret    ; Return from the function


global ccos_puts
ccos_puts:
    push ebx                  ; Save the value of ebx register
    push ecx                  ; Save the value of ecx register
    xor ecx, ecx              ; Clear ecx (used for character processing)
    mov ebx, [esp + 12]       ; Get the pointer to the string (first argument passed)

.continue_fetch:
    mov cl, [ebx]             ; Load the next character from the string into cl
    cmp cl, 0                 ; Check if it's the null terminator (end of string)
    jz .ccos_puts_done        ; If it is, jump to end of function

    push ecx                  ; Save the current character
    call __ccos_putchar       ; Call the function to output the character
    add esp, 4                ; Clean up the stack (remove pushed character)
    inc ebx                   ; Move to the next character in the string
    jmp .continue_fetch       ; Repeat the process for the next character

.ccos_puts_done:
    pop ecx                   ; Restore the saved ecx register
    pop ebx                   ; Restore the saved ebx register
    ret                       ; Return from the function

global __ccos_display_int
__ccos_display_int:
    pushad                    ; Save all 32-bit registers (eax, ebx, ecx, edx, etc.)
    mov ebp, esp              ; Set up the stack frame (base pointer for local variables)
    mov eax, [ebp + 36]       ; Load the integer to be displayed (passed as argument)
    mov edx, eax              ; Copy the value to edx (to use for division)
    mov edi, 7                ; Set edi to 7 (index for placing digits into the buffer)
    mov ecx, 8                ; Set ecx to 8 (loop counter for 8 hexadecimal digits)
    mov ebx, __ccos_interger_display_buffer  ; Pointer to the buffer to store the formatted digits

.16based_4bits:
    and edx, 0x0000_000F      ; Mask the lower 4 bits of edx (get the current hex digit)
    cmp edx, 9                ; Check if the digit is less than or equal to 9
    jg .display_A2F           ; If the digit is greater than 9, jump to A-F conversion
    add edx, '0'              ; If the digit is 0-9, convert it to its ASCII equivalent
    jmp .store

.display_A2F:
    sub edx, 10               ; If the digit is 10 or higher, subtract 10
    add edx, 'A'              ; Convert to ASCII 'A' through 'F'

.store:
    mov [ebx + edi], dl       ; Store the ASCII character in the display buffer
    dec edi                   ; Decrease the index (move backwards in the buffer)
    shr eax, 4                ; Shift the value in eax right by 4 bits (move to the next hex digit)
    mov edx, eax              ; Copy the shifted value to edx for the next iteration
    loop .16based_4bits       ; Repeat the loop for all 8 hex digits

.ready_to_trans:
    inc edi                   ; Increment edi to skip the leading zeros
.skip_prefix_0:
    cmp edi, 8                ; Check if we have processed all 8 characters
    je .full0                 ; If we have, jump to full0 label (to print zero if necessary)
.skip:
    mov cl, [__ccos_interger_display_buffer + edi] ; Load the character from the buffer
    inc edi                   ; Move to the next character in the buffer
    cmp cl, '0'               ; Check if the character is a leading zero
    je .skip_prefix_0         ; If it is, skip it and continue
    dec edi                   ; Otherwise, step back to print the last non-zero character
    jmp .print_each_num       ; Jump to print the current non-zero character

.full0:
    mov cl, '0'               ; If all characters were zero, load '0' to print
.print_each_num:
    push ecx                  ; Push the character to the stack for printing
    call __ccos_putchar       ; Call the function to print the character
    add esp, 4                ; Clean up the stack
    inc edi                   ; Move to the next character
    mov cl, [__ccos_interger_display_buffer + edi] ; Load the next character
    cmp edi, 8                ; Check if we have processed all 8 digits
    jl .print_each_num        ; If not, continue printing

    popad                     ; Restore all registers that were saved by pushad
    ret                       ; Return from the function

; Export the console_setter
global __ccos_set_console_cursor
__ccos_set_console_cursor:
    pushad                    ; Save all registers (eax, ebx, ecx, edx, etc.)
    ; The bh register contains the row position
    mov ecx, [esp + 36]       ; Load the row position (first argument)
    mov edx, [esp + 40]       ; Load the column position (second argument)

    ; Combine row and column into the correct position
    mov ebx, ecx              ; Copy row to ebx
    shl ebx, 8                ; Shift left by 8 bits to prepare for column addition
    add bl, dl                ; Add the column value to the lower byte of ebx
    xor ecx, ecx              ; Clear ecx (unused)
    xor edx, edx              ; Clear edx (unused)
    SET_CURSOR_CONV           ; Call macro to set the cursor position

    popad                     ; Restore all registers that were saved by pushad
    ret                       ; Return from the function

global set_console_color
set_console_color:
    push eax                  ; Save eax register
    mov eax, [esp + 36]       ; Load the color byte from the argument
    and al, 0x0F              ; Mask the lower 4 bits (only keep the color)
    mov byte [__ccos_internal_style_sheet], al ; Store the color in the style sheet
    pop eax                   ; Restore eax register
    ret                       ; Return from the function

; Screen cleaner function to clear the screen
global clean_screen
clean_screen:
    pushad                    ; Save all registers (eax, ebx, ecx, edx, etc.)
    mov ax, SELECTOR_VIDEO    ; Set ax to the video memory selector
    mov gs, ax                ; Set gs register to point to video memory segment

    mov ebx, 0                ; Start at the beginning of the video memory
    mov ecx, 80 * 25          ; Total number of lines (80 columns * 25 rows)

.clean_each_cls:
    mov word [gs:ebx], 0x0720  ; Write space character (0x20) with the current style (0x07)
    add ebx, 2                 ; Move to the next 2-byte word in video memory
    loop .clean_each_cls       ; Repeat for all positions on the screen

    mov ebx, 0                ; Reset the cursor position to the top left of the screen
    SET_CURSOR_CONV           ; Set the cursor back to the starting position

    popad                     ; Restore all registers that were saved by pushad
    ret                       ; Return from the function
