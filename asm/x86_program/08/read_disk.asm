; Print Message Written at Sector 1

; mbr size 0x200    => 0:7c00~0:7e00
; stack size 0x1000 => 0:f000~1:0000
; heap size 0x7000  => 0:8000~0:f000

str_ptr equ 0x8000
line_len equ 160

SECTION mbr align=16 vstart=0x7c00
    xor ax, ax
    mov ss, ax
    mov sp, ax   ; stack: 0:0000. Attension, first element is at 0:fffe

    mov ds, ax

    call cls

    push message
    call strlen
    add sp, 0x2

    push message
    push ax
    xor cx, cx
    push cx
    call display
    add sp, 0x6

    mov ax, 0x2
    push ax     ; number for read
    mov ax, str_ptr
    push ax     ; low addr  0:8000
    mov ax, ds
    push ax     ; high addr

    mov ax, 0x1
    push ax     ; sector number low 16
    xor ax, ax
    push ax     ; sector number high 12
    call read_disk
    add sp, 0xa

; display msg1
    push str_ptr
    call strlen
    add sp, 0x2

    push str_ptr
    push ax
    push line_len
    call display
    add sp, 0x6

; display msg2
    push str_ptr+0x200
    call strlen
    add sp, 0x2

    push str_ptr+0x200
    push ax
    push line_len*2
    call display
    add sp, 0x6


    ; xor cx, cx
    ; inc cx
    ; push cx     ;read 1 sector
    ; push ds
    ; mov ax, 0x200
    ; push ax     ;buffer: ds:200

    ; dec cx    
    ; push cx
    ; inc cx
    ; push cx     ;sector number:1
    ; call read_disk
    ; add sp, 0xA

    ; mov ax, 0x200
    ; push ax
    ; mov ax, 0x50
    ; push ax
    ; xor ax, ax
    ; push ax
    ; call display
    ; add sp, 0x6

    jmp near $


%include "common.asm"


message db 'Display test!!!'
times 510-($-$$) db 0
                 db 0x55, 0xaa