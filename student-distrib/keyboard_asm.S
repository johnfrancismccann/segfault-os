# 
#   The keyboard files' purpose is to handle interrupts coming
#   from the hardware RTC timer.
#   keyboard_asm.S is an interrupt wrapper called by the IDT, which
#   then executes the keyboard.c handler.  The wrapper is necessary
#   to handle the returns and state-saving interrupt handling
#   parts.
# 

.section .text

.globl kbd_wrapper

.type kbd_wrapper, @function

kbd_wrapper:
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
    call kbd_handle
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
