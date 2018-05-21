        global  main

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
        mov     rax, 0                          ; 读取一行输入
        mov     rdi, 1
        mov     rsi, input
        mov     rdx, 8
        syscall

        xor     r8, r8                          ; 清空r8，将其作为指针
getLower:
        cmp     byte[input + r8], 0x20          ; 是否是空格
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
        jle     initialize1
        jmp     boundException

initialize1:
        mov     r8, 63
initialize2:
        mov     byte[a + r8], 0x30
        mov     byte[b + r8], 0x30
        mov     byte[c + r8], 0x30
        dec     r8
        cmp     r8, 0
        jge     initialize2
        mov     byte[a + 64], 0x0A              ; 给a末尾加上换行符，因为a会输出
        mov     byte[b + 63], 0x31              ; 将b初始化为1
        xor     r8, r8                          ; 计数器，记录a是fibonacci的第几项
begin:
        cmp     r8, [lower]
        je      print
        ; 将b当前的值暂存
        mov     rbx, qword[b]
        mov     rcx, qword[b + 8]
        mov     r10, qword[b + 16]
        mov     r11, qword[b + 24]
        mov     r12, qword[b + 32]
        mov     r13, qword[b + 40]
        mov     r14, qword[b + 48]
        mov     r15, qword[b + 56]
        mov     qword[c], rbx
        mov     qword[c + 8], rcx
        mov     qword[c + 16], r10
        mov     qword[c + 24], r11
        mov     qword[c + 32], r12
        mov     qword[c + 40], r13
        mov     qword[c + 48], r14
        mov     qword[c + 56], r15
        mov     r9, 63                          ; 指针
addition:
        cmp     r9, 0
        jl      begin2
        mov     al, 0x00
        add     al, byte[a + r9]
        add     al, byte[b + r9]
        add     al, byte[carry]
        sub     al, 0x30
        cmp     al, 0x3A
        jl      clearCarry
        sub     al, 0x0A
        mov     byte[carry], 0x01
        jmp     restore
clearCarry:
        mov     byte[carry], 0x00
restore:
        mov     byte[b + r9], al
        dec     r9
        jmp     addition
begin2:
        ; 还原
        mov     rbx, qword[c]
        mov     rcx, qword[c + 8]
        mov     r10, qword[c + 16]
        mov     r11, qword[c + 24]
        mov     r12, qword[c + 32]
        mov     r13, qword[c + 40]
        mov     r14, qword[c + 48]
        mov     r15, qword[c + 56]
        mov     qword[a], rbx
        mov     qword[a + 8], rcx
        mov     qword[a + 16], r10
        mov     qword[a + 24], r11
        mov     qword[a + 32], r12
        mov     qword[a + 40], r13
        mov     qword[a + 48], r14
        mov     qword[a + 56], r15
        inc     r8
        jmp     begin
print:
        mov     r9, r8
        sub     r9, [lower]
        cmp     r9, [COLOR_COUNT]
        jl      setColor
mod:
        sub     r9, [COLOR_COUNT]
        cmp     r9, [COLOR_COUNT]
        jge     mod
setColor:
        cmp     r9, 0
        je      setRed
        cmp     r9, 1
        je      setGreen
        cmp     r9, 2
        je      setYellow
        cmp     r9, 3
        je      setBlue
setRed:
        push    r8
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_red
        mov     rdx, color_red.len
        int     80h
        pop     r8
        jmp     next
setGreen:
        push    r8
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_green
        mov     rdx, color_green.len
        int     80h
        pop     r8
        jmp     next
setYellow:
        push    r8
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_yellow
        mov     rdx, color_yellow.len
        int     80h
        pop     r8
        jmp     next
setBlue:
        push    r8
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_blue
        mov     rdx, color_blue.len
        int     80h
        pop     r8
        jmp     next
next:
        mov     r9, -1                          ; 清空r9，作为最高位的指示
findHighest:
        inc     r9
        cmp     r9, 63
        je      calculate
        cmp     byte[a + r9], 0x30
        je      findHighest
calculate:
        mov     r10, 0                          ; r10 = 0
        sub     r10, r9                         ; r10 = -r9
        add     r10, 65                         ; r10 = 65 - r9，即要输出的长度，数本身64位，加一位换行符

        mov     rax, 1
        mov     rdi, 1
        mov     rsi, a
        add     rsi, r9
        mov     rdx, r10
        syscall

        cmp     r8, [upper]
        je      done
        mov     rbx, qword[b]
        mov     rcx, qword[b + 8]
        mov     r10, qword[b + 16]
        mov     r11, qword[b + 24]
        mov     r12, qword[b + 32]
        mov     r13, qword[b + 40]
        mov     r14, qword[b + 48]
        mov     r15, qword[b + 56]
        mov     qword[c], rbx
        mov     qword[c + 8], rcx
        mov     qword[c + 16], r10
        mov     qword[c + 24], r11
        mov     qword[c + 32], r12
        mov     qword[c + 40], r13
        mov     qword[c + 48], r14
        mov     qword[c + 56], r15
        mov     r9, 63                          ; 指针
addition2:
        cmp     r9, 0
        jl      print2
        mov     al, 0x00
        add     al, byte[a + r9]
        add     al, byte[b + r9]
        add     al, byte[carry]
        sub     al, 0x30
        cmp     al, 0x3A
        jl      clearCarry2
        sub     al, 0x0A
        mov     byte[carry], 0x01
        jmp     restore2
clearCarry2:
        mov     byte[carry], 0x00
restore2:
        mov     byte[b + r9], al
        dec     r9
        jmp     addition2
print2:
        mov     rbx, qword[c]
        mov     rcx, qword[c + 8]
        mov     r10, qword[c + 16]
        mov     r11, qword[c + 24]
        mov     r12, qword[c + 32]
        mov     r13, qword[c + 40]
        mov     r14, qword[c + 48]
        mov     r15, qword[c + 56]
        mov     qword[a], rbx
        mov     qword[a + 8], rcx
        mov     qword[a + 16], r10
        mov     qword[a + 24], r11
        mov     qword[a + 32], r12
        mov     qword[a + 40], r13
        mov     qword[a + 48], r14
        mov     qword[a + 56], r15
        inc     r8
        jmp     print

boundException:
        mov     rax, 1
        mov     rdi, 1
        mov     rsi, ERROR
        mov     rdx, 54
        syscall

done:
        mov     rax, 60
        xor     rdi, rdi
        syscall
        
        section .bss
a:      resb    65                              ; 存放其中一个
b:      resb    64                              ; 存放另一个
c:      resb    64                              ; 暂存中间结果

        section .data
TITLE:  db      "OS_Lab_1: Fibonacci Sequence", 10
PROMPT: db      "Please input x and y: "
ERROR:  db      "Lower bound must be less than or equal to upper bound", 10

input:  dq      0                               ; 输入
lower:  dq      0                               ; 区间左边界
upper:  dq      0                               ; 区间右边界
carry:  db      0x00                            ; 进位

COLOR_COUNT:
        dq      4
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
