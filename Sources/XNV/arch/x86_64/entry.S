.text
.code64
.global xnv_arch_enter
.type xnv_arch_enter,@function
xnv_arch_enter:
    cli
    movq %rdi,%r12
    movq %rsi,%rsp
    andq $-16,%rsp
    xorq %rbp,%rbp
    movq %r12,%rdi
    call xnv_kernel_main
1:  hlt
    jmp 1b
.size xnv_arch_enter,.-xnv_arch_enter
.global xnv_arch_load_tables
.type xnv_arch_load_tables,@function
xnv_arch_load_tables:
    lgdt (%rdi)
    pushq $0x08
    leaq 1f(%rip),%rax
    pushq %rax
    lretq
1:  movw $0x10,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%ss
    xorw %ax,%ax
    movw %ax,%fs
    movw %ax,%gs
    lidt (%rsi)
    ret
.size xnv_arch_load_tables,.-xnv_arch_load_tables
.global xnv_arch_enable_paging
.type xnv_arch_enable_paging,@function
xnv_arch_enable_paging:
    movq %rdi,%cr3
    movq %cr4,%rax
    orq $0x20,%rax
    movq %rax,%cr4
    movq %cr0,%rax
    orq $0x80000000,%rax
    movq %rax,%cr0
    ret
.size xnv_arch_enable_paging,.-xnv_arch_enable_paging
