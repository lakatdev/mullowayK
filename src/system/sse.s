.global enable_sse
.type enable_sse, @function
enable_sse:
    movl %cr0, %eax
    andw $0xFFFB, %ax
    orw $0x2, %ax
    movl %eax, %cr0
    movl %cr4, %eax
    orw $3 << 9, %ax
    movl %eax, %cr4
    ret

.global memcpy_sse
.type memcpy_sse, @function
memcpy_sse:
    mov 8(%esp), %esi #src pointer
    mov 4(%esp), %edi #dest pointer

    mov 12(%esp), %edx #edx is the counter
    shr $7, %edx #divide by 128

    loop_copy:
        prefetchnta 128(%esi) #SSE2 prefetch
        prefetchnta 160(%esi)
        prefetchnta 192(%esi)
        prefetchnta 224(%esi)

        movdqa 0(%esi), %xmm0 #move data from src to registers
        movdqa 16(%esi), %xmm1
        movdqa 32(%esi), %xmm2
        movdqa 48(%esi), %xmm3
        movdqa 64(%esi), %xmm4
        movdqa 80(%esi), %xmm5
        movdqa 96(%esi), %xmm6
        movdqa 112(%esi), %xmm7

        movntdq %xmm0, 0(%edi) #move data from registers to dest
        movntdq %xmm1, 16(%edi)
        movntdq %xmm2, 32(%edi)
        movntdq %xmm3, 48(%edi)
        movntdq %xmm4, 64(%edi)
        movntdq %xmm5, 80(%edi)
        movntdq %xmm6, 96(%edi)
        movntdq %xmm7, 112(%edi)

        add $128, %esi
        add $128, %edi
        dec %edx

        jnz loop_copy
    loop_copy_end:
    ret
