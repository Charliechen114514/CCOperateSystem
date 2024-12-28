%include "boot.inc"

; ------------------------------------
;   print.inc helps print some infos
; ------------------------------------

; ------------------------------------
; In VGA Text Mode, Each Line contains 80 chars
; and 2 bytes each char promised 160 bytes are requried
; for each line, so add 160 can shift the displaying into
; next line
; ------------------------------------
%define NEW_LINE_LENGTH 160

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
;   ---------------------------------
;   | K | R | G | B | I | R | G | B |
;   ---------------------------------
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

; Oh shit, I make a difficulty in displaying
; as the data segments abolished in the using file
; so currently place as here
; then i shell modified at next release :)
; possibly...
%macro print_hello_os_protect_mode 0
    PRINT_IN_PROTECT_MODE_STYLE 'C'
    PRINT_IN_PROTECT_MODE_STYLE 'H'
    PRINT_IN_PROTECT_MODE_STYLE 'A'
    PRINT_IN_PROTECT_MODE_STYLE 'R'
    PRINT_IN_PROTECT_MODE_STYLE 'L'
    PRINT_IN_PROTECT_MODE_STYLE 'I'
    PRINT_IN_PROTECT_MODE_STYLE 'E'
    PRINT_IN_PROTECT_MODE_STYLE 0x27
    PRINT_IN_PROTECT_MODE_STYLE 'S'
    PRINT_IN_PROTECT_MODE_STYLE ' '
    PRINT_IN_PROTECT_MODE_STYLE 'O'
    PRINT_IN_PROTECT_MODE_STYLE 'S'
    PRINT_IN_PROTECT_MODE_STYLE '!'
%endmacro

%macro print_protect_mode_state 0
    PRINT_IN_PROTECT_MODE_STYLE 'N'
    PRINT_IN_PROTECT_MODE_STYLE 'O'
    PRINT_IN_PROTECT_MODE_STYLE 'W'
    PRINT_IN_PROTECT_MODE_STYLE ','
    PRINT_IN_PROTECT_MODE_STYLE 'O'
    PRINT_IN_PROTECT_MODE_STYLE 'S'
    PRINT_IN_PROTECT_MODE_STYLE ' '
    PRINT_IN_PROTECT_MODE_STYLE 'I'
    PRINT_IN_PROTECT_MODE_STYLE 'S'
    PRINT_IN_PROTECT_MODE_STYLE ' '
    PRINT_IN_PROTECT_MODE_STYLE 'I'
    PRINT_IN_PROTECT_MODE_STYLE 'N'
    PRINT_IN_PROTECT_MODE_STYLE ' '
    PRINT_IN_PROTECT_MODE_STYLE 'P'
    PRINT_IN_PROTECT_MODE_STYLE 'R'
    PRINT_IN_PROTECT_MODE_STYLE 'O'
    PRINT_IN_PROTECT_MODE_STYLE 'T'
    PRINT_IN_PROTECT_MODE_STYLE 'E'
    PRINT_IN_PROTECT_MODE_STYLE 'C'
    PRINT_IN_PROTECT_MODE_STYLE 'T'
    PRINT_IN_PROTECT_MODE_STYLE ' '
    PRINT_IN_PROTECT_MODE_STYLE 'M'
    PRINT_IN_PROTECT_MODE_STYLE 'O'
    PRINT_IN_PROTECT_MODE_STYLE 'D'
    PRINT_IN_PROTECT_MODE_STYLE 'E'
    PRINT_IN_PROTECT_MODE_STYLE '!'
%endmacro

%macro print_protect_mode_virtual 0
    PRINT_IN_PROTECT_MODE_STYLE 'N'
    PRINT_IN_PROTECT_MODE_STYLE 'O'
    PRINT_IN_PROTECT_MODE_STYLE 'W'
    PRINT_IN_PROTECT_MODE_STYLE ','
    PRINT_IN_PROTECT_MODE_STYLE 'O'
    PRINT_IN_PROTECT_MODE_STYLE 'S'
    PRINT_IN_PROTECT_MODE_STYLE ' '
    PRINT_IN_PROTECT_MODE_STYLE 'O'
    PRINT_IN_PROTECT_MODE_STYLE 'W'
    PRINT_IN_PROTECT_MODE_STYLE 'N'
    PRINT_IN_PROTECT_MODE_STYLE 'S'
    PRINT_IN_PROTECT_MODE_STYLE ' '
    PRINT_IN_PROTECT_MODE_STYLE 'V'
    PRINT_IN_PROTECT_MODE_STYLE 'I'
    PRINT_IN_PROTECT_MODE_STYLE 'R'
    PRINT_IN_PROTECT_MODE_STYLE 'T'
    PRINT_IN_PROTECT_MODE_STYLE 'U'
    PRINT_IN_PROTECT_MODE_STYLE 'A'
    PRINT_IN_PROTECT_MODE_STYLE 'L'
    PRINT_IN_PROTECT_MODE_STYLE ' '
    PRINT_IN_PROTECT_MODE_STYLE 'M'
    PRINT_IN_PROTECT_MODE_STYLE 'E'
    PRINT_IN_PROTECT_MODE_STYLE 'M'
    PRINT_IN_PROTECT_MODE_STYLE '!'
%endmacro