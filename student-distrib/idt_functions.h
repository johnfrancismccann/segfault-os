/*
 *  The purpose of this file is to contain all IDT handler
 *  functions called from the IDT and set up in setup_idt.c.
 */

void idt_div_error();
void idt_debug();
void idt_nmi();
void idt_breakpoint();
void idt_overflow();
void idt_bound();
void idt_invalid_op();
void idt_device_not_available();
void idt_double_fault();
void idt_coprocessor_segment_overrun();
void idt_invalid_TSS();
void idt_segment_not_present();
void idt_stack_segment();
void idt_general_protection();
void idt_page_fault();
void idt_coprocessor_error();
void idt_alignment_check();
void idt_system_call();
