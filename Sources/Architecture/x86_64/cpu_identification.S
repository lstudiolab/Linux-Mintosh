.text
.globl mintosh_cpuid
.type mintosh_cpuid, @function
mintosh_cpuid:
    pushq %rbx
    movq %rdx, %r8
    movl %edi, %eax
    movl %esi, %ecx
    cpuid
    movl %eax, 0(%r8)
    movl %ebx, 4(%r8)
    movl %ecx, 8(%r8)
    movl %edx, 12(%r8)
    popq %rbx
    ret
.size mintosh_cpuid, .-mintosh_cpuid

.section .note.GNU-stack,"",@progbits
