%include "asm/boot.inc"

; ------------------------------------
;   print.inc helps print some infos
; ------------------------------------

; ------------------------------------
; In VGA Text Mode, Each Line contains 80 chars
; and 2 bytes each char promised 160 bytes are requried
; for each line, so add 160 can shift the displaying into
; next line
; ------------------------------------
%define NEW_LINE_LENGTH         160

; base offser print of the 
%define BASE_PRINT              NEW_LINE_LENGTH * 3

; ------------------------------------
;   SET_PROTECT_PRINT_LINE
;   params: line offset in the protected mode
; ------------------------------------
%macro SET_PROTECT_PRINT_LINE 1
    mov di, BASE_PRINT + %1 * NEW_LINE_LENGTH
%endmacro

; ------------------------------------
;   PRINT_IN_PROTECT_MODE
;   params1: the Char display
;   params2: the Style follows the VGA Settings
; ------------------------------------

%macro PRINT_IN_PROTECT_MODE 2
    mov byte [gs:di], %1
    inc di
    mov byte [gs:di], %2
    inc di
%endmacro

; ------------------------------------
;   PRINT_IN_PROTECT_MODE
;   params1: the Char display
;   Protect Mode Print Style
%define PRO_PRINT_STYLE         0x0A
; ------------------------------------
%macro PRINT_IN_PROTECT_MODE_STYLE 1
    mov byte [gs:di], %1
    inc di
    mov byte [gs:di], PRO_PRINT_STYLE
    inc di
%endmacro

