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
#define KBD_PORT    0x60

#define VGA_LOW 0x3D4 //low register of VGA port
#define VGA_HIGH 0x3D5 //high register of VGA port
#define CAP_OFFSET 0x20 //offset between lower case and upper case ASCII value
#define TAB_LEN 5 //set tab length to 5 spaces

/* ASCII values */
#define TAB_ASC 0x09 //tab key
#define ENT_ASC 0x0A //enter key

/* scancodes */
#define CTRL_PRS 0x1D //left/right control press
#define CTRL_RLS 0x9D //left/right control release
#define L_KEY 0x26 //'L' pressed
#define B_SPACE 0x0E //backspace pressed
#define TAB 0x0F //tab pressed
#define ENTER 0x1C //enter pressed
#define LSHIFT_PRS 0x2A //left shift pressed
#define RSHIFT_PRS 0x36 //right shift pressed
#define LSHIFT_RLS 0xAA //left shift released
#define RSHIFT_RLS 0xB6 //right shift released
#define CAPS 0x3A //caps lock pressed

#include "types.h"

void init_kbd();

void kbd_handle();

void update_cursor(int index);

void check_scroll(int print_index);

int32_t get_read_buf(void* buf);

void clear_read_buf();

#endif
