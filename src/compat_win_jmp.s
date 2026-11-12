.section .text
.globl ps4rt_setjmp
.globl ps4rt_longjmp
ps4rt_setjmp:
    movq (%rsp), %rax
    movq %rax, (%rcx)
    movq %rsp, 8(%rcx)
    movq %rbx, 16(%rcx)
    movq %rbp, 24(%rcx)
    movq %rdi, 32(%rcx)
    movq %rsi, 40(%rcx)
    movq %r12, 48(%rcx)
    movq %r13, 56(%rcx)
    movq %r14, 64(%rcx)
    movq %r15, 72(%rcx)
    movdqu %xmm6, 80(%rcx)
    movdqu %xmm7, 96(%rcx)
    movdqu %xmm8, 112(%rcx)
    movdqu %xmm9, 128(%rcx)
    movdqu %xmm10, 144(%rcx)
    movdqu %xmm11, 160(%rcx)
    movdqu %xmm12, 176(%rcx)
    movdqu %xmm13, 192(%rcx)
    movdqu %xmm14, 208(%rcx)
    movdqu %xmm15, 224(%rcx)
    xorl %eax, %eax
    ret
ps4rt_longjmp:
    movl %edx, %eax
    testl %eax, %eax
    jnz 1f
    movl $1, %eax
1:
    movq 16(%rcx), %rbx
    movq 24(%rcx), %rbp
    movq 32(%rcx), %rdi
    movq 40(%rcx), %rsi
    movq 48(%rcx), %r12
    movq 56(%rcx), %r13
    movq 64(%rcx), %r14
    movq 72(%rcx), %r15
    movdqu 80(%rcx), %xmm6
    movdqu 96(%rcx), %xmm7
    movdqu 112(%rcx), %xmm8
    movdqu 128(%rcx), %xmm9
    movdqu 144(%rcx), %xmm10
    movdqu 160(%rcx), %xmm11
    movdqu 176(%rcx), %xmm12
    movdqu 192(%rcx), %xmm13
    movdqu 208(%rcx), %xmm14
    movdqu 224(%rcx), %xmm15
    movq 8(%rcx), %rsp
    ret
