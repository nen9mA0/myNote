; mbr size 0x200    => 0:7c00~0:7e00

; code  size 0x1000 => 0
; stack size 0x1000 => 0:6c00~0:7bff

SECTION mbr align=16 vstart=0
mbr_start:
    ; set stack
    mov ax, 0x6c00
    mov ss, ax
    mov ax, 0x1000
    mov bp, ax
    mov sp, bp

    call cls

    mov ax, [0x7c00+mbr_length]
    push ax     ; sector to read
    mov ax, 0x7e0
    push ax     ; load addr high
    xor ax, ax
    push ax     ; load addr low

    push ax     ; sector index high
    inc ax
    push ax     ; sector index low
    call read_disk
    add sp, 0xA

    xor ax, ax
    call setpos ; reset position

    push init_str
    call putstr
    add sp, 0x2

    jmp init_start


%include "common.asm"
%include "display.asm"


mbr_length  dw (init_end-mbr_start)/512
init_str  db "Init ..."

times 510-($-$$) db 0
                 db 0x55, 0xaa

; ==== init section ====
init_start:
    ; load gdt
    lgdt [0x7c00+gdtr]

    ; enable A20
    in al, 0x92
    or al, 0x02
    out 0x92, al

    ; disable interrupt because the descriptor has changed
    ; in this program we don't enable it again because we didn't handle IVT
    cli

    ; change Protection enable
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; now, we have entered the protection mode
    ; use a jmp far to flush pipeline, as long as flush sreg cs
    ; 0x0008: index=0x0001 TI=0 RPL=00
    jmp dword 0x0008:flush

[bits 32]
flush:
    ; now, everything have changed

    ; load ss
    ; 0x0010: index=0x0002 TI=0 RPL=00
    mov ax, 0x0010
    mov ss, ax

    ; load ds
    ; 0x0018: index=0x0003 TI=0 RPL=00
    mov ax, 0x0018
    mov ds, ax


    ; we have load all new 
    mov al, 0x07
    mov byte [0x00], 'H'
    mov byte [0x02], 'e'
    mov byte [0x04], 'l'
    mov byte [0x06], 'l'
    mov byte [0x08], 'o'
    mov byte [0x0a], ' '
    mov byte [0x0c], 'W'
    mov byte [0x0e], 'o'
    mov byte [0x10], 'r'
    mov byte [0x12], 'l'
    mov byte [0x14], 'd'
    mov byte [0x16], '!'
    mov ecx, 0xc
    mov esi, 0x1
@loop_1:
    mov byte [esi], al
    add esi, 0x2
    loop @loop_1

    jmp $


gdtr    dw (gdt_end - gdt_begin)
        dd gdt_begin+0x7c00

; | base 31~24 | G | D/B | L | AVL | limit 19~16 | P | DPL | S | TYPE | base 23~16 |
; | base 15~0                                    | limit 15~0                      |

gdt_begin:
gdt_null    dd 0, 0
; base 0x00007c00 limit=0x005ff G=0 D=1 L=0 AVL=0 P=1 DPL=00 S=1 TYPE=1000
; length=0x600
gdt_code    dd 0x7c0005ff
            dd 0x00409800
; base 0x00007c00 limit=0xFFFFE G=1 D=1 L=0 AVL=0 P=1 DPL=00 S=1 TYPE=0110
; length=0x1000
gdt_stack   dd 0x7c00fffe
            dd 0x00cf9600
; base 0x000b8000 limit=0x00F9F G=0 D=1 L=0 AVL=0 P=1 DPL=00 S=1 TYPE=0010
; length=0xFA0
; point to framebuffer
gdt_data    dd 0x80000f9f
            dd 0x0040920b
gdt_end:

init_end:
