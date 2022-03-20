; clear
cls:            ;para none
    push cx
    push si
    push es

    mov ax, 0xb800
    mov es, ax
    xor si, si
    xor ax, ax
    mov cx, 0x7D0  ;80*25*2, loop 80*25 times

    @loop1:
        mov [es:si], ax
        add si, 2
        loop @loop1
    xor ax,  ax      ; return 0

    pop es
    pop si
    pop cx
    ret

; strlen
strlen:         ;para a1:pointer
    push bp
    mov bp, sp

    push bx
    push si
    
    mov si, [bp+0x4]
    mov ax, si
    @l:
        mov bl, [ds:si]
        inc si
        test bl, bl
        jnz @l
    dec si
    sub si, ax
    mov ax, si

    pop si
    pop bx
    pop bp
    ret

; display
display:        ;parameter: a1:framebuffer pointer | a2:length  a3:position
    push bp
    mov bp, sp
    push bx
    push cx
    push di
    push si
    push es

    mov ax, 0xb800
    mov es, ax

    mov al, 0x07
    mov si, [bp+0x8]
    mov cx, [bp+0x6]
    mov di, [bp+0x4]

    test cx, cx ; if is zero
    jz @copy_end

    @copy:
        mov bl, [ds:si]
        mov [es:di], bl
        inc di
        mov [es:di], al
        inc di
        inc si
        loop @copy

    @copy_end:
    mov ax, si

    pop es
    pop si
    pop di
    pop cx
    pop bx
    pop bp
    ret

; read disk
; 4B address parameter: first push low addr, then push high addr(or segment pointer)
read_disk:      ;para a1:number of read sector 2B(take 1B) | a2:buffer 4B ds:si little endian | a3:sector number 4B
    push bp
    mov bp, sp
    push bx
    push cx
    push dx
    push si
    push ds
;===============send cmd===============
    mov ax, [bp+0xC]
    mov bx, ax              ; save sector number in bx
    mov dx, 0x1f2            ; 0x1f2  number of sector to operate  <== a1
    out dx, al
    mov ax, [bp+0x6]         ; a3 sector number low 16b
    inc dx                  ; 0x1f3 LBA addr:
    out dx, al               ;           0~7
    mov al, ah
    inc dx                  ; 0x1f4 LBA addr:
    out dx, al               ;           8~15
    mov ax, [bp+0x4]
    inc dx                  ; 0x1f5 LBA addr:
    out dx, al               ;           16~23
    mov al, ah

    and al, 0xef             ; clr bit 4
    or al, 0xe0              ; set bit 5~7
    inc dx                  ; 0x1f6 | 111 | 0 | LBA addr 24~27
    out dx, al

    mov al, 0x20
    inc dx                  ; 0x1f7 CMD/STATUS: 0x20 Read
    out dx, al
;====================================
    mov ax, [bp+0x8]
    mov dx, [bp+0xA]
    mov ds, ax
    mov si, dx
;   from now, the segment pointer may be change, so we will not use any memory data

;   check if is ready for reading
    mov dx, 0x1f7            ; 0x1f7 for STATUS
                            ;          7   6 5 4   3   2 1   0
                            ;       | BSY | | | | DRQ | | | ERR |
    @wait1:
        in al, dx
        ;and al, 0x09
        ;cmp al, 0x01 ;if error
        ;jnz @error
        and al, 0x88
        cmp al, 0x08
        jnz @wait1

;   here we move the ds in reading every sector
;    -----------  0
;   |           |   <== ds:si   (1) read first sector
;    -----------  256   |
;   |           |       |__si       when reading a sector, change si only
;    -----------  512
;   |           |   <== ds:si   (2) read second sector, ds += 0x20(512=0x200) si-=512
;    -----------  768
    mov dx, 0x1f0
;   now bx save the number of sector to read
    @readall:
        mov cx, 256  ; 256 Words = 512 Bytes
        push si     ; save the old si
        @readone:
            in ax, dx
            mov [ds:si], ax
            add si, 2
            loop @readone

        xor ax, ax   ; finish read
        pop si      ; pop the old si. because ds has changed, we can use old si directly
        dec bx
        test bx, bx
        jz @jumpout

        mov ax, ds
        add ax, 0x20
        mov ds, ax
        jmp @readall

    ; mov dx, 0x1f0
    ; shl cx, 8    ;cx = cx*512Byte = cx*256Word
    ; @readw:
    ;     in ax, dx
    ;     mov [ds:si], ax
    ;     add si, 2
    ;     loop @readw
    ; xor ax, ax
    ;jmp @jumpout

    ;@error:
    ;    inc dx
    ;    in al, dx
    @jumpout:
    pop ds
    pop si
    pop dx
    pop cx
    pop bx
    pop bp
    ret


