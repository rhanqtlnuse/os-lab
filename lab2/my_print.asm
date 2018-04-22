; 输出格式：
; |
; |- house
; |  |- room
; |  |  |- kitchen
; |  |     |- path.txt
; |  |  |- bed.txt
; |  |- table.txt
; |  |- chair.txt
; |- animal
; |  |- bird
; |  |- cat.txt
; |- river.txt
; |- mountain.txt
        global  my_print
        section .text
my_print:

        section .data
COLOR_ORDINARY:
        db      1Bh, '[32;1m', 0
.len    equ     $ - COLOR_ORDINARY
COLOR_DIRECTORY:
        db      1Bh, '[34;1m', 0
.len    equ     $ - COLOR_DIRECTORY
