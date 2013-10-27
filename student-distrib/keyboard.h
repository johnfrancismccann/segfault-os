/*
 *  The keyboard files' purpose is to handle interrupts coming
 *  from the hardware RTC timer.
 *  keyboard_asm.S is an interrupt wrapper called by the IDT, which
 *  then executes the keyboard.c handler.  The wrapper is necessary
 *  to handle the returns and state-saving interrupt handling
 *  parts.
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

//Keyboard is IRQ 1, IDT number 33, port 0x60
#define KBD_IRQ_NUM 1
#define KBD_IDT_NUM 33
#define KBD_PORT 	0x60

void init_kbd();

void kbd_handle();

#endif
