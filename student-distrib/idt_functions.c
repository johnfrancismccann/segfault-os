/*
 *  The purpose of this file is to contain all IDT handler
 *  functions called from the IDT and set up in setup_idt.c.
 */


#include "lib.h"
#include "idt_functions.h"
#include "types.h"

/*
 *
 */
void do_idt_div_error(uint32_t error_code)
{
	cli();
	clear();
	printf("DIV BY 0 ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_debug(uint32_t error_code)
{
	cli();
	clear();
	printf("DEBUG ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_nmi(uint32_t error_code)
{
	cli();
	clear();
	printf("NMI ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_breakpoint(uint32_t error_code)
{
	cli();
	clear();
	printf("BREAKPOINT ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_overflow(uint32_t error_code)
{
	cli();
	clear();
	printf("OVERFLOW ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_bound(uint32_t error_code)
{
	cli();
	clear();
	printf("BOUND ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_invalid_op(uint32_t error_code)
{
	cli();
	clear();
	printf("INVALID OPCODE ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_device_not_available(uint32_t error_code)
{
	cli();
	clear();
	printf("DEVICE NOT AVAILABLE ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_double_fault(uint32_t error_code)
{
	cli();
	clear();
	printf("DOUBLE FAULT ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_coprocessor_segment_overrun(uint32_t error_code)
{
	cli();
	clear();
	printf("SEGMENT OVERRUN ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_invalid_TSS(uint32_t error_code)
{
	cli();
	clear();
	printf("INVALID TSS ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_segment_not_present(uint32_t error_code)
{
	cli();
	clear();
	printf("SEGMENT NOT PRESENT ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_stack_segment(uint32_t error_code)
{
	cli();
	clear();
	printf("STACK SEGMENT ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_general_protection(uint32_t error_code)
{
	cli();
	clear();
	printf("GENERAL PROTECTION ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_page_fault(uint32_t error_code_a, uint32_t error_code_b)
{
	cli();
	clear();
	printf("PAGE FAULT ERROR!\nERROR CODE 1: %x\nERROR CODE 2: %x\n", error_code_b, error_code_a);
	while(1);
}


/*
 *
 */
void do_idt_coprocessor_error(uint32_t error_code)
{
	cli();
	clear();
	printf("COPROCESSOR ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_alignment_check(uint32_t error_code)
{
	cli();
	clear();
	printf("ALIGNMENT CHECK ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}


/*
 *
 */
void do_idt_system_call(uint32_t error_code)
{
	cli();
	clear();
	printf("SYSTEM CALL ERROR!\nERROR CODE: %x\n", error_code);
	while(1);
}
