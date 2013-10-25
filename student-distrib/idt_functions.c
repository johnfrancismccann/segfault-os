/*
 *  The purpose of this file is to contain all IDT handler
 *  functions called from the IDT and set up in setup_idt.c.
 */


#include "lib.h"
#include "idt_functions.h"

/*
 *
 */
void do_idt_div_error()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("DIV BY 0 ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_debug()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("DEBUG ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_nmi()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("NMI ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_breakpoint()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("BREAKPOINT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_overflow()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("OVERFLOW ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_bound()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("BOUND ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_invalid_op()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("INVALID OPCODE ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_device_not_available()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("DEVICE NOT AVAILABLE ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_double_fault()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("DOUBLE FAULT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_coprocessor_segment_overrun()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("SEGMENT OVERRUN ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_invalid_TSS()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("INVALID TSS ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_segment_not_present()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("SEGMENT NOT PRESENT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_stack_segment()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("STACK SEGMENT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_general_protection()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("GENERAL PROTECTION ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_page_fault()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("PAGE FAULT ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_coprocessor_error()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("COPROCESSOR ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_alignment_check()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("ALIGNMENT CHECK ERROR!\n");
	while(1);
}


/*
 *
 */
void do_idt_system_call()//uint32_t error_code, uint32_t stack_address)
{
	clear();
	printf("SYSTEM CALL ERROR!\n");
	while(1);
}
