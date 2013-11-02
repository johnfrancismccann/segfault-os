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

//Flag indicating an interrupt has occured
uint8_t interrupt_flag;

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
    //Set interrupt flag to 0 indicating no interrupts
    interrupt_flag = 0;
	//Mask RTC interrupts during initialization.
    //Must mask RTC IRQ and possibly the chain IRQ
    //if RTC IRQ is 8:15.
	disable_irq(RTC_IRQ_NUM);
    disable_irq(PIC_CHAIN_IRQ);


    //select status register A of RTC
    outb(0x8A, RTC_PORT);
    //Set Frequency
    outb(0x2F, RTC_PORT + 1);
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
    // test_interrupts();
    //Set flag to indicate a new interrupt
    interrupt_flag |= 1;
    send_eoi(RTC_IRQ_NUM);
    //Register C on RTC must be read to re-enable interrupts
    //on IRQ_8.  See OS Dev Wiki.
    outb(0x0C, RTC_PORT);
    inb(RTC_PORT + 1);
}


/*
 * rtc_open()
 *   DESCRIPTION:
 *   INPUTS: 
 *   OUTPUTS: 
 *   RETURN VALUE: 
 *   SIDE EFFECTS: 
 */
int rtc_open()
{
    return 0;
}


/*
 * rtc_read()
 *   DESCRIPTION:  Returns 0 after interrupt from RTC.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: 0 after interrupt from RTC 
 *   SIDE EFFECTS: None
 */
int rtc_read()
{
    //reset interrupt_flag to wait for interrupt
    interrupt_flag &= 0;
    //wait for interrupt
    while(!interrupt_flag);
    //return after interrupt
    return 0;
}


/*
 * rtc_write()
 *   DESCRIPTION:
 *   INPUTS: 
 *   OUTPUTS: 
 *   RETURN VALUE: 
 *   SIDE EFFECTS: 
 */
int rtc_write(uint32_t freq)
{
    uint8_t rate = 0;
    uint32_t flags;
    //Don't set if outside valid frequency range
    if(freq > MAXFREQ || freq < MINFREQ)
        return -1;
    //Only set for valid powers of 2
    else if((freq - 1) & freq)
        return -1;
    else
    {
        /* Find log_2 of frequency to feed to the RTC. */
        while(freq >>= 1)
            rate ++;
        /* There is no 2^1 available, so compensate by making
         * 2^2 the first value */
        rate--;
        /* rate initially has log_2(frequency), but RTC
         * expects a shift amount.  This means a small log_2
         * value cooresponds to a large shift */
        rate = 0xF - rate;

        //block interrupts during frequency set
        cli_and_save(flags);
        //Initiate writing to RTC register A
        outb(0x8A, RTC_PORT);
        //Write new frequency to RTC
        outb(0x20 | rate, RTC_PORT + 1);
        //re-enable interrupts if they were enabled before
        restore_flags(flags);
        return 0;
    }
}


/*
 * rtc_close()
 *   DESCRIPTION:
 *   INPUTS: 
 *   OUTPUTS: 
 *   RETURN VALUE: 
 *   SIDE EFFECTS: 
 */
int rtc_close()
{
    return 0;
}
