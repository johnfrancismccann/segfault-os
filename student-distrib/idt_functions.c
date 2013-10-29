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
void do_idt_div_error()
{
	cli();
	// clear();
	printf("DIV BY 0 ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_debug()
{
	cli();
	// clear();
	printf("DEBUG ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_nmi()
{
	cli();
	// clear();
	printf("NMI ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_breakpoint()
{
	cli();
	// clear();
	printf("BREAKPOINT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_overflow()
{
	cli();
	// clear();
	printf("OVERFLOW ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_bound()
{
	cli();
	// clear();
	printf("BOUND ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_invalid_op()
{
	cli();
	// clear();
	printf("INVALID OPCODE ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_device_not_available()
{
	cli();
	// clear();
	printf("DEVICE NOT AVAILABLE ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_double_fault()
{
	cli();
	// clear();
	printf("DOUBLE FAULT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_coprocessor_segment_overrun()
{
	cli();
	// clear();
	printf("SEGMENT OVERRUN ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_invalid_TSS()
{
	cli();
	// clear();
	printf("INVALID TSS ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_segment_not_present()
{
	cli();
	// clear();
	printf("SEGMENT NOT PRESENT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_stack_segment()
{
	cli();
	// clear();
	printf("STACK SEGMENT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_general_protection()
{
	cli();
	// clear();
	printf("GENERAL PROTECTION ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_page_fault()
{
	cli();
	// clear();
	printf("PAGE FAULT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_coprocessor_error()
{
	cli();
	// clear();
	printf("COPROCESSOR ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_alignment_check()
{
	cli();
	// clear();
	printf("ALIGNMENT CHECK ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_system_call()
{
	cli();
	// clear();
	printf("SYSTEM CALL ERROR!\n");
	while(1);
}
