;binary file format

    ;dd program length
    ;entry point 
        ;dw offset
        ;dd segment address
    ;dw realloc table number
    ;realloc table
    ;  dd section start address
; ==================================

; mbr size 0x200    => 0:7c00~0:7e00
; stack size 0x1000 => 0:f000~1:0000
; heap size 0x7000  => 0:8000~0:f000

SECTION mbr align=16 vstart=0x7c00
    xor ax, ax
    mov ss, ax
    mov sp, ax  ; stack: 0:0000. Attension, first element is at 0:fffe
    mov ds, ax

    call cls

    ; === remove this display program because of space limitation ===
    ; push message
    ; call strlen
    ; add sp, 0x2

    ; push message
    ; push ax
    ; xor cx, cx
    ; push cx
    ; call display
    ; add sp, 0x6

    mov ax, 0x1
    push ax     ;read 1 sector
    mov ax, [load_addr+2]   ;push si
    push ax
    mov ax, [load_addr]     ;push ds
    push ax     ;buffer
    mov ax, [app_sector]    ;sector index low
    push ax
    mov ax, [app_sector+2]  ;sector index high
    push ax
    call read_disk
    add sp, 0xA

    mov ax, [load_addr]     ;push ds
    push ax
    mov ax, [load_addr+2]   ;push si
    push ax     ;buffer
    call load_bin
    add sp, 0x4

    mov si, [load_addr+2]   ;ATTENTION: SI must be assigned before DS change
    mov ax, [load_addr]
    mov ds, ax
    mov es, ax

    jmp far [0x04]          ; 0x04: start offset address  0x06: segment address of start address



load_bin:    ;a1: dd pointer to bin header ds:si
    push bp
    mov bp, sp
    push ds
    push si
    push bx
    push dx
    sub sp, 0x4

    mov bx, [app_sector]
    mov [bp-0xC], bx
    mov bx, [app_sector+2]
    mov [bp-0xA], bx

    mov si, [bp+0x4]
    mov ax, [bp+0x6]
    mov ds, ax

    mov ax, [si]     ;length low
    mov dx, [si+2]   ;length high
    mov bx, 512
    div bx              ;calculate the sectors that app occupied

    cmp dx, 0
    jz @1
        inc ax          ;if has reminder
    
    @1:
    dec ax              ;1 sector has been read
    cmp ax, 0
    jz @reloc
; === here we read the remain app ===
        push ax         ;number of sector
        mov ax, ds       ;push ds
        add ax, 0x20
        push ax
        push si         ;push si
                        ;buffer

        mov bx, [bp-0xC]        ;sector index low
        mov ax, [bp-0xA]    ;sector index high
        inc bx
        mov [bp-0xC], bx
        jnc @2
            inc ax
            mov [bp-0xA], ax
        @2:
        push bx         ; push low addr
        push ax         ; push high addr
        call read_disk
        add sp, 0xA

; === relocate the program ===
    @reloc:

    push ds
    push si
    call reloc
    add sp, 0x4

    add sp, 0x4
    pop dx
    pop bx
    pop si
    pop ds
    pop bp
    ret


reloc:      ;a1 ds:si  point to program header
    push bp
    mov bp, sp
    push ax
    push bx
    push cx
    push dx
    push ds
    push si

    mov si, [bp+0x4]
    mov ax, [bp+0x6]
    mov ds, ax

    mov ax, [si+0x08]       ; high addr
    mov dx, [si+0x06]       ; low addr
    call calc_base_addr
    mov [si+0x06], ax       ; here we use segment addr to replace [0x06] for long jmp at line 66 

    mov bx, si
    add bx, 0xc

    mov cx, [si+0xa]        ; table length
    cmp cx, 0
    jz @exit
    @3:
        mov ax, [0x02+bx]
        mov dx, [bx]
        call calc_base_addr
        mov [bx], ax
        add bx, 0x4
        loop @3

    @exit:
    pop si
    pop ds
    pop dx
    pop cx
    pop bx
    pop ax
    pop bp
    ret


calc_base_addr:     ;address dd ax:high word  dx:low word
                    ;ds:si must point to app header
    push bx
    push cx

    mov bx, dx
    and bx, 0xF     ; save low 4 bits of low word

    shl ax, 0xC     ; addr a:bcde,  ax = a      -->     a000
    shr dx, 0x4     ;               dx = bcde   -->     0bcd
    or ax, dx       ;               ax          -->     abcd

    mov cx, ds
    add ax, cx      ; add high word and ds

    mov dx, bx      ; low 4bits

    pop cx
    pop bx
    ret


%include "common.asm"


;message db 'Loading Program...'
;message db 'Loading'
load_addr   dw 0x1000
            dw 0x0000
app_sector  dd 100
times 510-($-$$) db 0
                 db 0x55, 0xaa