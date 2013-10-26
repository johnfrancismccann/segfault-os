/*
 *  The purpose of this file is to contain all IDT handler
 *  functions called from the IDT and set up in setup_idt.c.
 */


#include "lib.h"
#include "idt_functions.h"

/*
 *
 */
void do_idt_div_error()
{
	clear();
	printf("DIV BY 0 ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_debug()
{
	clear();
	printf("DEBUG ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_nmi()
{
	clear();
	printf("NMI ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_breakpoint()
{
	clear();
	printf("BREAKPOINT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_overflow()
{
	clear();
	printf("OVERFLOW ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_bound()
{
	clear();
	printf("BOUND ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_invalid_op()
{
	clear();
	printf("INVALID OPCODE ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_device_not_available()
{
	clear();
	printf("DEVICE NOT AVAILABLE ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_double_fault()
{
	clear();
	printf("DOUBLE FAULT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_coprocessor_segment_overrun()
{
	clear();
	printf("SEGMENT OVERRUN ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_invalid_TSS()
{
	clear();
	printf("INVALID TSS ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_segment_not_present()
{
	clear();
	printf("SEGMENT NOT PRESENT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_stack_segment()
{
	clear();
	printf("STACK SEGMENT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_general_protection()
{
	clear();
	printf("GENERAL PROTECTION ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_page_fault()
{
	clear();
	printf("PAGE FAULT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_coprocessor_error()
{
	clear();
	printf("COPROCESSOR ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_alignment_check()
{
	clear();
	printf("ALIGNMENT CHECK ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_system_call()
{
	clear();
	printf("SYSTEM CALL ERROR!\n");
	while(1);
}
