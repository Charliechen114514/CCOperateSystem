%include "asm/boot.inc"             ; boot settings
%include "asm/gpt.inc"              ; gpt settings
%include "asm/detect_memory.inc"    ; memory detector settings
%include "asm/print.inc"            ; including for the print in preserved


; -------------------------------------------
;   Author Mentioned:
;       You can place a jmp here, but the base will
;       be offseted to 0x903, so remember this later
; -------------------------------------------
section loader vstart=LOADER_BASE_ADDR   ; Define loader section starting at LOADER_BASE_ADDR

; gpt table here, see the gpt.inc for better 
GPT_TABLE
; kernel info here
KERNEL_INFO

LOADER_STACK_TOP    equ LOADER_BASE_ADDR   ; Set stack top address for the loader

loader_start:

    xor ebx, ebx		      ; ebx should be 0
    mov edx, E820Magic	      
    mov di, ards_buf	      ;  see gpt sections for details
 .fetch_each_memory_info:	      ;
    mov eax, E820SubFunc	   ;
    mov ecx, 20		         ;  20 bytes each
    int 0x15
    ;  if 1, failed, so we check e801
    jc .try_e801   
    add di, cx              ; Increase di by 20 bytes to point to the new ARDS structure position in the buffer
    inc word [ards_nr]       ; Increment the ARDS count
    cmp ebx, 0               ; If ebx is 0 and cf is not 1, it means all ARDS have been returned, and we are at the last one
    jnz .fetch_each_memory_info
 
 ; Find the maximum value of (base_add_low + length_low) among all ARDS structures, which represents the largest memory capacity.
    mov cx, [ards_nr]        ; Loop through each ARDS structure, the loop runs for the number of ARDS structures
    mov ebx, ards_buf 
    xor edx, edx             ; Clear edx, which will hold the largest memory capacity
 .find_max_mem_area:        ; No need to check the type, the largest memory block is always usable
    mov eax, [ebx]           ; base_add_low
    add eax, [ebx+8]         ; length_low
    add ebx, 20              ; Move to the next ARDS structure in the buffer
    cmp edx, eax             ; Compare and find the maximum value, edx will always hold the largest memory capacity
    jge .next_ards
    mov edx, eax             ; edx will hold the total memory size
 
 .next_ards:
    loop .find_max_mem_area
    jmp .mem_get_ok
 
 ;------  int 15h ax = E801h Max support 4G  ------
 .try_e801:
    mov ax,E801Magic
    int 0x15
    jc .try88Method   ; same, if we failed, our last way :(
 
    mov cx,0x400	     
    mul cx 
    shl edx,16
    and eax,0x0000FFFF
    or edx,eax
    add edx, 0x100000    ; add 1MB
    mov esi,edx	         ;
 
 ;  transfer to byte storage
    xor eax,eax
    mov ax,bx		
    mov ecx, 0x10000	; => 64KB
    mul ecx		      
    add esi,eax		  
    mov edx,esi		   
    jmp .mem_get_ok
 
 ;-----------------  int 15h ah = 0x88 64M  ----------
 .try88Method: 
    ;int 15后，KB as the counts
    mov  ah, 0x88
    int  0x15
    jc .error_hlt
    and eax,0x0000FFFF
       
    mov cx, 0x400    
    mul cx
    shl edx, 16	     
    or edx, eax	    
    add edx,0x100000  
 
 .mem_get_ok:
    mov [total_mem_bytes], edx	 ; put it into the case, we will fetch it later:)

; memory checker 
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

; memory check failed 
.error_hlt:		  
   hlt

enter_loader        db "Enter the Loader Sections Success!", 0  ; Success message string
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
