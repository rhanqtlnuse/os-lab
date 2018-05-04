; my_print(long color, char *format, ...)
; 1. color有 0、1、2 三个值，分别代表默认颜色（白色）、普通文件、目录
; 2. 实现占位符 %d
; 3. 假定不会出现错误且只有一个参数
        global  my_print
        section .text

my_print:
        ; 设置颜色
        call    setColor
        
        mov     rcx, 63
init:
        mov     byte[strnum + rcx], 0
        dec     rcx
        cmp     rcx, 0
        jge     init

        ; 处理格式化串
        ; rcx作为指针
        xor     rcx, rcx
        ; 本次输出起始位置
        xor     r10, r10
processFormat:
        mov     bl, byte[rsi + rcx]
        cmp     bl, 0x00        ; 结束
        je      processEOS
        cmp     bl, 0x25        ; 百分号
        je      processPlaceholder
        inc     rcx
next:
        jmp     processFormat

processEOS:
        mov     rax, 1
        mov     rdi, 1
        add     rsi, r10
        mov     rdx, rcx        ; 本次输出长度 = 当前指针位置 - 本次输出起始位置 + 1
        sub     rdx, r10
        inc     rdx
        syscall

        jmp     exit

processPlaceholder:
        dec     rcx

        push    rax
        push    rdi
        push    rsi
        push    rdx
        push    rcx

        mov     rax, 1
        mov     rdi, 1
        add     rsi, r10
        mov     rdx, rcx        ; 本次输出长度 = 当前指针位置 - 本次输出起始位置 + 1
        sub     rdx, r10
        inc     rdx
        syscall

        pop     rcx
        pop     rdx
        pop     rsi
        pop     rdi
        pop     rax

        add     rcx, 2          ; 将指针移到百分号的下一个字符
        mov     r10, rcx
        inc     r10
        mov     bl, byte[rsi + rcx]
        cmp     bl, 0x64        ; %d
        jne     endProcessPlaceholder
placeholderD:
        push    rax
        push    rdi
        push    rsi
        push    rdx
        push    rcx

        mov     rdi, rdx
        jmp     toString
dPrint:
        mov     rax, 1
        mov     rdi, 1
        mov     rsi, strnum
        mov     rdx, 64
        syscall

        pop     rcx
        pop     rdx
        pop     rsi
        pop     rdi
        pop     rax
        
        jmp     endProcessPlaceholder
endProcessPlaceholder:
        inc     rcx
        jmp     next

toString:
        cmp     rdi, 0
        je      zero
        mov     rcx, 63
        mov     rax, rdi
        mov     rdi, 10
while:
        xor     rbx, rbx
        xor     rdx, rdx
        cmp     rax, 0
        je      endToString
        div     rdi
        add     dl, 0x30
        mov     byte[strnum + rcx], dl
        dec     rcx
        jmp     while
zero:
        mov     byte[strnum + 63], 0x30
endToString:
        jmp     dPrint

setColor:
        push    rax
        push    rdi
        push    rsi
        push    rdx
        push    rcx
        
        mov     rax, 4
        mov     rbx, 1

        cmp     rdi, 0
        je      setDefault
        cmp     rdi, 1
        je      setOrdinary
        cmp     rdi, 2
        je      setDirectory
        cmp     rdi, 3
        je      setDebug
setDefault:
        mov     rcx, COLOR_DEFAULT
        mov     rdx, COLOR_DEFAULT.len
        int     80h
        jmp     endSetColor
setOrdinary:
        mov     rcx, COLOR_ORDINARY
        mov     rdx, COLOR_ORDINARY.len
        int     80h
        jmp     endSetColor
setDirectory:
        mov     rcx, COLOR_DIRECTORY
        mov     rdx, COLOR_DIRECTORY.len
        int     80h
        jmp     endSetColor
setDebug:
        mov     rcx, COLOR_DEBUG
        mov     rdx, COLOR_DEBUG.len
        int     80h
        jmp     endSetColor
endSetColor:
        pop     rcx
        pop     rdx
        pop     rsi
        pop     rdi
        pop     rax
        ret

exit:
        ret

        section .data
COLOR_DEFAULT:  
        db      1Bh, '[37;0m', 0
.len    equ     $ - COLOR_DEFAULT
COLOR_ORDINARY:
        db      1Bh, '[32;1m', 0
.len    equ     $ - COLOR_ORDINARY
COLOR_DIRECTORY:
        db      1Bh, '[34;1m', 0
.len    equ     $ - COLOR_DIRECTORY
COLOR_DEBUG:
        db      1Bh, '[33;1m', 0
.len    equ     $ - COLOR_DEBUG

DEFAULT REL
        section .bss
strnum  resb    64
