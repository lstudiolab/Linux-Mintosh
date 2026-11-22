.text
.code64
.macro XNV_ISR n
.global xnv_isr_\n
.type xnv_isr_\n,@function
xnv_isr_\n:
    cli
    pushq $\n
    jmp xnv_exception_common
.endm
XNV_ISR 0
XNV_ISR 1
XNV_ISR 2
XNV_ISR 3
XNV_ISR 4
XNV_ISR 5
XNV_ISR 6
XNV_ISR 7
XNV_ISR 8
XNV_ISR 9
XNV_ISR 10
XNV_ISR 11
XNV_ISR 12
XNV_ISR 13
XNV_ISR 14
XNV_ISR 15
XNV_ISR 16
XNV_ISR 17
XNV_ISR 18
XNV_ISR 19
XNV_ISR 20
XNV_ISR 21
XNV_ISR 22
XNV_ISR 23
XNV_ISR 24
XNV_ISR 25
XNV_ISR 26
XNV_ISR 27
XNV_ISR 28
XNV_ISR 29
XNV_ISR 30
XNV_ISR 31
.global xnv_isr_default
xnv_isr_default:
    cli
    pushq $255
xnv_exception_common:
    movq %rsp,%rdi
    andq $-16,%rsp
    call xnv_exception_dispatch
1:  hlt
    jmp 1b
