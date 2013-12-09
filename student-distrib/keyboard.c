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
#include "process.h"
#include "paging.h"

 #include "test_syscalls.h"


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

#define TEST_MULT_TERM 1
#define CHAR_DIS_SZ 2


static uint8_t store[NUM_TERMS][NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));
static uint8_t* stores[NUM_TERMS];

// display parameters
//static char* video_mem = (char *)VIDEO;
static uint8_t* video_mem = (uint8_t *)VIDEO;

/* terminal to which open, read, write, and close are performed on */
static uint32_t act_ops_term = 0;
/* terminal whose contents are currently displayed on screen */
static uint32_t act_disp_term = 0;
/* bufferes that are written to by a terminal when that terminal is not the 
   active display terminal */
//static uint8_t stores[NUM_TERMS][NUM_COLS*NUM_ROWS*CHAR_DIS_SZ];    
/* holds the position of the cursor in the store for each terminal */
static uint32_t cursors[NUM_TERMS];

/* pointers to buffers to write to for each terminal */
static uint8_t* write_buffs[NUM_TERMS];

static uint16_t print_inds[NUM_TERMS];

static uint8_t scancodes[NUM_TERMS];
/* odn't forgot to init to \0 */
static uint8_t last_chars[NUM_TERMS];  
static uint8_t read_buffs[NUM_TERMS][BUF_SIZE];
static uint16_t buff_inds[NUM_TERMS];

// flags
/* indicates whether or not each terminal's virtual display has been
   mapped for a user program */
static uint32_t vid_mapped[NUM_TERMS];
static int alt_flag;
static int ctrl_flag; 
static int shift_flag; 
static int caps_lock;


// function declarations
void check_scroll(uint32_t term_index);
void check_term_switch();
void set_display_term(uint32_t term_index);
void update_hw_cursor(uint32_t curs_pos);
void set_term_mapped(uint32_t term_index);
void update_cursor(uint32_t curs_pos, uint32_t term_index);

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
    uint32_t i,j;
    for(i=0; i<NUM_TERMS; i++)
        stores[i] = store[i];

    /* initialize settings for eeach terminal */
    for(i=0; i<NUM_TERMS; i++) {
        /* set terminals' stores to be black, blank */
        for(j=0; j<NUM_COLS*NUM_ROWS*CHAR_DIS_SZ; j+=2) {
            stores[i][j] = ' ';
            stores[i][j+1] = LT_GREY;
        }
        /* set terminals to write to respective stores */
        write_buffs[i] = stores[i];
        /* set terminals' virtual display memory as unmapped */
        vid_mapped[i] = OFF;
        /* set terminals' input buffers to be empty */
        buff_inds[i] = 0;
        read_buffs[i][buff_inds[i]] = '\0';
        last_chars[i] = '\0';
        /* set terminals to write at top left of display */
        print_inds[i] = 0;
    }
    /* init the active display terminal to write to video memory */
    write_buffs[act_disp_term] = (uint8_t*)video_mem;
    /* set all modifier keys as not pressed */
    ctrl_flag = OFF; 
    shift_flag = OFF; 
    caps_lock = OFF; 
    /* initialize cursor to top left corner of video display */
    update_cursor(0, act_disp_term); 
    /* init keyboard handler to echo incoming characters to display */
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
    int32_t flags;
    scancodes[act_disp_term] = inb(KBD_PORT); //get key press
    cli_and_save(flags);
    int16_t i;

#if 0
    if(print_inds[act_disp_term] == 0) {
        clear();
    }
#endif

    switch(scancodes[act_disp_term]) {
        case ALT_PRS:
            alt_flag = ON;
            break;
        case ALT_RLS:
            alt_flag = OFF;
            break;
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
            if(print_inds[act_disp_term] == 0) 
            { //can't backspace if at first location of video memory and buffer empty
                if(buff_inds[act_disp_term] == 0)
                {   
                    send_eoi(KBD_IRQ_NUM);
                    restore_flags(flags);
                    return;
                }
                //Scroll up to reveal previous command
                else
                {
                    //Remove last entry (a new line '\n') and set to endline
                    read_buffs[act_disp_term][--buff_inds[act_disp_term]] = '\0';
                    int num_to_print = 0;
                    int char_to_print = 0;

                    //find how many characters make up the next block of characters
                    while((read_buffs[act_disp_term][buff_inds[act_disp_term] - num_to_print] != ENT_ASC) && ((buff_inds[act_disp_term] - num_to_print) > 0) && (char_to_print < (NUM_COLS*NUM_ROWS)))
                    {
                        num_to_print++;
                        if (read_buffs[act_disp_term][buff_inds[act_disp_term] - num_to_print] == TAB_ASC) {
                            char_to_print += TAB_LEN; //increase number of chars to print by tab length
                        }
                        else {
                            char_to_print++; //increment for regular characters
                        }
                    }
                    //write new line(s) at top of screen
                    print_inds[act_disp_term] = 0;
                    //copy all characters for created line(s)
                    for(i = 0; i < num_to_print; i++)                               
                    {
                        switch(read_buffs[act_disp_term][buff_inds[act_disp_term] - num_to_print + i]) {
                            case TAB_ASC:
                                print_inds[act_disp_term] += TAB_LEN; //add 5 spaces/tab
                                break;
                            case ENT_ASC:
                                break;
                            default: //for regular characters (only increment print index)
                                *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1)) = read_buffs[act_disp_term][buff_inds[act_disp_term] - num_to_print + i]; // "<< 1" because each character is 2 bytes
                                *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) = TEXT_COLOR; //change text color (accessing attribute byte)
                                print_inds[act_disp_term]++;
                        }
                    }
                    update_cursor(print_inds[act_disp_term], act_disp_term);
                    send_eoi(KBD_IRQ_NUM);
                    restore_flags(flags);
                    return;
                }
            }
            if(buff_inds[act_disp_term] > 0) { //make sure not accessing empty buffer, decrement buff_inds[act_disp_term] since deleted char
                switch(read_buffs[act_disp_term][--buff_inds[act_disp_term]]) {
                    case TAB_ASC: //for backspacing after tab
                        print_inds[act_disp_term] -= TAB_LEN;
                        break;
                    case ENT_ASC: //for backspacing after new line
                        break;
                    default: //for backspacing regular characters
                        print_inds[act_disp_term]--;
                        *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1)) = ' '; //delete character and move print index back 1 char
                        *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) = TEXT_COLOR; //clear character's attribute byte
                }
                read_buffs[act_disp_term][buff_inds[act_disp_term]] = '\0'; //remove character from buffer
            }
            update_cursor(print_inds[act_disp_term], act_disp_term);
            send_eoi(KBD_IRQ_NUM);
            restore_flags(flags);
            return;
    } /* switch end bracket */

    check_term_switch();

    // clear screen 
    if (ctrl_flag == ON && scancodes[act_disp_term] == L_KEY) { //ctrl+L
        clear();
        clear_read_buf();
        print_inds[act_disp_term] = 0; //reset print location to top left corner
        update_cursor(0, act_disp_term);
        send_eoi(KBD_IRQ_NUM);
        restore_flags(flags);
        // test_halt(0);
        return;
    }

    // print to screen and add to buffer 
    //only register characters (including enter and tab)
    if(KBD_MAP[scancodes[act_disp_term]] != 0 && scancodes[act_disp_term] != CTRL_PRS && scancodes[act_disp_term] != B_SPACE && scancodes[act_disp_term] != LSHIFT_PRS && scancodes[act_disp_term] != RSHIFT_PRS && scancodes[act_disp_term] != CAPS) { 
        //reserve last element in buffer for newline character
        if(scancodes[act_disp_term] != ENTER && buff_inds[act_disp_term] == BUF_SIZE-2); 
        // don't take any more characters if the buffer is full, "-1" is 
        // because final element of buffer is reserved for enter (newline) 
        else if(buff_inds[act_disp_term] < BUF_SIZE-1) { 
            int capital = OFF; //should be capital letter if 1
            // check capital flag
            if((caps_lock == ON) ^ (shift_flag == ON))
                capital = ON; //set capital flag
            // add character to buffer (accounting for case) and increment index
            if(KBD_MAP[scancodes[act_disp_term]] >= 'a' && KBD_MAP[scancodes[act_disp_term]] <= 'z') {
                last_chars[act_disp_term] = read_buffs[act_disp_term][buff_inds[act_disp_term]++] = KBD_MAP[scancodes[act_disp_term]] - capital*CAP_OFFSET;
            }
            else if(capital == ON)
                last_chars[act_disp_term] = read_buffs[act_disp_term][buff_inds[act_disp_term]++] = KBD_MAP_SHIFT[scancodes[act_disp_term]];
            else
                last_chars[act_disp_term] = read_buffs[act_disp_term][buff_inds[act_disp_term]++] = KBD_MAP[scancodes[act_disp_term]];             
            read_buffs[act_disp_term][buff_inds[act_disp_term]] = '\0'; //set end of string
            // either handle tab, handle enter, or display to screen
            switch(scancodes[act_disp_term]) {
                case TAB:
                    print_inds[act_disp_term] += TAB_LEN; //add 5 spaces/tab
                    break;
                case ENTER:
                    print_inds[act_disp_term] += NUM_COLS - (print_inds[act_disp_term] % NUM_COLS); //add number of characters between current position and new line
                    break;
                default: //for regular characters (only increment print index)
                    *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1)) = last_chars[act_disp_term];
                    *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) = TEXT_COLOR;
                    print_inds[act_disp_term]++;
            }
            check_scroll(act_disp_term);
            update_cursor(print_inds[act_disp_term], act_disp_term);
        }
    }
    send_eoi(KBD_IRQ_NUM);
    restore_flags(flags);
}

/* check to see if a terminal switch occurred */
void check_term_switch()
{
    int32_t flags;
    cli_and_save(flags);
    /* check that alt is pressed */
    if (alt_flag == ON)  {
        /* check if any of the first 3 function keys are pressed */
        switch(scancodes[act_disp_term]) {
            /* change display to correct terminal if any of the first
               3 function keys are pressed */
            case F1:
                set_display_term(0);
                break;
            case F2:
                set_display_term(1);
                break;
            case F3:
                set_display_term(2);
                break;
            default:
                break;
        }
    }
    restore_flags(flags);
}

/* set a terminal as the active display terminal */
void set_display_term(uint32_t term_index)
{
    int32_t flags;
    cli_and_save(flags);
    /* copy video memory to old terminal's store */
    memcpy(stores[act_disp_term], video_mem, NUM_COLS*NUM_ROWS*CHAR_DIS_SZ);
    /* change old active terminal settings so that it writes to its store, 
       not video memory  */
    write_buffs[act_disp_term] = stores[act_disp_term];

    /* check if old active display terminal is video mapped */ 
    if(get_vid_mapped(act_disp_term)) 
        /* if so, map virtual video memory to store */
        remap_4KB_user_page(act_disp_term, (uint32_t)(write_buffs[act_disp_term]), 
                                        (uint32_t)VID_VIRT_ADDR);

    /* set requsted terminal as active display terminal  */
    act_disp_term = term_index;
    /*  copy new active display terminal's store into video memory */
    memcpy(video_mem, stores[act_disp_term], NUM_COLS*NUM_ROWS*CHAR_DIS_SZ);
    /* set new active terminal settings so that it writes to video memory,
       not its store */
    write_buffs[act_disp_term] = video_mem;

    /* check if new active display terminal is video mapped */ 
    if(get_vid_mapped(act_disp_term))
        /* if so, map virtual video memory to video memory */
        remap_4KB_user_page(act_disp_term, (uint32_t)(write_buffs[act_disp_term]),
                                        (uint32_t)VID_VIRT_ADDR);

    /* update video memory's cursor with saved viruatl, cursor */
    update_hw_cursor(cursors[act_disp_term]); 

    /* test multiple terminals */
    //act_ops_term = act_disp_term;

    restore_flags(flags);
}

/* set term_index to be active operations terminal. all subsequent reads,
   writes will be performed on this terminal */
void set_act_ops_term(uint32_t term_index)
{
    int32_t flags;
    cli_and_save(flags);
    act_ops_term = term_index;
    restore_flags(flags);
}

/* get the active operations terminal's virtual display address */
uint32_t get_act_ops_disp()
{
    return (uint32_t)write_buffs[act_ops_term];
}

 /*
  * update_cursor(int index), adapted from wiki.osdev.org (by Dark Fiber)
  *   DESCRIPTION: moves blinking cursor to designated row and column when
  *                characters typed/deleted or tab/enter
  *   INPUTS: index-- index in video memory
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: cursor moves
  */
 void update_cursor(uint32_t curs_pos, uint32_t term_index) {
    int32_t flags;
    cli_and_save(flags);
    /* update active ops terminal's virtual cursor */
    cursors[term_index] = curs_pos;
    /* set the cursor color in active ops terminal's virtual display */
    *(uint8_t *)(write_buffs[term_index] + ((curs_pos << 1) + 1)) = CURSOR_COLOR;
    /* update real cursor's location if term_index is also the 
       active display terminal */
    if(term_index == act_disp_term)
        update_hw_cursor(curs_pos);
    restore_flags(flags);
 }

void update_hw_cursor(uint32_t curs_pos) 
{
    outb(0x0F, VGA_LOW);
    outb((unsigned char)(curs_pos & 0xFF), VGA_HIGH);
    outb(0x0E, VGA_LOW);
    outb((unsigned char)((curs_pos >> 8) & 0xFF), VGA_HIGH);
    *(uint8_t *)(video_mem + ((curs_pos << 1) + 1)) = CURSOR_COLOR;
}

/* 
 *
 *
 *
 */
void check_scroll(uint32_t term_index)
{
    int32_t flags;
    cli_and_save(flags);
    uint32_t i;
    // check if the print location has run past the end display 
    if(print_inds[term_index] >= NUM_COLS*NUM_ROWS) {
        // copy every row to the row above it 
        for(i=0; i<(NUM_ROWS-1); i++)
            memcpy(write_buffs[term_index]+(2*i*NUM_COLS), write_buffs[term_index]+(2*(i+1)*NUM_COLS), 2*NUM_COLS);
            memcpy(write_buffs[term_index]+(2*i*NUM_COLS+1), write_buffs[term_index]+(2*(i+1)*NUM_COLS+1), 2*NUM_COLS);
         // clear newly inserted line 
        for(i=0; i < NUM_COLS; i++) {
            *(uint8_t *)(write_buffs[term_index] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + (i << 1)) = ' ';
            *(uint8_t *)(write_buffs[term_index] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + (i << 1) + 1) = TEXT_COLOR;
        }
        // begin printing at left-most position of lowest row 
        print_inds[term_index] -= NUM_COLS;
    }
    restore_flags(flags);
}

 /*
  * get_read_buf[act_ops_term]()
  *   DESCRIPTION: give newline-terminated buffer to terminal
  *   INPUTS: pointer to copy character buffer typed in to
  *   OUTPUTS: number of bytes input from keyboard
  *   RETURN VALUE: none
  *   SIDE EFFECTS: none
  */
int32_t get_read_buf(void* ptr, int32_t bytes) {
    int32_t flags;
    int dummy=0;
    clear_read_buf();
    //while(read_buffs[act_ops_term][buff_inds[act_ops_term]-1] != ENT_ASC); //wait until enter key is pressed
    while(last_chars[act_ops_term] != '\n')
        dummy++; //wait until enter key is pressed
    cli_and_save(flags); //make sure not to interrupt memcpy
    last_chars[act_ops_term] = '\0';
    if(bytes > buff_inds[act_ops_term])
        bytes = buff_inds[act_ops_term];
    memcpy(ptr, (void*) read_buffs[act_ops_term], bytes);
    restore_flags(flags);
    return bytes;
}

 /*
  * clear_read_buf[act_ops_term]()
  *   DESCRIPTION: clear the read buffer
  *   INPUTS: none
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: none
  */
void clear_read_buf() {
    int32_t flags;
    cli_and_save(flags); //make sure not writing to buffer while clearing it
    buff_inds[act_ops_term] = 0; //reset buffer index
    read_buffs[act_ops_term][0] = '\0'; //clear buffer
    restore_flags(flags);
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
    int32_t flags;
    char* buf = (char*) wrt_buf;
    if(buf[0] == '\0') return 0;
    cli_and_save(flags); //clear interrupts to ensure buffer correctly printed to screen
    for(i = 0; i < bytes; i++) {
        switch(buf[i]) {
            case '\0': break;
            case TAB_ASC:
                print_inds[act_ops_term] += TAB_LEN; //add 5 spaces/tab
                break;
            case ENT_ASC:
                print_inds[act_ops_term] += NUM_COLS - (print_inds[act_ops_term] % NUM_COLS);
                break;
            default: //for regular characters (only increment print index)
                *(uint8_t *)(write_buffs[act_ops_term] + (print_inds[act_ops_term] << 1)) = buf[i]; // "<< 1" because each character is 2 bytes
                *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) = PROG_COLOR;
                print_inds[act_ops_term]++;
        }
        check_scroll(act_ops_term);
    }
    update_cursor(print_inds[act_ops_term], act_ops_term);
    restore_flags(flags);
    return bytes;
 }

