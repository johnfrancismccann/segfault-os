/*
 *  The keyboard files' purpose is to handle interrupts coming
 *  from the hardware RTC timer.
 *  keyboard_asm.S is an interrupt wrapper called by the IDT, which
 *  then executes the keyboard.c handler.  The wrapper is necessary
 *  to handle the returns and state-saving interrupt handling
 *  parts.
 */

#include "keyboard.h"
#include "keyboard_asm.h"
#include "setup_idt.h"
#include "i8259.h"
#include "lib.h"
#include "types.h"

static const char KBD_MAP[KBD_MAP_SIZE] =
{0x00, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x2D, 0X3D, 0x08, 0x09,  //0x0F
 0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69, 0x6F, 0x70, 0x5B, 0x5D, 0x0A, 0x00, 0x61, 0x73,  //0x1F 0x00 is left control
 0x64, 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6C, 0x3B, 0x27, 0x60, 0x00, 0x5C, 0x7A, 0x78, 0x63, 0x76,  //0x2F 0x00 is left shift
 0x62, 0x6E, 0x6D, 0x2C, 0x2E, 0x2F, 0x00, 0x2A, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x3F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x38, 0x39, 0x2D, 0x64, 0x65, 0x66, 0x2B, 0x61,  //0x4F
 0x62, 0x63, 0x60, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x5F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x6F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x7F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x8F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x9F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0xAF
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0xBF
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0xCF
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0xDF
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0xEF
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //0xFF

static const char KBD_MAP_SHIFT[KBD_MAP_SIZE] =
{0x00, 0x1B, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5E, 0x26, 0x2A, 0x28, 0x29, 0x5F, 0X2B, 0x08, 0x09,  //0x0F
 0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69, 0x6F, 0x70, 0x7B, 0x7D, 0x0A, 0x00, 0x61, 0x73,  //0x1F 0x00 is left control
 0x64, 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6C, 0x3A, 0x22, 0x7E, 0x00, 0x7C, 0x7A, 0x78, 0x63, 0x76,  //0x2F 0x00 is left shift
 0x62, 0x6E, 0x6D, 0x3C, 0x3E, 0x3F, 0x00, 0x2A, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x3F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x38, 0x39, 0x2D, 0x64, 0x65, 0x66, 0x2B, 0x61,  //0x4F
 0x62, 0x63, 0x60, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x5F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x6F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x7F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x8F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x9F
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0xAF
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0xBF
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0xCF
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0xDF
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0xEF
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //0xFF

static char read_buf[BUF_SIZE]; //buffer to store characters typed in from user
static uint8_t scancode; //make this with file scope so can check for enter in get_read_buf (for term_read)
static char* video_mem = (char *)VIDEO;
static uint16_t buf_idx; //index for storing characters in buffer
static uint16_t print_idx; //index for printing characters to screen (video memory offset)
static int ctrl_flag; //flag to indicate if control key currently pressed or not (1 is yes, 0 is no)
static int shift_flag; //indicates if shift is pressed or not
static int caps_lock; //indicates if caps lock is enabled/disabled
static int prev_loc[NUM_ROWS]; //gives location before hitting enter
static int prev_idx; //index of previous location

/*
 * init_kbd()
 *   DESCRIPTION: Allows keyboard presses to interrupt kernel
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables keyboard interrupts
 */
void init_kbd()
{
    buf_idx = 0;
    read_buf[buf_idx] = '\0';
    print_idx = 0;
    prev_idx = 0;
    ctrl_flag = OFF; //initialize to control unpressed
    shift_flag = OFF; //init to shift unpressed
    caps_lock = OFF; //init to caps off

    update_cursor(0); //initialize cursor to top left corner

    printf("I LOVE ");

    set_interrupt_gate(KBD_IDT_NUM, kbd_wrapper);
    enable_irq(KBD_IRQ_NUM);
}

/*
 * kbd_handle()
 *   DESCRIPTION: handles interrupts from keyboard presses
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Prints characters to screen from keyboard.
 */
void kbd_handle()
{
    scancode = inb(KBD_PORT); //get key press
    int16_t i;

    if(print_idx == 0) {
        clear();
    }

    switch(scancode) {
        case CTRL_PRS:
            ctrl_flag = ON; //set control flag
            break;
        case CTRL_RLS:
            ctrl_flag = OFF; //clear control flag
            break;
        case LSHIFT_PRS:
        case RSHIFT_PRS:
            shift_flag = ON; //set shift flag
            break;
        case LSHIFT_RLS:
        case RSHIFT_RLS:
            shift_flag = OFF; //clear shift flag
            break;
        case CAPS:
            caps_lock ^= 1; //invert value of caps lock
            break;
        case B_SPACE:
            if(print_idx == 0) 
            { //can't backspace if at first location of video memory and buffer empty
                if(buf_idx == 0)
                {   
                    send_eoi(KBD_IRQ_NUM);
                    return;
                }
                //Scroll up to reveal previous command
                else
                {
                    //Remove last entry (a new line '\n') and set to endline
                    read_buf[--buf_idx] = '\0';
                    int num_to_print = 0;
                    int char_to_print = 0;

                    //find how many characters make up the next block of characters
                    while((read_buf[buf_idx - num_to_print] != ENT_ASC) && ((buf_idx - num_to_print) > 0) && (char_to_print < (NUM_COLS*NUM_ROWS)))
                    {
                        num_to_print++;
                        if (read_buf[buf_idx - num_to_print] == TAB_ASC) {
                            char_to_print += TAB_LEN; //increase number of chars to print by tab length
                        }
                        else {
                            char_to_print++; //increment for regular characters
                        }
                    }
                    //write new line(s) at top of screen
                    print_idx = 0;
                    //copy all characters for created line(s)
                    for(i = 0; i < num_to_print; i++)                               
                    {
                        switch(read_buf[buf_idx - num_to_print + i]) {
                            case TAB_ASC:
                                print_idx += TAB_LEN; //add 5 spaces/tab
                                break;
                            case ENT_ASC:
                                break;
                            default: //for regular characters (only increment print index)
                                *(uint8_t *)(video_mem + (print_idx << 1)) = read_buf[buf_idx - num_to_print + i]; // "<< 1" because each character is 2 bytes
                                print_idx++;
                        }
                    }
                    update_cursor(print_idx);
                    send_eoi(KBD_IRQ_NUM);
                    return;
                }
            }
            if(buf_idx > 0) { //make sure not accessing empty buffer, decrement buf_idx since deleted char
                switch(read_buf[--buf_idx]) {
                    case TAB_ASC: //for backspacing after tab
                        print_idx -= TAB_LEN;
                        break;
                    case ENT_ASC: //for backspacing after new line
                        print_idx = prev_loc[--prev_idx];
                        prev_loc[prev_idx] = 0;
                        break;
                    default: //for backspacing regular characters
                        *(uint8_t *)(video_mem + (--print_idx << 1)) = ' '; //delete character and move print index back 1 char
                }
                read_buf[buf_idx] = '\0'; //remove character from buffer
            }
            update_cursor(print_idx);
            send_eoi(KBD_IRQ_NUM);
            return;
    }

    /* clear screen */
    if (ctrl_flag == ON && scancode == L_KEY) { //ctrl+L
        clear();
        clear_read_buf();
        print_idx = 0; //reset print location to top left corner
        update_cursor(0);
        send_eoi(KBD_IRQ_NUM);
        return;
    }

    /* print to screen and add to buffer */
    if(KBD_MAP[scancode] != 0 && scancode != CTRL_PRS && scancode != B_SPACE && scancode != LSHIFT_PRS && scancode != RSHIFT_PRS && scancode != CAPS) { //only register characters (including enter and tab)
        if(buf_idx < BUF_SIZE-1) { //don't take any more characters if the buffer is full, "-1" is because final element of buffer is reserved for enter (newline)
            int capital = OFF; //should be capital letter if 1
            if((caps_lock == ON) ^ (shift_flag == ON)){capital = ON;} //set capital flag
            if(KBD_MAP[scancode] >= 'a' && KBD_MAP[scancode] <= 'z')
                read_buf[buf_idx++] = KBD_MAP[scancode] - capital*CAP_OFFSET; //add character to buffer (accounting for case) and increment index
            else if(capital == ON)
                read_buf[buf_idx++] = KBD_MAP_SHIFT[scancode]; //add character to buffer (accounting for case) and increment index
            else
                read_buf[buf_idx++] = KBD_MAP[scancode]; //add character to buffer (accounting for case) and increment index
            read_buf[buf_idx] = '\0'; //set end of string
            switch(scancode) {
                case TAB:
                    print_idx += TAB_LEN; //add 5 spaces/tab
                    break;
                case ENTER:
                    prev_loc[prev_idx++] = print_idx; //set previous location
                    print_idx += NUM_COLS - (print_idx % NUM_COLS); //add number of characters between current position and new line
                    break;
                default: //for regular characters (only increment print index)
                    *(uint8_t *)(video_mem + (print_idx++ << 1)) = read_buf[buf_idx-1]; // "<< 1" because each character is 2 bytes
            }
            check_scroll(print_idx);
            update_cursor(print_idx);
        }
    }
    send_eoi(KBD_IRQ_NUM);
}

 /*
  * update_cursor(int index), adapted from wiki.osdev.org (by Dark Fiber)
  *   DESCRIPTION: moves blinking cursor to designated row and column when characters typed/deleted or tab/enter
  *   INPUTS: none
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: cursor moves
  */
 void update_cursor(int index) {
    outb(0x0F, VGA_LOW);
    outb((unsigned char)(index & 0xFF), VGA_HIGH);
    outb(0x0E, VGA_LOW);
    outb((unsigned char)((index >> 8) & 0xFF), VGA_HIGH);
 }

 /*
  * check_scroll(int tmp_index)
  *   DESCRIPTION: checks if have passed lower screen boundary and scrolls down one line if have
  *   INPUTS: index of next location for character to be printed
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: video memory shifted up 1 row (through copying)
  */
 void check_scroll(int tmp_index) {
    int i; //loop iterator
    if(tmp_index >= NUM_COLS*NUM_ROWS) { //check if outside screen boundary
        for(i = 0; i < NUM_COLS*(NUM_ROWS-1); i++) { //move screen up 1 line by copying
            *(uint8_t *)(video_mem + (i << 1)) = *(video_mem + ((i + NUM_COLS) << 1));
        }
        for(i = 0; i < NUM_COLS; i++) { //clear newly inserted line
            *(uint8_t *)(video_mem + ((NUM_COLS*(NUM_ROWS-1)) << 1) + (i << 1)) = ' ';   
        }
        print_idx -= NUM_COLS;
        for(i = 1; i < 25; i++) { //update array of previous locations for newlines
            prev_loc[i-1] = prev_loc[i]; //scrolling down 1 line means don't need 1st previous location
            prev_loc[i-1] -= NUM_COLS;
        }
        prev_idx--;
    }
 }

 /*
  * get_read_buf()
  *   DESCRIPTION: give newline-terminated buffer to terminal
  *   INPUTS: pointer to copy character buffer typed in to
  *   OUTPUTS: number of bytes input from keyboard
  *   RETURN VALUE: none
  *   SIDE EFFECTS: none
  */
int32_t get_read_buf(void* ptr, int32_t bytes) {
    while(read_buf[buf_idx-1] != ENT_ASC); //wait until enter key is pressed
    cli(); //make sure not to interrupt memcpy
    memcpy(ptr, (void*) read_buf, bytes);
    sti();
    return bytes;
}

 /*
  * clear_read_buf()
  *   DESCRIPTION: clear the read buffer
  *   INPUTS: none
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: none
  */
void clear_read_buf() {
    cli(); //make sure not writing to buffer while clearing it
    buf_idx = 0; //reset buffer index
    read_buf[0] = '\0'; //clear buffer
    sti();
 }

 /*
  * print_write_buf(const void* wrt_buf)
  *   DESCRIPTION: print a buffer to the screen at location given by cursor
  *   INPUTS: pointer to buffer to be written
  *   OUTPUTS: number of bytes written to terminal
  *   RETURN VALUE: none
  *   SIDE EFFECTS: none
  */
int32_t print_write_buf(const void* wrt_buf, int32_t bytes) {
    int i; //loop iterator
    char* buf = (char*) wrt_buf;
    if(buf[0] == '\0') return 0;
    for(i = 0; i < bytes; i++) {
        switch(buf[i]) {
            case '\0': break;
            case TAB_ASC:
                print_idx += TAB_LEN; //add 5 spaces/tab
                break;
            case ENT_ASC:
                print_idx += NUM_COLS - (print_idx % NUM_COLS);
                break;
            default: //for regular characters (only increment print index)
                *(uint8_t *)(video_mem + (print_idx << 1)) = buf[i]; // "<< 1" because each character is 2 bytes
                print_idx++;
        }
        check_scroll(print_idx);
    }
    update_cursor(print_idx);
    return bytes;
 }
