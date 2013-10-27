/*
 *  The keyboard files' purpose is to handle interrupts coming
 *  from the hardware RTC timer.
 *  keyboard_asm.S is an interrupt wrapper called by the IDT, which
 *  then executes the keyboard.c handler.  The wrapper is necessary
 *  to handle the returns and state-saving interrupt handling
 *  parts.
 */

#ifndef _KEYBOARD_ASM_H
#define _KEYBOARD_ASM_H

extern void kbd_wrapper();

#endif
