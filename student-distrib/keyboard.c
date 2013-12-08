/*
 *  The keyboard fileet' purpose is to handle interrupts coming
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
#include "paging.h"
#include "process.h"
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

//typedef uint8_t store_t[NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));


// display parameters
//static char* video_mem = (char *)VIDEO;
static uint8_t* video_mem = (uint8_t *)VIDEO;
static uint8_t  store_0[NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));
static uint8_t  store_1[NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));
static uint8_t  store_2[NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));

/* terminal to which open, read, write, and close are performed on */
static uint32_t act_ops_term = 0;
/* terminal whose contents are currently displayed on screen */
static uint32_t act_disp_term = 0;
/* bufferes that are written to by a terminal when that terminal is not the 
   active display terminal */
//static uint8_t stores[NUM_TERMS][NUM_COLS*NUM_ROWS*CHAR_DIS_SZ];    
static uint8_t* stores[NUM_TERMS];    


/* holds the position of the cursor in the store for each terminal */
static uint32_t cursors[NUM_TERMS];

/* pointers to buffers to write to for each terminal */
static uint8_t* write_buffs[NUM_TERMS];

static uint16_t print_inds[NUM_TERMS];

static uint8_t scancodes[NUM_TERMS];
/* don't forget to init to \0 */
static uint8_t last_chars[NUM_TERMS];  
static uint8_t read_buffs[NUM_TERMS][BUF_SIZE];
static uint16_t buff_inds[NUM_TERMS];

/* indicates whether or not each terminal's virtual display has been
   mapped for a user program */
static uint32_t vid_mapped[NUM_TERMS];
static int ctrl_flag; //flag to indicate if control key currently pressed or not (1 is yes, 0 is no)
static int shift_flag; //indicates if shift is pressed or not
static int alt_flag; //indicates if alt is pressed or not
static int caps_lock; //indicates if caps lock is enabled/disabled

//color scheme variables
static uint8_t color_scheme[NUM_TERMS]; //index to color arrays for typed color, printed color, and cursor color
static uint32_t bg_scheme[NUM_TERMS]; //index for background color scheme
static const char TYPED_COLOR[10] =       { GREEN,      BLACK,      WHITE,      BROWN,      BLUE,       MAGENTA,    BLACK,      RED,        LT_CYAN,    LT_BROWN};
static const char PRINTED_COLOR[10] =     { GREEN,      BLACK,      WHITE,      RED,        LT_BLUE,    LT_MAGENTA, RED,        BLUE,       LT_MAGENTA, MAGENTA};
static const char CURSOR_COLOR[10] =      { GREEN,      BLACK,      WHITE,      RED,        LT_BLUE,    LT_MAGENTA, RED,        BLUE,       LT_GREEN,   LT_RED};
static const char BACKGROUND_COLOR[10] =  { BLACK,      WHITE,      DK_GREY,    LT_GREY,    LT_BLUE,    LT_GREEN,   LT_CYAN,    LT_RED,     LT_MAGENTA, LT_BROWN};
static const char STATUS_BAR_COLOR[10] =  { WHITE,      BLACK,      WHITE,      BLACK,      BLACK,      BLACK,      BLACK,      BLACK,      BLACK,      BLACK};
static const char SBAR_TEXT_COLOR[10] =   { BLACK,      WHITE,      BLACK,      WHITE,      WHITE,      WHITE,      WHITE,      WHITE,      WHITE,      WHITE};

// function declarations
void check_scroll();
void change_text_colors(uint8_t color_combo);
void change_background(uint8_t bg_color);
void check_term_switch();
void set_display_term(uint32_t term_index);
void update_hw_cursor(uint32_t curs_pos);
void set_term_mapped(uint32_t term_index);

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
    /* initialize settings for eeach terminal */
    stores[0] = store_0;
    stores[1] = store_1;
    stores[2] = store_2;

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
    alt_flag = OFF;
    caps_lock = OFF; 
    /* initialize cursor to top left corner of video display */
    update_cursor(0); 
    /* init keyboard handler to echo incoming characters to display */

    //initialize color schemes and status bar
    for(i=0; i<NUM_TERMS; i++) {
        bg_scheme[i] = 0;
        color_scheme[i] = 0;
    }

    change_background(bg_scheme[act_disp_term]); //initialize background to default color
    change_text_colors(color_scheme[act_disp_term]); //initialize keyboard color
    print_status_bar();

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
    scancodes[act_ops_term] = inb(KBD_PORT); //get key press

    uint16_t i; //loop iterator
    uint8_t clr_combo; //color combination number

#if 0
    if(print_inds[act_ops_term] == 0) {
        clear();
    }
#endif

    //change background color with alt+num
    if (alt_flag == ON && ctrl_flag == OFF && (scancodes[act_ops_term] >= ONE_ASC && scancodes[act_ops_term] <= ZERO_ASC)) {
        if (scancodes[act_ops_term] == ZERO_ASC) clr_combo = 0;
        else clr_combo = scancodes[act_ops_term] - NUM_SCANCODE_OFFSET;
        change_background(clr_combo);
        print_status_bar(); //update status bar background color info field
        send_eoi(KBD_IRQ_NUM);
        return;
    }

    //change color scheme with ctrl+num
    if (ctrl_flag == ON && alt_flag == OFF && (scancodes[act_ops_term] >= ONE_ASC && scancodes[act_ops_term] <= ZERO_ASC)) {
        if (scancodes[act_ops_term] == ZERO_ASC) clr_combo = 0;
        else clr_combo = scancodes[act_ops_term] - NUM_SCANCODE_OFFSET;
        change_text_colors(clr_combo);
        print_status_bar(); //update status bar text color info field
        send_eoi(KBD_IRQ_NUM);
        return;
    }

    // clear screen with ctrl+l
    if (ctrl_flag == ON && scancodes[act_ops_term] == L_KEY) {
        cli(); //make sure screen completely clear before anything is typed
        clear();
        clear_read_buf();
        for(i = 0; i < STATUS_BAR_START; i++) { //reset screen to background color
            *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) &= 0x0F;
            *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4);
        }
        print_inds[act_ops_term] = 0; //reset print location to top left corner
        update_cursor(0);
        sti();
        send_eoi(KBD_IRQ_NUM);
        test_halt(0);
        return;
    }

    switch(scancodes[act_ops_term]) {
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
        case ALT_PRS:
            alt_flag = ON; //set alt flag
            break;
        case ALT_RLS:
            alt_flag = OFF; //clear alt flag
            break;
        case CAPS:
            caps_lock ^= 1; //invert value of caps lock
            break;
        case B_SPACE:
            if(print_inds[act_ops_term] == 0) 
            { //can't backspace if at first location of video memory and buffer empty
                if(buff_inds[act_ops_term] == 0)
                {   
                    send_eoi(KBD_IRQ_NUM);
                    return;
                }
                //Scroll up to reveal previous command
                else
                {
                    //Remove last entry (a new line '\n') and set to endline
                    read_buffs[act_ops_term][--buff_inds[act_ops_term]] = '\0';
                    int num_to_print = 0;
                    int char_to_print = 0;

                    //find how many characters make up the next block of characters
                    while((read_buffs[act_ops_term][buff_inds[act_ops_term] - num_to_print] != ENT_ASC) && ((buff_inds[act_ops_term] - num_to_print) > 0) && (char_to_print < (NUM_COLS*NUM_ROWS)))
                    {
                        num_to_print++;
                        if (read_buffs[act_ops_term][buff_inds[act_ops_term] - num_to_print] == TAB_ASC) {
                            char_to_print += TAB_LEN; //increase number of chars to print by tab length
                        }
                        else {
                            char_to_print++; //increment for regular characters
                        }
                    }
                    //write new line(s) at top of screen
                    print_inds[act_ops_term] = 0;
                    //copy all characters for created line(s)
                    for(i = 0; i < num_to_print; i++)                               
                    {
                        switch(read_buffs[act_ops_term][buff_inds[act_ops_term] - num_to_print + i]) {
                            case TAB_ASC:
                                print_inds[act_ops_term] += TAB_LEN; //add 5 spaces/tab
                                break;
                            case ENT_ASC:
                                break;
                            default: //for regular characters (only increment print index)
                                *(uint8_t *)(write_buffs[act_ops_term] + (print_inds[act_ops_term] << 1)) = read_buffs[act_ops_term][buff_inds[act_ops_term] - num_to_print + i]; // "<< 1" because each character is 2 bytes
                                *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) &= 0xF0; //maintain background color
                                *(uint8_t *)(write_buffs[act_ops_term] + (print_inds[act_ops_term] << 1) + 1) = TYPED_COLOR[color_scheme[act_disp_term]]; //change text color (accessing attribute byte)
                                print_inds[act_ops_term]++;
                        }
                    }
                    update_cursor(print_inds[act_ops_term]);
                    send_eoi(KBD_IRQ_NUM);
                    return;
                }
            }
            if(buff_inds[act_ops_term] > 0) { //make sure not accessing empty buffer, decrement buff_inds[act_ops_term] since deleted char
                switch(read_buffs[act_ops_term][--buff_inds[act_ops_term]]) {
                    case TAB_ASC: //for backspacing after tab
                        print_inds[act_ops_term] -= TAB_LEN;
                        break;
                    case ENT_ASC: //for backspacing after new line
                        break;
                    default: //for backspacing regular characters
                        print_inds[act_ops_term]--;
                        *(uint8_t *)(write_buffs[act_ops_term] + (print_inds[act_ops_term] << 1)) = ' '; //delete character and move print index back 1 char
                        *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) &= 0xF0; //maintain background color
                        *(uint8_t *)(write_buffs[act_ops_term] + (print_inds[act_ops_term] << 1) + 1) = TYPED_COLOR[color_scheme[act_disp_term]]; //clear character's attribute byte
                }
                read_buffs[act_ops_term][buff_inds[act_ops_term]] = '\0'; //remove character from buffer
            }
            update_cursor(print_inds[act_ops_term]);
            send_eoi(KBD_IRQ_NUM);
            return;
        default:
            break;
    } /* switch end bracket */

    check_term_switch();

    // print to screen and add to buffer 
    //only register characters (including enter and tab)
    if(KBD_MAP[scancodes[act_ops_term]] != 0 && scancodes[act_ops_term] != CTRL_PRS && scancodes[act_ops_term] != B_SPACE && scancodes[act_ops_term] != LSHIFT_PRS && scancodes[act_ops_term] != RSHIFT_PRS && scancodes[act_ops_term] != CAPS) { 
        //reserve last element in buffer for newline character
        if(scancodes[act_ops_term] != ENTER && buff_inds[act_ops_term] == BUF_SIZE-2); 
        // don't take any more characters if the buffer is full, "-1" is 
        // because final element of buffer is reserved for enter (newline) 
        else if(buff_inds[act_ops_term] < BUF_SIZE-1) { 
            int capital = OFF; //should be capital letter if 1
            // check capital flag
            if((caps_lock == ON) ^ (shift_flag == ON))
                capital = ON; //set capital flag
            // add character to buffer (accounting for case) and increment index
            if(KBD_MAP[scancodes[act_ops_term]] >= 'a' && KBD_MAP[scancodes[act_ops_term]] <= 'z') {
                //read_buffs[act_ops_term][buff_inds[act_ops_term]++] = KBD_MAP[scancodes[act_ops_term]] - capital*CAP_OFFSET; 
                last_chars[act_ops_term] = read_buffs[act_ops_term][buff_inds[act_ops_term]++] = KBD_MAP[scancodes[act_ops_term]] - capital*CAP_OFFSET;
            }
                //read_buffs[act_ops_term][buff_inds[act_ops_term]++] = KBD_MAP[scancodes[act_ops_term]] - capital*CAP_OFFSET; 
            else if(capital == ON)
                //read_buffs[act_ops_term][buff_inds[act_ops_term]++] = KBD_MAP_SHIFT[scancodes[act_ops_term]];
                last_chars[act_ops_term] = read_buffs[act_ops_term][buff_inds[act_ops_term]++] = KBD_MAP_SHIFT[scancodes[act_ops_term]];
            else
                //read_buffs[act_ops_term][buff_inds[act_ops_term]++] = KBD_MAP[scancodes[act_ops_term]];
                last_chars[act_ops_term] = read_buffs[act_ops_term][buff_inds[act_ops_term]++] = KBD_MAP[scancodes[act_ops_term]];             
            read_buffs[act_ops_term][buff_inds[act_ops_term]] = '\0'; //set end of string
            // either handle tab, handle enter, or display to screen
            switch(scancodes[act_ops_term]) {
                case TAB:
                    print_inds[act_ops_term] += TAB_LEN; //add 5 spaces/tab
                    break;
                case ENTER:
                    print_inds[act_ops_term] += NUM_COLS - (print_inds[act_ops_term] % NUM_COLS); //add number of characters between current position and new line
                    break;
                default: //for regular characters (only increment print index)
                    *(uint8_t *)(write_buffs[act_ops_term] + (print_inds[act_ops_term] << 1)) = last_chars[act_ops_term];
                    *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) &= 0xF0; //maintain background color
                    *(uint8_t *)(write_buffs[act_ops_term] + (print_inds[act_ops_term] << 1) + 1) = TYPED_COLOR[color_scheme[act_disp_term]];
                    *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) &= 0x0F; //clear background color
                    *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4); //change to new background color
                    print_inds[act_ops_term]++;
            }
            check_scroll();
            update_cursor(print_inds[act_ops_term]);
        }
    }
    send_eoi(KBD_IRQ_NUM);
}

/* check to see if a terminal switch occurred */
void check_term_switch()
{
    /* check that alt is pressed */
    if (alt_flag == ON)  {
        /* check if any of the first 3 function keys are pressed */
        switch(scancodes[act_ops_term]) {
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
}

/* set a terminal as the active display terminal */
void set_display_term(uint32_t term_index)
{
    /* copy video memory to old terminal's store */
    memcpy(stores[act_disp_term], video_mem, NUM_COLS*NUM_ROWS*CHAR_DIS_SZ);
    /* change old active display terminal settings so that it writes to its  
       store, not video memory  */
    write_buffs[act_disp_term] = stores[act_disp_term];
#if 0
    /* check if old active display terminal is video mapped */ 
    if(vid_mapped[act_disp_term]) 
        /* if so, map virtual video memory to store */
        remap_4KB_user_page((uint32_t)(write_buffs[act_disp_term]), 
                            (uint32_t)VID_VIRT_ADDR);
#endif

    /* set requsted terminal as active display terminal  */
    act_disp_term = term_index;

    /*  copy new active display terminal's store into video memory */
    memcpy(video_mem, stores[act_disp_term], NUM_COLS*NUM_ROWS*CHAR_DIS_SZ);
    /* set new active terminal settings so that it writes to video memory,
       not its store */
    write_buffs[act_disp_term] = video_mem; 
#if 0
    /* check if new active display terminal is video mapped */ 
    if(vid_mapped[act_disp_term])
        /* if so, map virtual video memory to video memory */
        remap_4KB_user_page(((uint32_t)write_buffs[act_disp_term]),
                            (uint32_t)VID_VIRT_ADDR);
#endif
    /* update video memory's cursor with saved virtual, cursor */
    update_hw_cursor(cursors[act_disp_term]);

    /* test multiple terminals */
    act_ops_term = act_disp_term;

    change_background(bg_scheme[act_disp_term]); //initialize background to default color
    change_text_colors(color_scheme[act_disp_term]); //initialize keyboard color
    print_status_bar();
}

/* set term_index to be active operations terminal. all subsequent reads,
   writes will be performed on this terminal */
void set_act_ops_term(uint32_t term_index)
{
    act_ops_term = term_index;
}

/* get the active operations terminal's virtual display address */
uint32_t get_act_ops_disp()
{
    vid_mapped[act_ops_term] = ON;
    return (uint32_t)(write_buffs[act_ops_term]);

}

 /*
  * update_cursor(uint32_t curs_pos), adapted from wiki.osdev.org (by Dark Fiber)
  *   DESCRIPTION: moves blinking cursor to designated row and column when characters typed/deleted or tab/enter
  *   INPUTS: index in video memory
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: cursor moves
  */
 void update_cursor(uint32_t curs_pos) {
    /* update active ops terminal's virtual cursor */
    cursors[act_ops_term] = curs_pos;
    /* set the cursor color in active ops terminal's virtual display */
    *(uint8_t *)(write_buffs[act_ops_term] + ((curs_pos << 1) + 1)) = CURSOR_COLOR[color_scheme[act_disp_term]];
    /* update real cursor's location if the the active ops terminal is also the 
       active display terminal */
    if(act_ops_term == act_disp_term)
        update_hw_cursor(curs_pos);
 }

void update_hw_cursor(uint32_t curs_pos) 
{
    outb(0x0F, VGA_LOW);
    outb((unsigned char)(curs_pos & 0xFF), VGA_HIGH);
    outb(0x0E, VGA_LOW);
    outb((unsigned char)((curs_pos >> 8) & 0xFF), VGA_HIGH);
    *(uint8_t *)(video_mem + ((curs_pos << 1) + 1)) &= 0xF0; //maintain background color
    *(uint8_t *)(video_mem + ((curs_pos << 1) + 1)) = CURSOR_COLOR[color_scheme[act_disp_term]];
    *(uint8_t *)(video_mem + ((curs_pos << 1) + 1)) &= 0x0F; //clear background color
    *(uint8_t *)(video_mem + ((curs_pos << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4); //change to new background color
}
/* 
 *
 *
 *
 */
void check_scroll()
{
    uint32_t i;
    cli(); //make sure video memory manipulated atomically
    // check if the print location has run past the end display 
    if(print_inds[act_ops_term] >= STATUS_BAR_START) {
        // copy every row to the row above it 
        for(i=0; i<(NUM_ROWS-1); i++) {
            memcpy(write_buffs[act_ops_term]+(2*i*NUM_COLS), write_buffs[act_ops_term]+(2*(i+1)*NUM_COLS), 2*NUM_COLS);
            memcpy(write_buffs[act_ops_term]+(2*i*NUM_COLS+1), write_buffs[act_ops_term]+(2*(i+1)*NUM_COLS+1), 2*NUM_COLS);
        }
         // clear newly inserted line 
        for(i=0; i < NUM_COLS; i++) {
            *(uint8_t *)(write_buffs[act_ops_term] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + (i << 1)) = ' ';
            *(uint8_t *)(write_buffs[act_ops_term] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + (i << 1) + 1) &= 0xF0; //maintain background color
            *(uint8_t *)(write_buffs[act_ops_term] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + (i << 1) + 1) = TYPED_COLOR[color_scheme[act_disp_term]];
            *(uint8_t *)(write_buffs[act_ops_term] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + ((i << 1) + 1)) &= 0x0F; //clear background color
            *(uint8_t *)(write_buffs[act_ops_term] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + ((i << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4); //change to new background color
        }
        // begin printing at left-most position of lowest row 
        print_inds[act_ops_term] -= NUM_COLS;
    }
    sti();
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
    int dummy=0;
    clear_read_buf();
    //while(read_buffs[act_ops_term][buff_inds[act_ops_term]-1] != ENT_ASC); //wait until enter key is pressed
    while(last_chars[act_ops_term] != '\n')
        dummy++; //wait until enter key is pressed
    cli(); //make sure not to interrupt memcpy
    last_chars[act_ops_term] = '\0';
    if(bytes > buff_inds[act_ops_term])
        bytes = buff_inds[act_ops_term];
    memcpy(ptr, (void*) read_buffs[act_ops_term], bytes);
    sti();
    return bytes;
}

 /*
  * clear_read_buf[act_ops_term]()
  *   DESCRIPTION: clear the read buffer
  *   INPUTS: none
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: buffer is cleared
  */
void clear_read_buf() {
    cli(); //make sure not writing to buffer while clearing it
    buff_inds[act_ops_term] = 0; //reset buffer index
    read_buffs[act_ops_term][0] = '\0'; //clear buffer
    sti();
 }

 /*
  * print_write_buf(const void* wrt_buf)
  *   DESCRIPTION: print a buffer to the screen at location given by cursor
  *   INPUTS: pointer to buffer to be written and number of bytes to write
  *   OUTPUTS: number of bytes written to terminal
  *   RETURN VALUE: none
  *   SIDE EFFECTS: none
  */
int32_t print_write_buf(const void* wrt_buf, int32_t bytes) {
    uint16_t i, j; //loop iterators
    char* buf = (char*) wrt_buf;
    if(buf[0] == '\0') return 0;
    cli(); //clear interrupts to ensure buffer correctly printed to screen
    for(i = 0; i < bytes; i++) {
        switch(buf[i]) {
            case '\0': break;
            case TAB_ASC:
                for(j = print_inds[act_ops_term]; j < TAB_LEN; j++) {
                    *(uint8_t *)(write_buffs[act_ops_term] + (((print_inds[act_ops_term] + j) << 1) + 1)) &= 0x0F; //clear background color
                    *(uint8_t *)(write_buffs[act_ops_term] + (((print_inds[act_ops_term] + j) << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4); //change to new background color
                }
                print_inds[act_ops_term] += TAB_LEN;
                break;
            case ENT_ASC:
                for(j = 0; j < (NUM_COLS - (print_inds[act_ops_term] % NUM_COLS)); j++) {
                    *(uint8_t *)(write_buffs[act_ops_term] + (((print_inds[act_ops_term] + j) << 1) + 1)) &= 0x0F; //clear background color
                    *(uint8_t *)(write_buffs[act_ops_term] + (((print_inds[act_ops_term] + j) << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4); //change to new background color
                }
                print_inds[act_ops_term] += NUM_COLS - (print_inds[act_ops_term] % NUM_COLS);
                break;
            default: //for regular characters (only increment print index)
                *(uint8_t *)(write_buffs[act_ops_term] + (print_inds[act_ops_term] << 1)) = buf[i];
                *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) &= 0xF0; //maintain background color
                *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) = PRINTED_COLOR[color_scheme[act_disp_term]];
                *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) &= 0x0F;
                *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4);
                print_inds[act_ops_term]++;
        }
        check_scroll();
    }
    update_cursor(print_inds[act_ops_term]);
    sti();
    return bytes;
 }
 /*
  * change_text_colors(int8_t color_combo)
  *   DESCRIPTION: change colors of terminal output, typed text, and cursor
  *   INPUTS: color combination number (0 is default)
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: terminal text colors changed
  */
void change_text_colors(uint8_t color_combo) {
    if (color_combo > 9) return; //must be 1-digit number
    color_scheme[act_disp_term] = color_combo;
    update_cursor(print_inds[act_ops_term]); //update cursor with new color
 }

 /*
  * change_background(bg_color)
  *   DESCRIPTION: change terminal background color
  *   INPUTS: background color number (0 is default)
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: terminal background color changed
  */
void change_background(uint8_t bg_color) {
    uint16_t i; //loop iterator

    if (bg_color > 9) return; //must be 1-digit number
    bg_scheme[act_disp_term] = bg_color;

    cli(); //clear interrupts to make sure background fully written before anything is typed

    //change background color
    for(i = 0; i < STATUS_BAR_START; i++) {
        *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) &= 0x0F; //clear background color
        *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4);
    }

    //change status bar color
    for(i = STATUS_BAR_START; i <= STATUS_BAR_END; i++) {
        *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) &= 0xF0; //maintain background color
        *(uint8_t *)(write_buffs[act_disp_term] + (i << 1) + 1) = SBAR_TEXT_COLOR[bg_scheme[act_disp_term]];
        *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) &= 0x0F; //clear background color
        *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) |= (STATUS_BAR_COLOR[bg_scheme[act_disp_term]] << 4);
    }

    sti();
 }

  /*
  * print_status_bar()
  *   DESCRIPTION: print status bar at bottom of terminal with color according to
                    background scheme
  *   INPUTS: none
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: status bar updated
  */
void print_status_bar() {
    uint16_t i, j; //loop iterator
    char* os_name = "SegFault OS";

    char term_num_text[12]; //exactly 11 chars needed
    char term_char[2]; //buffer to hold act_disp_term in ASCII (1 for num, 1 for '\0')
    char* term_num_info; //string of "Terminal: act_disp_term"

    char processes_text[13];
    char processes_char[2]; //buffer to hold num_proc in ASCII (1 for num, 1 for '\0')
    char* processes_info; //string of "Processes: num_proc"

    char bg_color_text[19];
    char bg_scheme_char[2]; //buffer to hold bg_scheme in ASCII (1 for num, 1 for '\0')
    char* bg_color_info; //string of "Bakcground Color: bg_scheme"

    char text_color_text[13];
    char text_scheme_char[2]; //buffer to hold bg_scheme in ASCII
    char* text_color_info;

    //set status bar background and text color
    for(i = STATUS_BAR_START; i <= STATUS_BAR_END; i++) {
        *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) &= 0xF0; //maintain background color
        *(uint8_t *)(write_buffs[act_disp_term] + (i << 1) + 1) = SBAR_TEXT_COLOR[bg_scheme[act_disp_term]];
        *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) &= 0x0F; //clear background color
        *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) |= (STATUS_BAR_COLOR[bg_scheme[act_disp_term]] << 4);
    }

    strcpy(term_num_text, "Terminal: ");
    strcpy(processes_text, "Processes: ");
    strcpy(bg_color_text, "Background Color: ");
    strcpy(text_color_text, "Text Color: ");

    // convert number to ASCII and copy to info buffer
    itoa(act_disp_term, term_char, (int32_t) 10);
    term_num_info = strcat(term_num_text, (char*) term_char);
    //itoa(get_num_processes(), processes_char, (int32_t) 10);
    itoa(1, processes_char, (int32_t) 10);
    processes_info = strcat(processes_text, (char*) processes_char);
    itoa(bg_scheme[act_disp_term], bg_scheme_char, (int32_t) 10);
    bg_color_info = strcat(bg_color_text, (char*) bg_scheme_char);
    itoa(color_scheme[act_disp_term], text_scheme_char, (int32_t) 10);
    text_color_info = strcat(text_color_text, (char*) text_scheme_char);

    // print OS name in status bar
    for(i = STATUS_BAR_START; i < (STATUS_BAR_START+strlen(os_name)); i++) {
        *(uint8_t *)(write_buffs[act_disp_term] + (i << 1)) = os_name[i-STATUS_BAR_START];
    }

    j = 0;
    //print terminal number
    for(i = TERM_INFO_START; i < (TERM_INFO_START+strlen(term_num_info)); i++) {
        *(uint8_t *)(write_buffs[act_disp_term] + (i << 1)) = term_num_info[j];
        j++;
    }

    j = 0;
    //print number of running processes
    for(i = PROC_INFO_START; i < (PROC_INFO_START+strlen(processes_info)); i++) {
        *(uint8_t *)(write_buffs[act_disp_term] + (i << 1)) = processes_info[j];
        j++;
    }

    j = 0;
    //print background color scheme number
    for(i = BG_CLR_INFO_START; i < (BG_CLR_INFO_START+strlen(bg_color_info)); i++) {
        *(uint8_t *)(write_buffs[act_disp_term] + (i << 1)) = bg_color_info[j];
        j++;
    }

    j = 0;
    //print text color scheme number
    for(i = TXT_CLR_INFO_START; i < (TXT_CLR_INFO_START+strlen(text_color_info)); i++) {
        *(uint8_t *)(write_buffs[act_disp_term] + (i << 1)) = text_color_info[j];
        j++;
    }
 }

  /*
  * strcat(char *dest, char *src)
  *   DESCRIPTION: concatenates 2 strings, adapted from http://stackoverflow.com/questions/2488563/strcat-implementation
  *   INPUTS: dest is string that will end up first, src is string that will end up second
  *   OUTPUTS: none
  *   RETURN VALUE: concatenation of the 2 strings (order dest, src)
  *   SIDE EFFECTS: none
  */
char* strcat(char *dest, char *src)
{
    uint8_t i,j;
    for (i = 0; dest[i] != '\0'; i++)
        ;
    for (j = 0; src[j] != '\0'; j++)
        dest[i+j] = src[j];
    dest[i+j] = '\0';
    return dest;
}

/*
* get_active_terminal
*   DESCRIPTION: lets other functions determine active terminal
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: active terminal number
*   SIDE EFFECTS: none
*/
uint32_t get_active_terminal()
{
    return 0;
}
