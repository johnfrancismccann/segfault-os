/*
 *  The rtc.* files' purpose is to handle interrupts coming
 *  from the hardware RTC timer.
 *  rtc.S is an interrupt wrapper called by the IDT, which
 *  then executes the rtc.c handler.  The wrapper is necessary
 *  to handle the returns and state-saving interrupt handling
 *  parts.
 */

#ifndef _RTC_H
#define _RTC_H

//The RTC is IRQ_8.  In IDT, IRQs are entries 32:47
#define RTC_IRQ_NUM 0x8
#define RTC_IDT_NUM 0x28

#define PIC_CHAIN_IRQ 0x2

#define RTC_PORT 0x70

void init_rtc();

void rtc_idt_handle();

#endif
