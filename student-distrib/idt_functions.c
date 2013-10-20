/*
 *  The purpose of this file is to contain all IDT handler
 *  functions called from the IDT and set up in setup_idt.c.
 */

/****THESE ARE BROKEN!  NEED ASM *AND* C HANDLERS AS PER:
 ****http://www.phrack.org/issues.html?issue=59&id=4&mode=txt
 ****Understanding the Linux Kernel §4.5
 */

#include "lib.h"
#include "idt_functions.h"

/*
 *
 */
void idt_div_error()
{
	clear();
	printf("DIV BY 0 ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_debug()
{
	clear();
	printf("DEBUG ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_nmi()
{
	clear();
	printf("NMI ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_breakpoint()
{
	clear();
	printf("BREAKPOINT ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_overflow()
{
	clear();
	printf("OVERFLOW ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_bound()
{
	clear();
	printf("BOUND ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_invalid_op()
{
	clear();
	printf("INVALID OPCODE ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_device_not_available()
{
	clear();
	printf("DEVICE NOT AVAILABLE ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_double_fault()
{
	clear();
	printf("DOUBLE FAULT ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_coprocessor_segment_overrun()
{
	clear();
	printf("SEGMENT OVERRUN ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_invalid_TSS()
{
	clear();
	printf("INVALID TSS ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_segment_not_present()
{
	clear();
	printf("SEGMENT NOT PRESENT ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_stack_segment()
{
	clear();
	printf("STACK SEGMENT ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_general_protection()
{
	clear();
	printf("GENERAL PROTECTION ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_page_fault()
{
	clear();
	printf("PAGE FAULT ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_coprocessor_error()
{
	clear();
	printf("COPROCESSOR ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_alignment_check()
{
	clear();
	printf("ALIGNMENT CHECK ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}


/*
 *
 */
void idt_system_call()
{
	clear();
	printf("SYSTEM CALL ERROR! \n");
	asm volatile("IRET"                 \
				:                       \
				:                       \
				: "memory", "cc"        \
				);                      \
}
