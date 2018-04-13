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
        jle     initialize1
        jmp     boundException

initialize1:
        xor     r8, r8
initialize2:
        mov     byte[a + r8], 0x30
        mov     byte[b + r8], 0x30
        mov     byte[c + r8], 0x30
        inc     r8
        cmp     r8, 64
        jl      initialize2
        mov     byte[a + 64], 0x0A              ; 给a末尾加上换行符，因为a会输出
        mov     byte[b], 0x31                   ; 将b初始化为1
        xor     r8, r8                          ; 计数器
begin:
        cmp     r8, [lower]
        je      print
        ; 进行相加
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
        je      setBlack
        cmp     r9, 1
        je      setRed
        cmp     r9, 2
        je      setGreen
        cmp     r9, 3
        je      setYellow
        cmp     r9, 4
        je      setBlue
setBlack:
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_black
        mov     rdx, color_black.len
        int     80h
        jmp     next
setRed:
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_red
        mov     rdx, color_red.len
        int     80h
        jmp     next
setGreen:
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_green
        mov     rdx, color_green.len
        int     80h
        jmp     next
setYellow:
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_yellow
        mov     rdx, color_yellow.len
        int     80h
        jmp     next
setBlue:
        mov     rax, 4
        mov     rbx, 1
        mov     rcx, color_blue
        mov     rdx, color_blue.len
        int     80h
        jmp     next
next:
        mov     rdi, format
        mov     rsi, r8
        xor     rax, rax
        call    printf
        
        cmp     r8, [upper]
        je      done
        ; 进行相加
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
a:      resb    65                  ; 存放其中一个
b:      resb    64                  ; 存放另一个
c:      resb    64                  ; 暂存中间结果

        section .data
TITLE:  db      "OS_Lab_1: Fibonacci Sequence", 10
PROMPT: db      "Please input x and y: "
ERROR:  db      "Lower bound must be less than or equal to upper bound", 10
input:  dq      0                   ; 输入
lower:  dq      0                   ; 区间左边界
upper:  dq      0                   ; 区间右边界
COLOR_COUNT:
        dq      5
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
