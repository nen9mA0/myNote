SECTION mbr align=16 vstart=0x7c00
    mov ax, 0x1000
    mov ss, ax
    xor sp, sp
    xor bp, bp

    mov bx, msg0
    @1:
    mov ax, [bx]
    test ax, ax
    jz @exit
        push ax
        call putchar
        add sp, 0x02
        inc bx
        jmp @1
    @exit:
    jmp $

; param1 char
putchar:
    push bp
    mov bp, sp

    push ds
    push es
    push bx
    push cx
    push dx
    push si
    push di

    mov ax, 0xb800
    mov ds, ax
    mov es, ax

    call getpos

    mov bx, [bp+4]      ; get char
    and bx, 0x00ff
    cmp bl, 0x0a
    jz @a
    cmp bl, 0x0d
    jz @d
    inc ax              ; handle normal character
    jmp @roll

    @a:                 ; handle 0x0a newline
    add ax, 80          ; one line has 80 char
    xor bl, bl
    jmp @roll

    @d:                 ; handle 0x0d enter
    xor dx, dx
    mov cx, 80
    div cx
    mul cl              ; get multiple of 80
    xor bl, bl

    @roll:
    cmp ax, 2000        ; compare with 80*25
    jna @put
    ; roll 1 line
    mov di, 0           ; line 0
    mov si, 160         ; line 1, 80*2B
    mov cx, 1920        ; 80*24
    rep movsw
    ; clr last line
    mov di, 3840        ; 1920*2
    mov cx, 80
    push ax
    xor ax, ax
    rep stosw
    pop ax

    sub ax, 80          ; sub 1 line of index

    @put:
    test bl, bl         ; if is 0x0a or 0x0d
    jz @set
    mov si, ax
    shl si, 1           ; index * 2
    mov [si], bl
    inc si
    mov bl, 0x07
    mov [si], bl
    inc si
    @set:
    call setpos

    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop es
    pop ds
    pop bp
    ret


; return ax
getpos:
    push dx
    mov dx, 0x3d4
    mov al, 0x0e
    out dx, al

    mov dx, 0x3d5
    in al, dx
    mov ah, al

    mov dx, 0x3d4
    mov al, 0x0f
    out dx, al

    mov dx, 0x3d5
    in al, dx
    pop dx
    ret

; ax: new cursor position
setpos:
    push bx
    push dx
    mov bx, ax

    mov dx, 0x3d4
    mov al, 0x0f
    out dx, al

    mov al, bl
    mov dx, 0x3d5
    out dx, al

    mov dx, 0x3d4
    mov al, 0x0e
    out dx, al

    mov al, bh
    mov dx, 0x3d5
    out dx, al

    pop dx
    pop bx
    ret
    



msg0 db '  This is NASM - the famous Netwide Assembler. '
        db 'Back at SourceForge and in intensive development! '
        db 'Get the current versions from http://www.nasm.us/.'
        db 0x0d,0x0a,0x0d,0x0a
        db '  Example code for calculate 1+2+...+1000:',0x0d,0x0a,0x0d,0x0a
        db '     xor dx,dx',0x0d,0x0a
        db '     xor ax,ax',0x0d,0x0a
        db '     xor cx,cx',0x0d,0x0a
        db 0

times 510-($-$$)    db 0
                    db 0x55, 0xaa