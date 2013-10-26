/*
 *  The purpose of this file is to contain all IDT handler
 *  functions called from the IDT and set up in setup_idt.c.
 */

#ifndef IDTFUNCTIONS
#define IDTFUNCTIONS

#include "types.h"

void do_idt_div_error();
void do_idt_debug();
void do_idt_nmi();
void do_idt_breakpoint();
void do_idt_overflow();
void do_idt_bound();
void do_idt_invalid_op();
void do_idt_device_not_available();
void do_idt_double_fault();
void do_idt_coprocessor_segment_overrun();
void do_idt_invalid_TSS();
void do_idt_segment_not_present();
void do_idt_stack_segment();
void do_idt_general_protection();
void do_idt_page_fault();
void do_idt_coprocessor_error();
void do_idt_alignment_check();
void do_idt_system_call();

#endif
