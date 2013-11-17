/*
 *  The purpose of this file is to contain all IDT handler
 *  functions called from the IDT and set up in setup_idt.c.
 */

#define ASM 1
#include "x86_desc.h"

.section .text

.globl idt_div_error, idt_debug, idt_nmi, idt_breakpoint, idt_overflow
.globl idt_bound, idt_invalid_op, idt_device_not_available, idt_double_fault
.globl idt_coprocessor_segment_overrun, idt_invalid_TSS, idt_segment_not_present
.globl idt_stack_segment, idt_general_protection, idt_page_fault
.globl idt_coprocessor_error, idt_alignment_check, idt_system_call

.type idt_div_error, @function
.type idt_debug, @function
.type idt_nmi, @function
.type idt_breakpoint, @function
.type idt_overflow, @function
.type idt_bound, @function
.type idt_invalid_op, @function
.type idt_device_not_available, @function
.type idt_double_fault, @function
.type idt_coprocessor_segment_overrun, @function
.type idt_invalid_TSS, @function
.type idt_segment_not_present, @function
.type idt_stack_segment, @function
.type idt_general_protection, @function
.type idt_page_fault, @function
.type idt_coprocessor_error, @function
.type idt_alignment_check, @function
.type idt_system_call, @function

idt_div_error:
    pushl $0 #no error code
    call do_idt_div_error
    iret

idt_debug:
    pushl $0 #no error code
    call do_idt_debug
    iret

idt_nmi:
    pushl $0 #no error code
    call do_idt_nmi
    iret

idt_breakpoint:
    pushl $0 #no error code
    call do_idt_breakpoint
    iret

idt_overflow:
    pushl $0 #no error code
    call do_idt_overflow
    iret

idt_bound:
    pushl $0 #no error code
    call do_idt_bound
    iret

idt_invalid_op:
    pushl $0 #no error code
    call do_idt_invalid_op
    iret

idt_device_not_available:
    pushl $0 #no error code
    call do_idt_device_not_available
    iret

idt_double_fault:
    call do_idt_double_fault
    iret

idt_coprocessor_segment_overrun:
    pushl $0 #no error code
    call do_idt_coprocessor_segment_overrun
    iret

idt_invalid_TSS:
    call do_idt_invalid_TSS
    iret

idt_segment_not_present:
    call do_idt_segment_not_present
    iret

idt_stack_segment:
    call do_idt_stack_segment
    iret

idt_general_protection:
    call do_idt_general_protection
    iret

idt_page_fault:
    call do_idt_page_fault
    iret

idt_coprocessor_error:
    pushl $0 #no error code
    call do_idt_coprocessor_error
    iret

idt_alignment_check:
    call do_idt_alignment_check
    iret

# saves all registers except eax
#define SAVE_ALL \
    pushl %fs;\
    pushl %es;\
    pushl %ds;\
    pushl %edi;\
    pushl %esi;\
    pushl %edx;\
    pushl %ecx;\
    pushl %ebx;

# restores all registers except eax
#define RESTORE_ALL \
    popl %ebx;\
    popl %ecx;\
    popl %edx;\
    popl %esi;\
    popl %edi;\
    popl %ds;\
    popl %es;\
    popl %fs;

idt_system_call:
# @@@@interrupts still need to be disabled@@@@

# save, restore registers to prevent leaking to user program. in SAVE_ALL, also
# prepare arguments on stack for function call
    SAVE_ALL
# reference data as kernel data
    movl $(KERNEL_DS), %ebx
    movl %ebx, %ds 
#check requested system call number
    cmpl    $10, %eax
    jb      call_syscall  
# return failure if greater than or equal to number of system calls
    movl    $-1, %eax
    jmp     ret_to_user

call_syscall:
    call *syscall_table(,%eax,4)

ret_to_user:
    RESTORE_ALL
    iret
