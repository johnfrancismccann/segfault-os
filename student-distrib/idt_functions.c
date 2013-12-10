/*
 *  The purpose of this file is to contain all IDT handler
 *  functions called from the IDT and set up in setup_idt.c.
 */


#include "lib.h"
#include "idt_functions.h"
#include "types.h"
#include "test_syscalls.h"
//#include "syscalls.h"

/*
 *  do_idt_div_error
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_div_error(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("DIV BY 0 ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_debug
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_debug(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("DEBUG ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_nmi
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_nmi(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("NMI ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_breakpoint
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_breakpoint(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("BREAKPOINT ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_overflow
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_overflow(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("OVERFLOW ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_bound
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_bound(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("BOUND ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_invalid_op
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_invalid_op(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("INVALID OPCODE ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_device_not_available
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_device_not_available(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("DEVICE NOT AVAILABLE ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_double_fault
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_double_fault(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("DOUBLE FAULT ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_coprocessor_segment_overrun
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_coprocessor_segment_overrun(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("SEGMENT OVERRUN ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_invalid_TSS
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_invalid_TSS(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("INVALID TSS ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_segment_not_present
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_segment_not_present(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("SEGMENT NOT PRESENT ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_stack_segment
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_stack_segment(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("STACK SEGMENT ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_general_protection
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_general_protection(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("GENERAL PROTECTION ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_page_fault
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_page_fault(uint32_t error_code_a, uint32_t error_code_b)
{
    cli();
    clear();
    reset_screen_pos();
    printf("PAGE FAULT ERROR!\nERROR CODE 1: %x\nERROR CODE 2: %x\n", error_code_b, error_code_a);
    while(1);
    test_halt(255);
}


/*
 *  do_idt_coprocessor_error
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_coprocessor_error(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("COPROCESSOR ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_alignment_check
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_alignment_check(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("ALIGNMENT CHECK ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


/*
 *  do_idt_system_call
 *  DESCRIPTION:
 *  INPUTS: error_code--the error code sent to the interrupt
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the interrupt is entered
 */
void do_idt_system_call(uint32_t error_code)
{
    cli();
    clear();
    reset_screen_pos();
    printf("SYSTEM CALL ERROR!\nERROR CODE: %x\n", error_code);
    test_halt(255);
}


