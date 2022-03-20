    jmp near start

message db '1+2+3+...+100 = '

start:
    mov ax,0x7c0
    mov ds,ax
    mov ss,ax
    xor ax,ax
    mov sp,ax

    mov ax,0xb800
    mov es,ax

    mov si,message
    mov di,0
    mov cx,start-message
    mov bl,0x07
    @display:
        mov al,[ds:si]
        mov [es:di],al
        inc di
        mov [es:di],bl
        inc di
        inc si
        loop @display

    mov cx,100
    xor ax,ax

    @add_100:
        add ax,cx
        loop @add_100

    mov bx,10
    xor cx,cx
    @divide:
        inc cx
        xor dx,dx
        div bx
        or dx,30h
        push dx
        cmp ax,0
        jne @divide

    mov dl,0x07
    @print_number:
        pop bx
        mov [es:di],bl
        inc di
        mov [es:di],dl
        inc di
        loop @print_number

    jmp near $

times 510-($-$$) db 0
                 db 0x55,0xaa
    