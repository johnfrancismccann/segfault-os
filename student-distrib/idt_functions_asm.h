/*
 *  The purpose of this file is to contain all IDT handler
 *  functions called from the IDT and set up in setup_idt.c.
 */

#ifndef _IDTFUNCTIONS_ASM_H
#define _IDTFUNCTIONS_ASM_H

extern void idt_div_error();
extern void idt_debug();
extern void idt_nmi();
extern void idt_breakpoint();
extern void idt_overflow();
extern void idt_bound();
extern void idt_invalid_op();
extern void idt_device_not_available();
extern void idt_double_fault();
extern void idt_coprocessor_segment_overrun();
extern void idt_invalid_TSS();
extern void idt_segment_not_present();
extern void idt_stack_segment();
extern void idt_general_protection();
extern void idt_page_fault();
extern void idt_coprocessor_error();
extern void idt_alignment_check();
extern void idt_system_call();

#endif
