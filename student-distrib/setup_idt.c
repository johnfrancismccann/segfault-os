/* 
 *	The purpose of this file is to set up the initial IDT
 *	created allocated in x86_desc.S.  The basic layout is as
 *	follows:
 *
 *  IDT Layout
 *  Source: Table 5-1, IA-32 Reference Manual Vol. 3
 *  0:  Divide Error                Fault       TRAP
 *  1:  RESERVED                    Fault/Trap  TRAP
 *  2:  NMI Interrupt               Interrupt   TRAP
 *  3:  Breakpoint                  Trap        SYSTEM
 *  4:  Overflow                    Trap        SYSTEM
 *  5:  BOUND Range Exceeded        Fault       SYSTEM
 *  6:  Invalid Opcode              Fault       TRAP
 *  7:  Device Not Available        Fault       TRAP
 *  8:  Double Fault                Abort       TRAP
 *  9:  Coprocessor Segment Overrun Fault       TRAP
 *  10: Invalid TSS                 Fault       TRAP
 *  11: Segment Not Present         Fault       TRAP
 *  12: Stack-Segment Fault         Fault       TRAP
 *  13: General Protection          Fault       TRAP
 *  14: Page Fault                  Fault       TRAP
 *  15: RESERVED
 *  16: x87 FPU Error (Math Fault)  Fault       TRAP
 *  17: Alignment Check             Fault       TRAP
 *  18: Machine Check               Abort       TRAP
 *  19: SIMD Float Exception        Fault       TRAP
 *  20:
 *  ... RESERVED
 *  31:
 *  32:
 *  ... USER DEFINED                Interrupt
 *  ... IRQ 0x00 - 0x0F in [32:47]
 *  255:
 */

#include "x86_desc.h"
#include "setup_idt.h"
#include "idt_functions.h"
#include "lib.h"
//#include "idt_stubs.S"

/*
 * write_to_idt
 *   DESCRIPTION: Writes one entry to the IDT.
 *   INPUTS: entry_num: The entry number in the IDT.  Range 0..255
 *           data:      The QUAD size piece of data to write to IDT.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes IDT entry first created in x86_desc.S.
 */
void write_to_idt(uint8_t entry_num, uint8_t* data)
{
    //Find beginning of IDT entry to write to.
    //Each entry is one QUAD (8 bytes).
    uint8_t* start_addr = (uint8_t*)idt + (QUAD_SIZE * entry_num);
    //Copy to IDT
    memcpy(start_addr, data, QUAD_SIZE);
}

/*
 * set_trap_gate
 *   DESCRIPTION: Sets up entry for trap gate IDT entry.
 *   INPUTS: entry_num: The entry number in the IDT.  Range 0..255
 *           function:  Address of handler function
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes IDT entry first created in x86_desc.S.
 */
void set_trap_gate(uint8_t entry_num, void* function)
{
    //See Fig 5-2 from IA32 Reference Manual Vol. 3
    uint8_t outdata[QUAD_SIZE];
    //Handler addr [31:16]
    outdata[7] = (((uint32_t)function & 0xFF000000) >> 24);
    outdata[6] = (((uint32_t)function & 0x00FF0000) >> 16);
    //P=0b1, DPL=0b00 (kernel), D=0b1 (32-bit)
    outdata[5] = 0x8F; //0b10001111
    //Reserved (0x0)
    outdata[4] = 0x0;
    //Segment = kernel code
    outdata[3] = ((KERNEL_CS >> 8) & 0xFF);
    outdata[2] = KERNEL_CS & 0xFF;
    //Handler addr [15:0]
    outdata[1] = (((uint32_t)function & 0x0000FF00) >> 8);
    outdata[0] = (uint32_t)function & 0xFF;

    //Write to IDT
    write_to_idt(entry_num, outdata);
}

/*
 * set_system_gate
 *   DESCRIPTION: Sets up entry for system gate IDT entry.
 *                *NOTE*: Same as Trap Gate, but DPL is for user, 
 *                        not kernel.
 *   INPUTS: entry_num: The entry number in the IDT.  Range 0..255
 *           function:  Address of handler function
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes IDT entry first created in x86_desc.S.
 */
void set_system_gate(uint8_t entry_num, void* function)
{
    //See Fig 5-2 from IA32 Reference Manual Vol. 3
    uint8_t outdata[QUAD_SIZE];
    //Handler addr [31:16]
    outdata[7] = (((uint32_t)function & 0xFF000000) >> 24);
    outdata[6] = (((uint32_t)function & 0x00FF0000) >> 16);
    //P=0b1, DPL=0b11 (user), D=0b1 (32-bit)
    outdata[5] = 0xEF; //0b11101111
    //Reserved (0x0)
    outdata[4] = 0x0;
    //Segment = kernel code
    outdata[3] = ((USER_CS >> 8) & 0xFF);
    outdata[2] = USER_CS & 0xFF;
    //Handler addr [15:0]
    outdata[1] = (((uint32_t)function & 0x0000FF00) >> 8);
    outdata[0] = (uint32_t)function & 0xFF;

    //Write to IDT
    write_to_idt(entry_num, outdata);
}

/*
 * set_interrupt_gate
 *   DESCRIPTION: Sets up entry for interrupt gate IDT Entry.
 *                This is accessible externally for interrupt
 *                setup.
 *   INPUTS: entry_num: The entry number in the IDT.  Range 0..255
 *           function:  Address of handler function
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes IDT entry first created in x86_desc.S.
 */
void set_interrupt_gate(uint8_t entry_num, void* function)
{
    //See Fig 5-2 from IA32 Reference Manual Vol. 3
    uint8_t outdata[QUAD_SIZE];
    //Handler addr [31:16]
    outdata[7] = (((uint32_t)function & 0xFF000000) >> 24);
    outdata[6] = (((uint32_t)function & 0x00FF0000) >> 16);
    //P=0b1, DPL=0b00 (user), D=0b1 (32-bit)
    outdata[5] = 0x8E; //0b10001110
    //Reserved (0x0)
    outdata[4] = 0x0;
    //Segment = kernel code
    outdata[3] = ((USER_CS >> 8) & 0xFF);
    outdata[2] = USER_CS & 0xFF;
    //Handler addr [15:0]
    outdata[1] = (((uint32_t)function & 0x0000FF00) >> 8);
    outdata[0] = (uint32_t)function & 0xFF;

    //Write to IDT
    write_to_idt(entry_num, outdata);
}


/*
 * init_idt
 *   DESCRIPTION: Sets up all basic IDT entries
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes IDT entries first created in x86_desc.S.
 */
void init_idt()
{
    set_trap_gate(0, do_idt_div_error);
    set_trap_gate(1, do_idt_debug);
    set_trap_gate(2, do_idt_nmi);
    set_system_gate(3, do_idt_breakpoint);
    set_system_gate(4, do_idt_overflow);
    set_system_gate(5, do_idt_bound);
    set_trap_gate(6, do_idt_invalid_op);
    set_trap_gate(7, do_idt_device_not_available);
    set_trap_gate(8, do_idt_double_fault);
    set_trap_gate(9, do_idt_coprocessor_segment_overrun);
    set_trap_gate(10, do_idt_invalid_TSS);
    set_trap_gate(11, do_idt_segment_not_present);
    set_trap_gate(12, do_idt_stack_segment);
    set_trap_gate(13, do_idt_general_protection);
    set_trap_gate(14, do_idt_page_fault);
    set_trap_gate(16, do_idt_coprocessor_error);
    set_trap_gate(17, do_idt_alignment_check);
    set_system_gate(0x80, do_idt_system_call);
}
