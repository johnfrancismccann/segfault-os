/* 
 *	The purpose of this file is to set up the initial IDT
 *	created allocated in x86_desc.S.  The basic layout is as
 *	follows:
 *
 *  IDT Layout
 *  Source: Table 5-1, IA-32 Reference Manual Vol. 3
 *  0:  Divide Error                Fault
 *  1:  RESERVED                    Fault/Trap
 *  2:  NMI Interrupt               Interrupt
 *  3:  Breakpoint                  Trap
 *  4:  Overflow                    Trap
 *  5:  BOUND Range Exceeded        Fault
 *  6:  Invalid Opcode              Fault
 *  7:  Device Not Available        Fault
 *  8:  Double Fault                Abort
 *  9:  Coprocessor Segment Overrun Fault
 *  10: Invalid TSS                 Fault
 *  11: Segment Not Present         Fault
 *  12: Stack-Segment Fault         Fault
 *  13: General Protection          Fault
 *  14: Page Fault                  Fault
 *  15: RESERVED
 *  16: x87 FPU Error (Math Fault)  Fault
 *  17: Alignment Check             Fault
 *  18: Machine Check               Abort
 *  19: SIMD Float Exception        Fault
 *  20:
 *  ... RESERVED
 *  31:
 *  32:
 *  ... USER DEFINED                Interrupt
 *  255:
 */

#define QUAD_SIZE 8

//only externally accessable function to setup main exceptions
void init_idt();
