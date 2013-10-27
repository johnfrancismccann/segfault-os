/*
 *  The rtc.* files' purpose is to handle interrupts coming
 *  from the hardware RTC timer.
 *  rtc.S is an interrupt wrapper called by the IDT, which
 *  then executes the rtc.c handler.  The wrapper is necessary
 *  to handle the returns and state-saving interrupt handling
 *  parts.
 */

#include "rtc.h"
#include "rtc_asm.h"
#include "setup_idt.h"
#include "i8259.h"
#include "lib.h"

/*
 * init_rtc()
 *   DESCRIPTION: Registers the rtc interrupt handler in the IDT
 *				  and enables the hardware interrupt on the PIC.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Updates IDT entry for RTC and modifies mask vector
 *				   on PIC.
 */
void init_rtc()
{
	uint32_t flags;
	uint8_t  prev_b_val;
    //Block interrupts and save flags
	cli_and_save(flags);
	//Mask RTC interrupts during initialization.
    //Must mask RTC IRQ and possibly the chain IRQ
    //if RTC IRQ is 8:15.
	disable_irq(RTC_IRQ_NUM);
    disable_irq(PIC_CHAIN_IRQ);
    //select status register A of RTC
    outb(RTC_PORT, 0x8A);
    //disable NMI
    outb(RTC_PORT + 1, 0x20);
    //select register B of RTC
    outb(RTC_PORT, 0x8B);
    //save current value of register B of RTC
    prev_b_val = inb(RTC_PORT + 1);
    //re-select register B of RTC
    outb(RTC_PORT, 0x8B);
    //Set bit 6 of register B on RTC to enable periodic interrupts
    outb(RTC_PORT + 1, prev_b_val | 0x40);
	//Set handler function; in this case, the assembly wrapper
	set_interrupt_gate(RTC_IDT_NUM, rtc_wrapper);
	//Unmask RTC interrupts
	enable_irq(RTC_IRQ_NUM);
    //Unmask PIC chain IRQ if chain needs enabled
    #if PIC_CHAIN_ENABLE
    enable_irq(PIC_CHAIN_IRQ);
    #endif
	//Restore flags
	restore_flags(flags);
}


/*
 * rtc_idt_handle()
 *   DESCRIPTION: C handler for RTC interrupts.  Currently only runs
 *                test_interrupts() from lib.c.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sends EOI to PIC.
 */
void rtc_idt_handle()
{
    test_interrupts();
    send_eoi(RTC_IRQ_NUM);
}
