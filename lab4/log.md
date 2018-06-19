# 修改记录
- `lib`
  - 在 `klib.c` 中添加 `disp_color_int(input: int, color: int): void`
  - 在 `kliba.asm` 中添加 `enable_int` 和 `disable_int`
- `include`
  - `const.h`
    - 修改 `NR_SYS_CALL` 为 7
  - `global.h`
    - 添加 PV 操作相关变量
      - `semaphore customers`
      - `semaphore barbers`
      - `semaphore mutex`
      - `int waiting`
      - `int customer_id`
  - `proc.h`
    - 添加 `semaphore` 结构体
    - 修改 `NR_TASKS` 为 5
    - 添加 `STACK_SIZE_PROCESSD` 和 `STACK_SIZE_PROCESSE`
    - 修改 `STACK_SIZE_TOTAL` 为加上 `STACK_SIZE_PROCESSD` 和 `STACK_SIZE_PROCESSE` 之后的值
  - `proto.h`
    - 添加进程原型和系统调用原型
- `kernel`
  - `global.c`
    - 在 `task_table[]` 中添加 `ProcessD` 和 `ProcessE` 的信息
    - 在 `sys_call_table[]` 中添加系统调用
  - `main.c`
    - 添加 `ProcessD(): void` 和 `ProcessE(): void`
    - 为所有进程分配相同的默认优先级和时间片
    - 实现 PV 操作
    - `initialize()` 用于初始化信号量相关变量
    - `clear_screen()` 用于清屏
  - `proc.c`
    - 实现系统调用
  - `syscall.asm`
    - 实现系统调用内核函数
  - `clock.c`
    - 添加睡眠相关代码（睡眠时剩余时间片不能减少、睡眠时间为 0 时不能再减少）
- `Makefile`
  - 添加 `run` 参数

# 遇到的坑
- 编译时提示 `error: redefinition of 'xxx'`
  - 在 `proc.h` 头尾加上 `#ifndef` `#define` `#endif`

# 问题
- 为什么 `proc.c` 的 `sys_sem_p` 要调用 `milli_delay(-10)`
- 系统调用中断向量号为什么是 `0x90` ？
- `proc.c` 中 `schedule` ？
- `TestB` 无限循环，似乎是 `ticks` 的问题
- `kernel.asm` 中 `sys_call`
```
call    save

sti

call    [sys_call_table + eax * 4]
mov     [esi + EAXREG - P_STACKBASE], eax

cli

ret
```
```
call    save
push	  dword	[p_proc_ready]
sti

push	  ecx
push	  ebx
call    [sys_call_table + eax * 4]
add 	  esp, 4 * 3

mov     [esi + EAXREG - P_STACKBASE], eax
cli
ret
```