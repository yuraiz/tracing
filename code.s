// Can be compiled using
// as code.s -o test.o && ld -o test test.o -l System -syslibroot `xcrun -sdk macosx --show-sdk-path`
.global _main
_main:
    mov x0, #0
.loop:
    cmp x0, 0
    add x1, x1, #1
    add x2, x2, #2
    add x3, x3, #3
    add x4, x4, #4
    b.eq .loop
    
    bl _exit

_exit:
    mov x0, #0
    mov x16, #1
    svc 0
    ret
