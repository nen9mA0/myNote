         ;代码清单9-1
         ;文件名：c09_1.asm
         ;文件说明：用户程序 
         ;创建日期：2011-4-16 22:03
         
;===============================================================================
SECTION header vstart=0                     ;定义用户程序头部段 
    program_length  dd program_end          ;程序总长度[0x00]
    
    ;用户程序入口点
    code_entry      dw start                ;偏移地址[0x04]
                    dd section.code.start   ;段地址[0x06] 
    
    realloc_tbl_len dw (header_end-realloc_begin)/4
                                            ;段重定位表项个数[0x0a]
    
    realloc_begin:
    ;段重定位表           
    code_segment    dd section.code.start   ;[0x0c]
    data_segment    dd section.data.start   ;[0x14]
    stack_segment   dd section.stack.start  ;[0x1c]
    
header_end:                
    
;===============================================================================
SECTION code align=16 vstart=0           ;定义代码段（16字节对齐） 
new_int_0x70:
      push ax
      push bx
      push cx
      push dx
      push es
      
  .w0:                                    
      mov al,0x0a                        ;阻断NMI。当然，通常是不必要的
      or al,0x80                          
      out 0x70,al
      in al,0x71                         ;读寄存器A
      test al,0x80                       ;测试第7位UIP 
      jnz .w0                            ;以上代码对于更新周期结束中断来说 
                                         ;是不必要的 
      xor al,al
      or al,0x80
      out 0x70,al
      in al,0x71                         ;读RTC当前时间(秒)
      push ax

      mov dl, [last_sec]
      cmp dl, al
      jz @save_sec_end
      mov [last_sec], al
      xor ax, ax
      mov [timer_count], al
@save_sec_end:

      mov al,2
      or al,0x80
      out 0x70,al
      in al,0x71                         ;读RTC当前时间(分)
      push ax

      mov al,4
      or al,0x80
      out 0x70,al
      in al,0x71                         ;读RTC当前时间(时)
      push ax

      mov al,0x0c                        ;寄存器C的索引。且开放NMI 
      out 0x70,al
      in al,0x71                         ;读一下RTC的寄存器C，否则只发生一次中断
                                         ;此处不考虑闹钟和周期性中断的情况 
      mov ax,0xb800
      mov es,ax

      pop ax
      call bcd_to_ascii
      mov bx,12*160 + 36*2               ;从屏幕上的12行36列开始显示
      mov cx, 0x07                       ;display property

      mov [es:bx],ah
      mov [es:bx+1],cl
      mov [es:bx+2],al                   ;显示两位小时数字
      mov [es:bx+3],cl

      mov al,':'
      mov [es:bx+4],al                   ;显示分隔符':'
      mov [es:bx+5],cl
      ; not byte [es:bx+5]                 ;反转显示属性 

      pop ax
      call bcd_to_ascii
      mov [es:bx+6],ah
      mov [es:bx+7],cl
      mov [es:bx+8],al                   ;显示两位分钟数字
      mov [es:bx+9],cl

      mov al,':'
      mov [es:bx+10],al                  ;显示分隔符':'
      mov [es:bx+11],cl
      ; not byte [es:bx+11]                ;反转显示属性

      pop ax
      call bcd_to_ascii
      mov [es:bx+12],ah
      mov [es:bx+13],cl
      mov [es:bx+14],al                  ;显示两位小时数字
      mov [es:bx+15],cl

      mov al, '_'
      mov [es:bx+16],al
      mov [es:bx+17],cl

      mov al,[timer_count]
      mov dl, al
      inc dl
      mov [timer_count], dl
      add al, 0x30

@display_point:
      mov [es:bx+18],al
      mov [es:bx+19],cl

      mov al,0x20                        ;中断结束命令EOI 
      out 0xa0,al                        ;向从片发送 
      out 0x20,al                        ;向主片发送 

      pop es
      pop dx
      pop cx
      pop bx
      pop ax

      iret

;-------------------------------------------------------------------------------
bcd_to_ascii:                            ;BCD码转ASCII
                                         ;输入：AL=bcd码
                                         ;输出：AX=ascii
      mov ah,al                          ;分拆成两个数字 
      and al,0x0f                        ;仅保留低4位 
      add al,0x30                        ;转换成ASCII 

      shr ah,4                           ;逻辑右移4位 
      and ah,0x0f                        
      add ah,0x30

      ret

;-------------------------------------------------------------------------------
start:
      mov ax,[stack_segment]
      mov ss,ax
      mov sp,ss_pointer
      mov ax,[data_segment]
      mov ds,ax
      
      mov bx,init_msg                    ;显示初始信息 
      call put_string

      mov bx,inst_msg                    ;显示安装信息 
      call put_string
      
      mov al,0x70
      mov bl,4
      mul bl                             ;计算0x70号中断在IVT中的偏移
      mov bx,ax                          

      cli                                ;防止改动期间发生新的0x70号中断

      push es
      mov ax,0x0000
      mov es,ax
      mov word [es:bx],new_int_0x70      ;偏移地址。
                                          
      mov word [es:bx+2],cs              ;段地址
      pop es

      ; set register A
      ; test UIP(Update In Progress)
      test_0a:
      mov al, 0x0a
      or al, 0x80
      out 0x70, al
      in al, 0x71
      test al, 0x80
      jnz test_0a

      mov ah, al
      or ah, 0x0e       ; set rate select period

      mov al, 0x0a
      or al, 0x80
      out 0x70, al
      mov al, ah
      out 0x71, al


      mov al,0x0b                        ;RTC寄存器B
      or al,0x80                         ;阻断NMI 
      out 0x70,al
      mov al,0x42                        ;设置寄存器B，允许周期性中断，BCD码，24小时制 
      out 0x71,al

      mov al,0x0c
      out 0x70,al
      in al,0x71                         ;读RTC寄存器C，复位未决的中断状态

      in al,0xa1                         ;读8259从片的IMR寄存器 
      and al,0xfe                        ;清除bit 0(此位连接RTC)
      out 0xa1,al                        ;写回此寄存器 

      sti                                ;重新开放中断 

      mov bx,done_msg                    ;显示安装完成信息 
      call put_string

      mov bx,tips_msg                    ;显示提示信息
      call put_string
      
      mov cx,0xb800
      mov ds,cx
      mov byte [12*160 + 33*2],'@'       ;屏幕第12行，35列
       
 .idle:
      hlt                                ;使CPU进入低功耗状态，直到用中断唤醒
      ; not byte [12*160 + 33*2+1]         ;反转显示属性 
      jmp .idle

;-------------------------------------------------------------------------------
put_string:                              ;显示串(0结尾)。
                                         ;输入：DS:BX=串地址
         mov cl,[bx]
         or cl,cl                        ;cl=0 ?
         jz .exit                        ;是的，返回主程序
         push cx
         call putchar
         add sp, 0x2
         inc bx                          ;下一个字符 
         jmp put_string

   .exit:
         ret

%include "display.asm"

;===============================================================================
SECTION data align=16 vstart=0

    init_msg       db 'Starting...',0x0d,0x0a,0
                   
    inst_msg       db 'Installing a new interrupt 70H...',0
    
    done_msg       db 'Done.',0x0d,0x0a,0

    tips_msg       db 'Clock is now working.',0

    timer_count    db 0

    last_sec       db 0
;===============================================================================
SECTION stack align=16 vstart=0
           
                 resb 256
ss_pointer:
 
;===============================================================================
SECTION program_trail
program_end: