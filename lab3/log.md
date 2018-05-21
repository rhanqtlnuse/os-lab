修改 `boot.asm` : 去掉显示 `Booting...` 和 `Ready`
修改 `loader.asm` : 去掉显示 `Loading` 
修改 `main.c` : 去掉显示 `kernel_main`
修改 `start.c` : 去掉显示 `cstart`

主要修改的部分位于 `console.c` `tty.c`
`main.c` : 修改 `TestA` ，定时清屏
`console.c` : 注释掉 `init_screen()` 中的两行
`tty.c`
- 对每个终端进行数据的初始化（input，searching，...）
- `in_process` 中增加对 `Esc` 和 `Tab` 的判断

