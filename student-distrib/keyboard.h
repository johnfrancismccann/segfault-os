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

#define COLORS_ENABLED 1 //enable colors

//text color codes for text-mode VGA attribute byte (LT == light, DK == dark)
#define BLACK       0
#define BLUE        1
#define GREEN       2
#define CYAN        3
#define RED         4
#define MAGENTA     5
#define BROWN       6
#define LT_GREY     7
#define DK_GREY     8
#define LT_BLUE     9
#define LT_GREEN    10
#define LT_CYAN     11
#define LT_RED      12
#define LT_MAGENTA  13
#define LT_BROWN    14
#define WHITE       15

#ifndef OFF
#define OFF 0
#endif

#ifndef ON
#define ON 1
#endif

//Keyboard is IRQ 1, IDT number 33, port 0x60
#define KBD_IRQ_NUM 1
#define KBD_IDT_NUM 33
#define KBD_PORT    0x60

#define VGA_LOW 0x3D4 //low register of VGA port
#define VGA_HIGH 0x3D5 //high register of VGA port
#define CAP_OFFSET 0x20 //offset between lower case and upper case ASCII value
#define TAB_LEN 5 //set tab length to 5 spaces
#define UP          0 //for upper scroll store (previous data)
#define DN          1 //for lower scroll store (current data)

/* ASCII values */
#define TAB_ASC 0x09 //tab key
#define ENT_ASC 0x0A //enter key
#define ONE_ASC     0x02 //#1
#define ZERO_ASC    0x0B //#0
/* Arrow Keys; NOTE QEMU is stupid, so the arrow keys show up
 * as number pad keys.
 *   NUM       KK0DE
 *    8         48
 *  4   6     4B  4D
 *    2         50
 */
#define K_UP    0x48
#define K_DOWN  0x50
#define K_LEFT  0x4B
#define K_RIGHT 0x4D


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
#define ALT_PRS     0x38
#define ALT_RLS     0xB8
#define PG_UP       0x49 //page up press
#define PG_DN       0x51 //page down press
/* ALT_FX found on stanislavs.org/helppc/scan_codes.html */
#define F1     0x3B 
#define F2     0x3C
#define F3     0x3D


//for changing color scheme
#define NUM_SCANCODE_OFFSET 1 //1 has scancode 0x02, 2 is 0x03, etc.

//status bar macros
#define STATUS_BAR_START   NUM_ROWS*NUM_COLS
#define STATUS_BAR_END     (NUM_ROWS+1)*NUM_COLS-1 //also final terminal location
#define TERM_INFO_START    NUM_ROWS*NUM_COLS //starting print index of terminal number information
#define PROC_INFO_START    NUM_ROWS*NUM_COLS+18 //starting print index of process information
#define BG_CLR_INFO_START  NUM_ROWS*NUM_COLS+37 //starting print index of background color information
#define TXT_CLR_INFO_START NUM_ROWS*NUM_COLS+62 //starting print index of text color information


#define KBD_MAP_SIZE 256
#define BUF_SIZE 128

#define NUM_TERMS 3

#define CMD_HIST_LEN 8

#include "types.h"

void init_kbd();

void kbd_handle();

uint32_t get_act_ops_disp();

//void check_scroll(int print_index);

void set_act_ops_term(uint32_t term_index);

int32_t get_read_buf(void* buf, int32_t bytes);

void clear_read_buf();

//int32_t get_act_ops_disp(uint8_t** act_ops_display);

int32_t print_write_buf(const void* wrt_buf, int32_t bytes);

void reset_print_inds();

void remove_hw_cursor();

#if COLORS_ENABLED
char* strcat(char *dest, char *src); //standard string concatenation

void print_status_bar(); //need to print status bar upon creation of new process

void reset_print_inds();
#endif

#endif
