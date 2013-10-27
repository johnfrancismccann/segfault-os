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
#include "idt_functions.h"

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
    uint8_t idt_num = RTC_IDT_NUM;
	uint8_t  prev_b_val;
    //Block interrupts and save flags
	cli_and_save(flags);
	//Mask RTC interrupts during initialization.
    //Must mask RTC IRQ and possibly the chain IRQ
    //if RTC IRQ is 8:15.
	disable_irq(RTC_IRQ_NUM);
    disable_irq(PIC_CHAIN_IRQ);


    //select status register A of RTC
    outb(0x8A, RTC_PORT);
    //Set Frequency
    outb(0x21, RTC_PORT + 1);
    //select register B of RTC
    outb(0x8B, RTC_PORT);
    //save current value of register B of RTC
    prev_b_val = inb(RTC_PORT + 1);
    //re-select register B of RTC
    outb(0x8B, RTC_PORT);
    // Set bit 6 of register B on RTC to enable periodic interrupts
    outb(prev_b_val | 0x40, RTC_PORT + 1);
	// Set handler function; in this case, the assembly wrapper
	set_interrupt_gate(idt_num, rtc_wrapper);
	//Unmask RTC interrupts
	enable_irq(RTC_IRQ_NUM);
    enable_irq(PIC_CHAIN_IRQ);
    //Unnecessary setting of IRQ2 to rtc handler, just for my fleeting sanity.
    // set_interrupt_gate(34, rtc_wrapper);
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
    // clear();
    test_interrupts();
    //Clear inerrupt info from RTC to allow future interrupts.
    send_eoi(RTC_IRQ_NUM);
    // send_eoi(PIC_CHAIN_IRQ);
    //Register C on RTC must be read to re-enable interrupts
    //on IRQ_8.  See OS Dev Wiki.
    outb(0x0C, RTC_PORT);
    inb(RTC_PORT + 1);
}
