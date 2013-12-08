.section .text

.globl pit_wrapper

.type pit_wrapper, @function

pit_wrapper:
    #  
    #  Save all registers except %eflags, %cs, %eip, %ss, %esp,
    #  which the CPU control unit automatically saves and restores.
    # 
    cld
    pushl %es
    pushl %ds
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    # Run C handler
    call schedul
    # Restore all registers
    popl  %esi
    popl  %edi
    popl  %edx
    popl  %ecx
    popl  %ebx
    popl  %eax
    popl  %ds
    popl  %es
    iret

