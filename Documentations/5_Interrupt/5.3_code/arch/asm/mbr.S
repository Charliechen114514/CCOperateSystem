%include "asm/boot.inc"      ; Includes the boot.inc file which might contain common boot-related functions and macros
%include "asm/print.inc"     ; Includes print.inc file for print functions

section mbr vstart=MBR_VSTART
    ; Load Segments
    mov ax, cs               ; Copy the current code segment (cs) to ax
    mov ds, ax               ; Set data segment (ds) to ax
    mov es, ax               ; Set extra segment (es) to ax
    mov ss, ax               ; Set stack segment (ss) to ax
    mov fs, ax               ; Set fs segment to ax
    mov sp, MBR_VSTART       ; Set stack pointer (sp) to the starting address of MBR_VSTART
    mov ax, 0xb800           ; Set ax to the video memory address (text mode VGA buffer)
    mov gs, ax               ; Set gs segment to ax (video memory segment)

; Clear the screen by using interrupt 0x10 with function 0x06 (scroll up)
    mov ax, 0x0600           ; Function 0x06: Scroll window up (clear screen)
    mov bx, 0x0700           ; Page number (0x07)
    xor cx, cx               ; Set cx to 0 (upper-left corner of screen)
    mov dx, 0x18f4           ; Set the number of rows and columns for the scroll operation
    int 0x10                 ; BIOS video interrupt

; Print the Hello Strings on the screen!
; Set cursor to the first line, first column (DH=0, DL=0)
    mov si, welcome          ; Load the address of "welcome" message into si
    mov cx, 0                ; Line number for the first message
    call mbr_print_strings   ; Print the welcome string

; After this, check the disk setup
    mov si, tell_check_disk  ; Load the address of "tell_check_disk" message
    mov cx, 1                ; Line number for the second message
    call mbr_print_strings   ; Print "Ready to check the disk!"

; Check the disk sectors
    mov eax, LOADER_START_SECTOR  ; Load the loader's start sector into eax
    mov bx, LOADER_BASE_ADDR     ; Load the loader's base address into bx
    mov cx, 4                    ; Number of sectors to read
    call check_disk              ; Call check_disk function to read the sectors

    mov si, tell_check_disk_done ; Load "Disk Check work done" message
    mov cx, 2                    ; Line number for the done message
    call mbr_print_strings       ; Print "Disk check done"

; Jump to the loader's code
    jmp JMP_TO_LOADER            ; Jump to loader code, execution continues there

; ------------------------------------------
; Declare functions below
; ------------------------------------------


; --------------------------
; check_disk
; --------------------------
; This function reads data from a specified disk sector.
; Parameters:
;   eax - The loader start sector address
;   bx  - The buffer to store the read data
;   cx  - The number of sectors to read
; --------------------------
check_disk:
    ; Save the values of the esi and di registers
    push esi
    push di
    
    ; Move the value of eax into esi and the value of cx into di
    mov esi, eax              ; esi will hold the starting address (LBA)
    mov di, cx                ; di will hold the number of sectors to read

    ; Send the number of sectors to the hard disk port (0x1f2)
    mov dx, 0x01f2            ; 0x01f2 is the port for sending the number of sectors to read
    mov al, cl                ; Load the number of sectors (cl) into al
    out dx, al                ; Output the number of sectors to port 0x1f2

    ; Restore eax register
    mov eax, esi              ; Restore the LBA (starting address) to eax

    ; Save the LBA to ports 0x1f3 - 0x1f6
    ; The LBA is a 28-bit value and needs to be written in parts to multiple ports

    mov dx, 0x01f3            ; Set the port to 0x1f3 for the low byte of the LBA
    out dx, al                ; Output the low byte (al) to port 0x1f3

    mov cl, 8                 ; Shift by 8 bits (moving to the next byte of LBA)
    shr eax, cl               ; Shift eax right by 8 bits (eax is the LBA)
    mov dx, 0x01f4            ; Set the port to 0x1f4 for the next byte of the LBA
    out dx, al                ; Output the byte to port 0x1f4

    shr eax, cl               ; Shift eax right by another 8 bits
    mov dx, 0x01f5            ; Set the port to 0x1f5 for the next byte of the LBA
    out dx, al                ; Output the byte to port 0x1f5

    shr eax, cl               ; Shift eax right again
    and al, 0x0f              ; Mask the lower nibble to keep the 4 low bits
    or al, 0xe0               ; Set the upper 4 bits to 0xe (LBA mode)
    mov dx, 0x1f6             ; Set the port to 0x1f6
    out dx, al                ; Output the high byte to port 0x1f6

    ; Send the command to the disk to start reading (0x20 is the read command)
    mov dx, 0x01f7            ; Set the port to 0x1f7 for the disk command
    mov al, 0x20              ; 0x20 is the command for reading data from the disk
    out dx, al                ; Output the command to port 0x1f7

.disk_not_ready:
    nop                       ; Do nothing for 1 clock cycle (to allow the disk to prepare)
    in al, dx                 ; Read the status byte from the disk (port 0x1f7)
    and al, 0x88              ; Mask the status byte to check the DRQ (Data Request) and BSY (Busy) flags
    cmp al, 0x08              ; Check if the disk is ready (BSY = 0 and DRQ = 1)
    jnz .disk_not_ready       ; If the disk is not ready, jump back to check again

    ; Disk is now ready for data transfer

    mov ax, di                ; Load the number of sectors to read into ax
    mov dx, 256               ; Each sector is 256 words (512 bytes)
    mul dx                    ; Multiply ax (number of sectors) by 256 to get the total data size
    mov cx, ax                ; Store the total size in cx
    mov dx, 0x01f0            ; Set the port to 0x01f0 for reading data from the disk

.read_data:
    in ax, dx                 ; Read 2 bytes of data from port 0x01f0 into ax
    mov [bx], ax              ; Store the data at the memory address pointed to by bx
    add bx, 2                 ; Move the memory pointer (bx) by 2 to store the next word
    loop .read_data           ; Loop until all the data has been read (cx decrements with each loop)

    ; Restore the saved registers
    pop di                    ; Restore the value of di register
    pop esi                   ; Restore the value of esi register
    ret                       ; Return from the function


; --------------------------
; mbr_print_strings
; --------------------------
; This function prints a string to the screen.
; Parameters:
;   si - Address of the string to print
;   cl - Line number to print the string
; --------------------------
; --------------------------
; mbr_print_strings
; --------------------------
; This function prints a string to the screen.
; Parameters:
;   si - Address of the string to print
;   cl - Line number to print the string
; --------------------------
mbr_print_strings:
    push di                    ; Save di register
    push ax                    ; Save ax register
    mov ax, NEW_LINE_LENGTH    ; Get the length of a new line
    mov di, 0x00               ; Set gs:di to the starting point of the video memory
    mul cl                     ; Multiply by the line number (cl)
    add di, ax                 ; Add the new line length to di for correct positioning

store_loop:
    lodsb                      ; Load a byte from si to al and increment si
    or al, al                  ; Check if the byte is null (end of string)
    jz done_print              ; If it's null, end the print loop

    ; Print the character at gs:di
    mov [gs:di], al            ; Store the character at the current position in video memory
    inc di                     ; Move to the next position in video memory

    ; Set the style for the character (e.g., color)
    mov byte [gs:di], PRINT_STYLE
    inc di                     ; Move to the next position in video memory for the style

    jmp store_loop             ; Continue to the next character

done_print:
    pop ax                     ; Restore ax
    pop di                     ; Restore di
    ret                        ; Return from the function

; ------------------------------------
; tell the data
; ------------------------------------
welcome                 db "Booting From MBR...", 0
tell_check_disk         db "Ready to check the disk!", 0
tell_check_disk_done    db "Disk Check work done", 0
times 510 - ($ - $$) db 0
PLACE_MBR_MAGIC
