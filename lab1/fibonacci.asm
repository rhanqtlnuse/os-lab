        global  main
        extern  printf

        section .text
main:
        mov     rax, 1
        mov     rdi, 1
        mov     rsi, TITLE
        mov     rdx, 29
        syscall

        mov     rax, 1
        mov     rdi, 1
        mov     rsi, PROMPT
        mov     rdx, 22
        syscall

read:
        mov     rax, 0              ; 读取一行输入
        mov     rdi, 1
        mov     rsi, input
        mov     rdx, 8
        syscall

        xor     r8, r8              ; 清空r8，将其作为指针
getLower:
        cmp     byte[input + r8], 0x20    ; 是否是空格
        je      getUpper
        movsx   r9, byte[input + r8]
        sub     r9, 0x30
        mov     r10, [lower]
        imul    r10, 10
        add     r9, r10
        mov     [lower], r9
        inc     r8
        jmp     getLower
getUpper:
        cmp     byte[input + r8 + 1], 0x0A
        je      checkBound
        movsx   r9, byte[input + r8 + 1]
        sub     r9, 0x30
        mov     r10, [upper]
        imul    r10, 10
        add     r9, r10
        mov     [upper], r9
        inc     r8
        jmp     getUpper

checkBound:
        mov     r8, [lower]
        mov     r9, [upper]
        cmp     r8, r9
        jle     initialize
        mov     rax, 1
        mov     rdi, 1
        mov     rsi, error
        mov     rdx, 54
        syscall
        jmp     done

initialize:
        xor     r8, r8          ; r8 = f(0)
        xor     r9, r9          ; r9 = f(1)
        inc     r9
        xor     r10, r10
        xor     r11, r11        ; 计数器
begin:
        cmp     r11, [lower]
        je      print
        mov     r10, r9
        add     r9, r8
        mov     r8, r10
        inc     r11
        jmp     begin
print:
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp, 8

        mov     r12, r11
        sub     r12, [lower]
        cmp     r12, 5
        jl      setColor
mod:
        sub     r12, 5
        cmp     r12, 5
        jge     mod
setColor:
        cmp     r12, 0
        je      setBlack
        cmp     r12, 1
        je      setRed
        cmp     r12, 2
        je      setGreen
        cmp     r12, 3
        je      setYellow
        cmp     r12, 4
        je      setBlue
setBlack:
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_black
        mov     rdx, color_black.len
        int     80h
        je      next
setRed:
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_red
        mov     rdx, color_red.len
        int     80h
        je      next
setGreen:
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_green
        mov     rdx, color_green.len
        int     80h
        je      next
setYellow:
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_yellow
        mov     rdx, color_yellow.len
        int     80h
        je      next
setBlue:
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_blue
        mov     rdx, color_blue.len
        int     80h
        je      next
next:
        add     rsp, 8
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp, 8

        mov     rdi, format
        mov     rsi, r8
        xor     rax, rax
        call    printf
        
        add     rsp, 8
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        
        cmp     r11, [upper]
        je      done
        mov     r10, r9
        add     r9, r8
        mov     r8, r10
        inc     r11
        jmp     print

done:
        mov     rax, 60
        xor     rdi, rdi
        syscall

toString:
        mov     r8, 0               ; 作为指针
        
        section .bss
result: resb    50                  ; 结果

        section .data
TITLE:  db      "OS_Lab_1: Fibonacci Sequence", 10
PROMPT: db      "Please input x and y: "
error:  db      "Lower bound must be less than or equal to upper bound", 10
input:  dq      0                   ; 输入
lower:  dq      0                   ; 区间左边界
upper:  dq      0                   ; 区间右边界
color_black:
        db      1Bh, '[30;1m', 0
.len    equ     $ - color_black
color_red:
        db      1Bh, '[31;1m', 0
.len    equ     $ - color_red
color_green:
        db      1Bh, '[32;1m', 0
.len    equ     $ - color_green
color_yellow:
        db      1Bh, '[33;1m', 0
.len    equ     $ - color_yellow
color_blue:
        db      1Bh, '[34;1m', 0
.len    equ     $ - color_blue

format: db      "%llu", 10, 0
