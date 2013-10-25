/*
 *  The purpose of this file is to contain all IDT handler
 *  functions called from the IDT and set up in setup_idt.c.
 */

#ifndef IDTFUNCTIONS
#define IDTFUNCTIONS

#include "types.h"

void do_idt_div_error();//uint32_t error_code, uint32_t stack_address);
void do_idt_debug();//uint32_t error_code, uint32_t stack_address);
void do_idt_nmi();//uint32_t error_code, uint32_t stack_address);
void do_idt_breakpoint();//uint32_t error_code, uint32_t stack_address);
void do_idt_overflow();//uint32_t error_code, uint32_t stack_address);
void do_idt_bound();//uint32_t error_code, uint32_t stack_address);
void do_idt_invalid_op();//uint32_t error_code, uint32_t stack_address);
void do_idt_device_not_available();//uint32_t error_code, uint32_t stack_address);
void do_idt_double_fault();//uint32_t error_code, uint32_t stack_address);
void do_idt_coprocessor_segment_overrun();//uint32_t error_code, uint32_t stack_address);
void do_idt_invalid_TSS();//uint32_t error_code, uint32_t stack_address);
void do_idt_segment_not_present();//uint32_t error_code, uint32_t stack_address);
void do_idt_stack_segment();//uint32_t error_code, uint32_t stack_address);
void do_idt_general_protection();//uint32_t error_code, uint32_t stack_address);
void do_idt_page_fault();//uint32_t error_code, uint32_t stack_address);
void do_idt_coprocessor_error();//uint32_t error_code, uint32_t stack_address);
void do_idt_alignment_check();//uint32_t error_code, uint32_t stack_address);
void do_idt_system_call();//uint32_t error_code, uint32_t stack_address);

#endif
