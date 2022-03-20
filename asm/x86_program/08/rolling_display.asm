SECTION header vstart=0
    program_length  dd program_end

    code_entry      dw start
                    dd section.code_1.start

    realloc_tbl_len dw (header_end-code_1_segment)/4

    code_1_segment  dd section.code_1.start
    code_2_segment  dd section.code_2.start
    data_1_segment  dd section.data_1.start
    data_2_segment  dd section.data_2.start
    stack_segment   dd section.stack.start

header_end:

; ===============================================================================
SECTION code_1 align=16 vstart=0

putchar:
    push bp
    mov bp, sp
    



; ===============================================================================
SECTION code_2 align=16 vstart=0



; ===============================================================================
SECTION data_1 align=16 vstart=0

    msg0 db '  This is NASM - the famous Netwide Assembler. '
         db 'Back at SourceForge and in intensive development! '
         db 'Get the current versions from http://www.nasm.us/.'
         db 0x0d,0x0a,0x0d,0x0a
         db '  Example code for calculate 1+2+...+1000:',0x0d,0x0a,0x0d,0x0a
         db '     xor dx,dx',0x0d,0x0a
         db '     xor ax,ax',0x0d,0x0a
         db '     xor cx,cx',0x0d,0x0a
         db '  @@:',0x0d,0x0a
         db '     inc cx',0x0d,0x0a
         db '     add ax,cx',0x0d,0x0a
         db '     adc dx,0',0x0d,0x0a
         db '     inc cx',0x0d,0x0a
         db '     cmp cx,1000',0x0d,0x0a
         db '     jle @@',0x0d,0x0a
         db '     ... ...(Some other codes)',0x0d,0x0a,0x0d,0x0a
         db 0

; ===============================================================================
SECTION data_2 align=16 vstart=0

    msg1 db '  The above contents is written by LeeChung. '
         db '2011-05-06'
         db 0

; ===============================================================================
SECTION stack align=16 vstart=0

    resb 256

stack_end:

; ===============================================================================
SECTION trail align=16
program_end:
