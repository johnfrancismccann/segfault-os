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
#include "terminal.h"

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

/* stores for scrolling */
//stores for previous data to access when scrolling up
static uint8_t  up_store_0[NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));
static uint8_t  up_store_1[NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));
static uint8_t  up_store_2[NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));
//store for most recent data to acess when scrolling down after scrolling up
static uint8_t  dn_store_0[NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));
static uint8_t  dn_store_1[NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));
static uint8_t  dn_store_2[NUM_COLS*NUM_ROWS*CHAR_DIS_SZ] __attribute__((aligned(PG_TBL_ALIGN)));
/* scrolling variables */
static uint8_t* scroll_stores[NUM_TERMS][2]; //1 for up scroll store and 1 for down
static uint8_t max_lines_up[NUM_TERMS]; //maximum number of lines can scroll up
static uint8_t max_lines_down[NUM_TERMS]; //max lines to scroll down

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

//color scheme variables
static uint8_t color_scheme[NUM_TERMS]; //index to color arrays for typed color, printed color, and cursor color
static uint32_t bg_scheme[NUM_TERMS]; //index for background color scheme
static const char TYPED_COLOR[10] =       { GREEN,      BLACK,      WHITE,      BROWN,      BLUE,       MAGENTA,    BLACK,      RED,        LT_CYAN,    LT_BROWN};
static const char PRINTED_COLOR[10] =     { GREEN,      BLACK,      WHITE,      RED,        LT_BLUE,    LT_MAGENTA, RED,        BLUE,       LT_MAGENTA, MAGENTA};
static const char CURSOR_COLOR[10] =      { GREEN,      BLACK,      WHITE,      RED,        LT_BLUE,    LT_MAGENTA, RED,        BLUE,       LT_GREEN,   LT_RED};
static const char BACKGROUND_COLOR[10] =  { BLACK,      WHITE,      DK_GREY,    LT_GREY,    LT_BLUE,    LT_GREEN,   LT_CYAN,    LT_RED,     LT_MAGENTA, LT_BROWN};
static const char STATUS_BAR_COLOR[10] =  { WHITE,      BLACK,      WHITE,      BLACK,      BLACK,      BLACK,      BLACK,      BLACK,      BLACK,      BLACK};
static const char SBAR_TEXT_COLOR[10] =   { BLACK,      WHITE,      BLACK,      WHITE,      WHITE,      WHITE,      WHITE,      WHITE,      WHITE,      WHITE};

//Tab complete lookup array.  Static since overhead is nasty for dynamic generation.
#define NUM_FILES 15
static const int8_t* FILES[NUM_FILES] = {".","cat","counter","fish","frame0.txt","frame1.txt",
                                         "grep","hello","ls","pingpong","shell","sigtest","syserr",
                                         "testprint","verylargetxtwithverylongname.tx"};

static uint8_t cmd_hist[NUM_TERMS][CMD_HIST_LEN][BUF_SIZE] = {{{'\0'}}}; //gcc bug #53119 fix
static int32_t active_command[NUM_TERMS] = {0};

// function declarations
void check_scroll(uint32_t term_index);
void check_term_switch();
void set_display_term(uint32_t term_index);
void update_hw_cursor(uint32_t curs_pos);
void set_term_mapped(uint32_t term_index);
void update_cursor(uint32_t curs_pos, uint32_t term_index);
void change_text_colors(uint8_t color_combo);
void change_background(uint8_t bg_color);
static void autocomplete();
static uint8_t* get_prev_word(uint8_t endindex);
static uint8_t partial_strcmp(uint8_t* partial, uint8_t* full);
static void type_char(uint8_t input);
static void type_str(uint8_t* input);
void scroll_up();
void scroll_down();
static void ins_cmd_hist(uint8_t* input);
static void newline_to_null(uint8_t* input);


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

    scroll_stores[0][UP] = up_store_0;
    scroll_stores[0][DN] = dn_store_0;
    scroll_stores[1][UP] = up_store_1;
    scroll_stores[1][DN] = dn_store_1;
    scroll_stores[2][UP] = up_store_2;
    scroll_stores[2][DN] = dn_store_2;

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
    alt_flag = OFF;
    caps_lock = OFF; 
    /* initialize cursor to top left corner of video display */
    update_cursor(0, act_disp_term); 
    /* init keyboard handler to echo incoming characters to display */
    //initialize color schemes and status bar

#if COLORS_ENABLED
    for(i=0; i<NUM_TERMS; i++) {
        bg_scheme[i] = 0;
        color_scheme[i] = 0;
        max_lines_up[i] = 0;
        max_lines_down[i] = 0;
    }

    change_background(bg_scheme[act_disp_term]); //initialize background to default color
    change_text_colors(color_scheme[act_disp_term]); //initialize keyboard color
    print_status_bar();
#endif

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
// #if COLORS_ENABLED
    uint8_t clr_combo; //color combination number
// #endif

#if 0
    if(print_inds[act_disp_term] == 0) {
        clear();
    }
#endif

// #if C0LORS_ENABLED
    //change background color with alt+num
    if (alt_flag == ON && ctrl_flag == OFF && (scancodes[act_disp_term] >= ONE_ASC && scancodes[act_disp_term] <= ZERO_ASC)) {
        if (scancodes[act_disp_term] == ZERO_ASC) clr_combo = 0;
        else clr_combo = scancodes[act_disp_term] - NUM_SCANCODE_OFFSET;
        change_background(clr_combo);
        print_status_bar(); //update status bar background color info field
        send_eoi(KBD_IRQ_NUM);
        return;
    }

    //change color scheme with ctrl+num
    else if (ctrl_flag == ON && alt_flag == OFF && (scancodes[act_disp_term] >= ONE_ASC && scancodes[act_disp_term] <= ZERO_ASC)) {
        if (scancodes[act_disp_term] == ZERO_ASC) clr_combo = 0;
        else clr_combo = scancodes[act_disp_term] - NUM_SCANCODE_OFFSET;
        change_text_colors(clr_combo);
        print_status_bar(); //update status bar text color info field
        send_eoi(KBD_IRQ_NUM);
        return;
    }
// #endif

    switch(scancodes[act_disp_term]) {
        case ALT_PRS:
            alt_flag = ON; //set alt flag
            break;
        case ALT_RLS:
            alt_flag = OFF; //clear alt flag
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
        case TAB:
            //if have scrolled up, scroll back down to typing location
            while(max_lines_down[act_disp_term] > 0) scroll_down();
            autocomplete();
            restore_flags(flags);
            send_eoi(KBD_IRQ_NUM);
            return;
        case K_UP:
            // reset_screen_pos();
            // int k;
            // for(k=0; k<CMD_HIST_LEN; k++)
            //     printf("%d\"%s\"\n",k,cmd_hist[act_disp_term][k]);
            if((active_command[act_disp_term] + 1) < CMD_HIST_LEN)
            {
                if(cmd_hist[act_disp_term][active_command[act_disp_term] + 1][0] == '\0')
                {
                    restore_flags(flags);
                    send_eoi(KBD_IRQ_NUM);
                    return;
                }
                uint8_t numtodelete;
                if(active_command[act_disp_term] == 0 && buff_inds == 0)
                    numtodelete = 0;
                else
                {
                    // update_cmd_hist(read_buffs[act_disp_term], active_command[act_disp_term]);
                    numtodelete = buff_inds[act_disp_term];//strlen((int8_t*)read_buffs[act_disp_term]);
                }
                active_command[act_disp_term]++;
                int i;
                for(i = 0; i < numtodelete; i++)
                {
                    print_inds[act_disp_term]--;
                    buff_inds[act_disp_term]--;
                    *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1)) = ' '; //delete character and move print index back 1 char
                    *(uint8_t *)(write_buffs[act_disp_term] + ((print_inds[act_disp_term] << 1) + 1)) &= 0xF0; //maintain background color
                    *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) |= TYPED_COLOR[color_scheme[act_disp_term]]; //clear character's attribute byte
                    if(buff_inds[act_disp_term] == 0)
                        break;
                }
                buff_inds[act_disp_term] = 0;
                type_str(cmd_hist[act_disp_term][active_command[act_disp_term]]);
                for(i = 0; i < strlen((int8_t*)cmd_hist[act_disp_term][active_command[act_disp_term]]); i++)
                {
                    read_buffs[act_disp_term][buff_inds[act_disp_term]++] = cmd_hist[act_disp_term][active_command[act_disp_term]][i];
                }
            }
            restore_flags(flags);
            send_eoi(KBD_IRQ_NUM);
            return;
        case K_DOWN:
            if(active_command[act_disp_term] > 0)
            {
                // update_cmd_hist(read_buffs[act_disp_term], active_command[act_disp_term]);
                uint8_t numtodelete = buff_inds[act_disp_term];//strlen((int8_t*)read_buffs[act_disp_term]);
                active_command[act_disp_term]--;
                int i;
                for(i = 0; i < numtodelete; i++)
                {
                    print_inds[act_disp_term]--;
                    buff_inds[act_disp_term]--;
                    *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1)) = ' '; //delete character and move print index back 1 char
                    *(uint8_t *)(write_buffs[act_disp_term] + ((print_inds[act_disp_term] << 1) + 1)) &= 0xF0; //maintain background color
                    *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) |= TYPED_COLOR[color_scheme[act_disp_term]]; //clear character's attribute byte
                    if(buff_inds[act_disp_term] == 0)
                        break;
                }
                // buff_inds[act_disp_term] = 0;
                type_str(cmd_hist[act_disp_term][active_command[act_disp_term]]);
                for(i = 0; i < strlen((int8_t*)cmd_hist[act_disp_term][active_command[act_disp_term]]); i++)
                {
                    read_buffs[act_disp_term][buff_inds[act_disp_term]++] = cmd_hist[act_disp_term][active_command[act_disp_term]][i];
                }
            }
            restore_flags(flags);
            send_eoi(KBD_IRQ_NUM);
            return;
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
                                *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) &= 0xF0; //maintain background color
                                *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) |= TYPED_COLOR[color_scheme[act_disp_term]]; //change text color (accessing attribute byte)
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
                        *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) &= 0xF0;
                        *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) |= TYPED_COLOR[color_scheme[act_disp_term]]; //clear character's attribute byte
                }
                read_buffs[act_disp_term][buff_inds[act_disp_term]] = '\0'; //remove character from buffer
            }
            update_cursor(print_inds[act_disp_term], act_disp_term);
            send_eoi(KBD_IRQ_NUM);
            restore_flags(flags);
            return;
        case PG_UP:
            scroll_up();
            send_eoi(KBD_IRQ_NUM);
            return;
        case PG_DN:
            scroll_down();
            send_eoi(KBD_IRQ_NUM);
            return;
    } /* switch end bracket */

    check_term_switch();

    // clear screen 
    if (ctrl_flag == ON && scancodes[act_disp_term] == L_KEY) { //ctrl+L
        clear();
        clear_read_buf();
        #if COLORS_ENABLED
        for(i = 0; i < STATUS_BAR_START; i++) { //reset screen to background color
            *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) &= 0x0F;
            *(uint8_t *)(write_buffs[act_disp_term] + ((i << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4);
        }
        #endif
        print_inds[act_disp_term] = 0; //reset print location to top left corner
        max_lines_up[act_disp_term] = 0; //can no longer scroll up
        max_lines_down[act_disp_term] = 0; //can no longer scroll up
        update_cursor(0, act_disp_term);
        send_eoi(KBD_IRQ_NUM);
        restore_flags(flags);
        // test_halt(0);
        return;
    }

    // print to screen and add to buffer 
    //only register characters (including enter and tab)
    if(KBD_MAP[scancodes[act_disp_term]] != 0 && scancodes[act_disp_term] != CTRL_PRS && scancodes[act_disp_term] != B_SPACE && scancodes[act_disp_term] != LSHIFT_PRS && scancodes[act_disp_term] != RSHIFT_PRS && scancodes[act_disp_term] != CAPS) { 
        //if have scrolled up, scroll back down to typing location
        while(max_lines_down[act_disp_term] > 0) scroll_down();

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
                    *(uint8_t *)(write_buffs[act_disp_term] + ((print_inds[act_disp_term] << 1) + 1)) &= 0xF0; //maintain background color
                    *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) |= TYPED_COLOR[color_scheme[act_disp_term]];
                    *(uint8_t *)(write_buffs[act_disp_term] + ((print_inds[act_disp_term] << 1) + 1)) &= 0x0F; //clear background color
                    *(uint8_t *)(write_buffs[act_disp_term] + ((print_inds[act_disp_term] << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4); //change to new background color
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
    //if have scrolled up, scroll back down to typing location
    while(max_lines_down[act_disp_term] > 0) scroll_down();

#if COLORS_ENABLED
    change_background(bg_scheme[act_disp_term]); //initialize background to default color
    change_text_colors(color_scheme[act_disp_term]); //initialize keyboard color
    print_status_bar();
#endif

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
    *(uint8_t *)(write_buffs[term_index] + ((curs_pos << 1) + 1)) = CURSOR_COLOR[color_scheme[act_disp_term]];
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
    *(uint8_t *)(video_mem + ((curs_pos << 1) + 1)) &= 0xF0; //maintain background color
    *(uint8_t *)(video_mem + ((curs_pos << 1) + 1)) = CURSOR_COLOR[color_scheme[act_disp_term]];
#if COLORS_ENABLED
    *(uint8_t *)(video_mem + ((curs_pos << 1) + 1)) &= 0x0F; //clear background color
    *(uint8_t *)(video_mem + ((curs_pos << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4); //change to new background color
#endif
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
    if(print_inds[term_index] >= STATUS_BAR_START) {
        //copy every row (except top one) in scroll up store to row above it
        for(i=0; i<(NUM_ROWS-1); i++) {
            memcpy(scroll_stores[term_index][UP]+(CHAR_DIS_SZ*i*NUM_COLS), scroll_stores[term_index][UP]+(2*(i+1)*NUM_COLS), CHAR_DIS_SZ*NUM_COLS);
        }
        // copy top row (which disappears) to lowest row of up scroll store
        memcpy(scroll_stores[term_index][UP]+(CHAR_DIS_SZ*(NUM_ROWS-1)*NUM_COLS), write_buffs[term_index], CHAR_DIS_SZ*NUM_COLS);

        // copy every row to the row above it 
        for(i=0; i<(NUM_ROWS-1); i++)
            memcpy(write_buffs[term_index]+(2*i*NUM_COLS), write_buffs[term_index]+(2*(i+1)*NUM_COLS), 2*NUM_COLS);
         // clear newly inserted line 
        for(i=0; i < NUM_COLS; i++) {
            *(uint8_t *)(write_buffs[term_index] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + (i << 1)) = ' ';
            *(uint8_t *)(write_buffs[term_index] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + (i << 1) + 1) &= 0xF0; //maintain background color
            *(uint8_t *)(write_buffs[term_index] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + (i << 1) + 1) |= TYPED_COLOR[color_scheme[term_index]];
        #if COLORS_ENABLED
            *(uint8_t *)(write_buffs[term_index] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + ((i << 1) + 1)) &= 0x0F; //clear background color
            *(uint8_t *)(write_buffs[term_index] + ((NUM_COLS*(NUM_ROWS-1)) << 1) + ((i << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[term_index]] << 4); //change to new background color
        #endif
        }
        // begin printing at left-most position of lowest row 
        print_inds[term_index] -= NUM_COLS;
        if(max_lines_up[term_index] < NUM_ROWS) max_lines_up[term_index]++; //increment number of lines can scroll up
    }
    restore_flags(flags);
}

 /*
  * scroll_up()
  *   DESCRIPTION: scroll up 1 line to view the last line that disappeared from the top of the terminal
  *   INPUTS: none
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: terminal scrolls up 1 line
  */
void scroll_up()
{
    uint32_t i; //iterator

    if(max_lines_up[act_disp_term] == 0) return; //haven't scrolled down so can't scroll up

    //move scroll down store rows down
    for(i=NUM_ROWS-1; i>0; i--) {
        memcpy(scroll_stores[act_disp_term][DN]+(CHAR_DIS_SZ*i*NUM_COLS), scroll_stores[act_disp_term][DN]+(CHAR_DIS_SZ*(i-1)*NUM_COLS), CHAR_DIS_SZ*NUM_COLS);
    }
    //copy lowest row of terminal to top row of down scroll store
    memcpy(scroll_stores[act_disp_term][DN], write_buffs[act_disp_term]+(CHAR_DIS_SZ*(NUM_ROWS-1)*NUM_COLS), CHAR_DIS_SZ*NUM_COLS);
    //move every terminal row down
    for(i=NUM_ROWS-1; i>0; i--) {
        memcpy(write_buffs[act_disp_term]+(CHAR_DIS_SZ*i*NUM_COLS), write_buffs[act_disp_term]+(CHAR_DIS_SZ*(i-1)*NUM_COLS), CHAR_DIS_SZ*NUM_COLS);
    }
    //copy lowest row of up scroll store to top row of terminal
    memcpy(write_buffs[act_disp_term], scroll_stores[act_disp_term][UP]+(CHAR_DIS_SZ*(NUM_ROWS-1)*NUM_COLS), CHAR_DIS_SZ*NUM_COLS);
    //move scroll up store rows down
    for(i=NUM_ROWS-1; i>0; i--) {
        memcpy(scroll_stores[act_disp_term][UP]+(CHAR_DIS_SZ*i*NUM_COLS), scroll_stores[act_disp_term][UP]+(CHAR_DIS_SZ*(i-1)*NUM_COLS), CHAR_DIS_SZ*NUM_COLS);
    }

    //set background color of new line
    for(i=0; i<NUM_COLS; i++) {
        *(uint8_t *)(write_buffs[act_disp_term] + (i << 1) + 1) &= 0x0F;
        *(uint8_t *)(write_buffs[act_disp_term] + (i << 1) + 1) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4);
    }

    max_lines_up[act_disp_term]--;
    if(max_lines_down[act_disp_term] < NUM_ROWS) max_lines_down[act_disp_term]++; //increment number of lines can scroll down

    remove_hw_cursor(); //remove cursor while scrolling up

    return;
}

  /*
  * scroll_down()
  *   DESCRIPTION: scroll down 1 line to view the last line that disappeared from the bottom of the terminal;
                   can only scroll down after scrolling up
  *   INPUTS: none
  *   OUTPUTS: none
  *   RETURN VALUE: none
  *   SIDE EFFECTS: terminal scrolls down 1 line
  */
void scroll_down()
{
    uint32_t i; //iterator

    if(max_lines_down[act_disp_term] == 0) {
        update_cursor(print_inds[act_disp_term], act_disp_term); //redisplay cursor
        return; //haven't scrolled up so can't scroll down
    }

    //move scroll up store rows up
    for(i=0; i<(NUM_ROWS-1); i++) {
        memcpy(scroll_stores[act_disp_term][UP]+(CHAR_DIS_SZ*i*NUM_COLS), scroll_stores[act_disp_term][UP]+(CHAR_DIS_SZ*(i+1)*NUM_COLS), CHAR_DIS_SZ*NUM_COLS);
    }
    //copy top row of terminal to lowest row of up scroll store
    memcpy(scroll_stores[act_disp_term][UP]+(CHAR_DIS_SZ*(NUM_ROWS-1)*NUM_COLS), write_buffs[act_disp_term], CHAR_DIS_SZ*NUM_COLS);
    //move every terminal row up
    for(i=0; i<(NUM_ROWS-1); i++) {
        memcpy(write_buffs[act_disp_term]+(CHAR_DIS_SZ*i*NUM_COLS), write_buffs[act_disp_term]+(CHAR_DIS_SZ*(i+1)*NUM_COLS), CHAR_DIS_SZ*NUM_COLS);
    }
    //copy top row of down scroll store to bottom row of terminal
    memcpy(write_buffs[act_disp_term]+(CHAR_DIS_SZ*(NUM_ROWS-1)*NUM_COLS), scroll_stores[act_disp_term][DN], CHAR_DIS_SZ*NUM_COLS);
    //move scroll down store rows up
    for(i=0; i<(NUM_ROWS-1); i++) {
        memcpy(scroll_stores[act_disp_term][DN]+(CHAR_DIS_SZ*i*NUM_COLS), scroll_stores[act_disp_term][DN]+(CHAR_DIS_SZ*(i+1)*NUM_COLS), CHAR_DIS_SZ*NUM_COLS);
    }

    //set background color of new line
    for(i=0; i<NUM_COLS; i++) {
        *(uint8_t *)(write_buffs[act_disp_term] + (CHAR_DIS_SZ*(NUM_ROWS-1)*NUM_COLS) + (i << 1) + 1) &= 0x0F;
        *(uint8_t *)(write_buffs[act_disp_term] + (CHAR_DIS_SZ*(NUM_ROWS-1)*NUM_COLS) + (i << 1) + 1) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4);
    }

    max_lines_down[act_disp_term]--;
    if(max_lines_down[act_disp_term] < NUM_ROWS) max_lines_up[act_disp_term]++; //increment number of lines can scroll up
    
    return;
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
    //Save command
    if(read_buffs[act_ops_term][0] != '\0')
        ins_cmd_hist(read_buffs[act_ops_term]);
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
    int i, j; //loop iterator
    int32_t flags;
    char* buf = (char*) wrt_buf;
    if(buf[0] == '\0') return 0;
    cli_and_save(flags); //clear interrupts to ensure buffer correctly printed to screen
    for(i = 0; i < bytes; i++) {
        switch(buf[i]) {
            case '\0': break;
            case TAB_ASC:
                #if COLORS_ENABLED
                for(j = print_inds[act_ops_term]; j < TAB_LEN; j++) {
                    *(uint8_t *)(write_buffs[act_ops_term] + (((print_inds[act_ops_term] + j) << 1) + 1)) &= 0x0F; //clear background color
                    *(uint8_t *)(write_buffs[act_ops_term] + (((print_inds[act_ops_term] + j) << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_ops_term]] << 4); //change to new background color
                }
                #endif
                print_inds[act_ops_term] += TAB_LEN; //add 5 spaces/tab
                break;
            case ENT_ASC:
            #if COLORS_ENABLED
                for(j = 0; j < (NUM_COLS - (print_inds[act_ops_term] % NUM_COLS)); j++) {
                    *(uint8_t *)(write_buffs[act_ops_term] + (((print_inds[act_ops_term] + j) << 1) + 1)) &= 0x0F; //clear background color
                    *(uint8_t *)(write_buffs[act_ops_term] + (((print_inds[act_ops_term] + j) << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_ops_term]] << 4); //change to new background color
                }
            #endif
                print_inds[act_ops_term] += NUM_COLS - (print_inds[act_ops_term] % NUM_COLS);
                break;
            default: //for regular characters (only increment print index)
                *(uint8_t *)(write_buffs[act_ops_term] + (print_inds[act_ops_term] << 1)) = buf[i]; // "<< 1" because each character is 2 bytes
                *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) &= 0xF0; //maintain background color
                *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) = PRINTED_COLOR[color_scheme[act_ops_term]];
            #if COLORS_ENABLED
                *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) &= 0x0F;
                *(uint8_t *)(write_buffs[act_ops_term] + ((print_inds[act_ops_term] << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_ops_term]] << 4);
            #endif
                print_inds[act_ops_term]++;
        }
        check_scroll(act_ops_term);
    }
    update_cursor(print_inds[act_ops_term], act_ops_term);
    restore_flags(flags);
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
    if(max_lines_down[act_disp_term] == 0) update_cursor(print_inds[act_disp_term], act_disp_term); //update cursor with new color
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
    itoa(get_num_processes(), processes_char, (int32_t) 10);
    // itoa(1, processes_char, (int32_t) 10);
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
* remove_hw_cursor() 
*   DESCRIPTION: Delete cursor by printing it off-screen
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: cursor gone
*/
void remove_hw_cursor()
{
    outb(0x0F, VGA_LOW);
    outb((unsigned char)((STATUS_BAR_END+1) & 0xFF), VGA_HIGH);
    outb(0x0E, VGA_LOW);
    outb((unsigned char)(((STATUS_BAR_END+1) >> 8) & 0xFF), VGA_HIGH);
}

/*
* autocomplete
*   DESCRIPTION: implements tab completion of incomplete expressions
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: changes terminal display, read buffer, and buffer size
*/
void autocomplete()
{
    //Can't do anything with blank buffer.  Otherwise implements ls
    if(buff_inds[act_disp_term] == 0)
        return;
    //Find the last word in the buffer string
    uint8_t* compstr = get_prev_word((uint8_t) buff_inds[act_disp_term]);
    //Check to make sure a word was found
    if(compstr == NULL)
        return;
    int i;
    int nummatch = 0;
    uint8_t* matches[NUM_FILES] = {NULL};
    uint8_t match;
    //Iterate through all possible complete file names to find
    //possible matches
    for(i = 0; i < NUM_FILES; i++)
    {
        //check partial string identicallness
        match = partial_strcmp(compstr, (uint8_t*)FILES[i]);
        //When potential match is found, add it to
        //array of potential matches and increment
        //the match count.
        if(match == 1)
        {
            matches[nummatch] = (uint8_t*)FILES[i];
            nummatch++;
        }
    }
    //No matches found, do nothing.
    if(nummatch == 0)
        return;
    //Multiple matches found.  Dumb completion is dumb.
    //Gives up and spits out possible solutions without
    //forcing user to do anything.
    else if(nummatch > 1)
    {
        //Print out all possible matches
        uint32_t prev_act_ops_term = act_ops_term;
        act_ops_term = act_disp_term;
        for(i = 0; i < nummatch; i++)
        {
            term_write((void*)"\n",1);
            term_write((void*)matches[i], strlen((int8_t*)matches[i]));
        }
        //Make terminal header come back
        //Disabled since tab complete works everywhere, not just shell
        // term_write((void*)"\n391OS> ",8);
        //Re-type previously typed characters on new line
        term_write((void*)"\n",1);
        type_str(read_buffs[act_ops_term]);
        act_ops_term = prev_act_ops_term;
        return;
    }
    //Only one potential match found, so we'll assume
    //user really wants that one!  Complete that bad boy.
    else if(nummatch == 1)
    {
        i = strlen((int8_t*)compstr);
        //Only you can prevent buffer overflow! -Smokey
        if(strlen((int8_t*)&(matches[0][i])) > (BUF_SIZE - buff_inds[act_disp_term]))
            return;
        //Type out remaining characters in matched string
        type_str(&(matches[0][i]));
        //Add to buffer
        int j;
        for(j = 0; j < strlen((int8_t*)&(matches[0][i])); j++)
            read_buffs[act_disp_term][buff_inds[act_disp_term]++] = matches[0][i+j];
        return;
    }
}


/*
* get_prev_word
*   DESCRIPTION: find last word in string.  Recursively searches
*                from end of string.  Recursion may not be most
*                efficient (haven't analyzed), but I feel smart when
*                I can actually write a working recursive function.
*   INPUTS: end index of buffer to begin reverse traversal from
*   OUTPUTS: none
*   RETURN VALUE: returns pointer to beginning of last word string,
*                 NULL on error or no string found.
*   SIDE EFFECTS: none
*/
uint8_t* get_prev_word(uint8_t endindex)
{
    //Check for invalid
    if(endindex >= BUF_SIZE)
        return NULL;
    //If reach beginning of buffer, that must be the beginning of word
    if(endindex == 0)
        return &read_buffs[act_disp_term][endindex];\
    if(read_buffs[act_disp_term][endindex] == ' ')
        return &read_buffs[act_disp_term][endindex + 1];
    else
        return get_prev_word(endindex - 1);
}


/*
* partial_strcmp
*   DESCRIPTION: determines if string partial is a match for
*                the beginning of string full.  Only successfull
*                iff partial is a substring of full.
*   INPUTS: partial:    pointer to a string user wishes to see
*                       if it's the beginning of full
*           full:   pointer to a string of the full string that
*                   may have a match.
*   OUTPUTS: none
*   RETURN VALUE: returns 1 for a match, 0 for no match
*   SIDE EFFECTS: none.
*/
uint8_t partial_strcmp(uint8_t* partial, uint8_t* full)
{
    int i = 0;
    //Loop through all characters in partial string
    //to check if identical to full string
    while(partial[i] != '\0')
    {
        //If full string terminates prematurely, no match
        if(full[i] == '\0')
            return 0;
        //If characters don't match, no match
        if(partial[i] != full[i])
            return 0;
        i++;
    }
    //partial is a substring of full that starts at the beginning
    //of full, so return match.
    return 1;
}

/*
* type_char
*   DESCRIPTION: helper function to write user-colored text to the
*                terminal.  Writes a single character at the next
*                appropriate position.
*   INPUTS: input:  The character to write.
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: Writes a character to the terminal.  Cannot fail,
*                 but may have to scroll terminal view to make space.
*/
void type_char(uint8_t input)
{
    *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1)) = input;
    *(uint8_t *)(write_buffs[act_disp_term] + ((print_inds[act_disp_term] << 1) + 1)) &= 0xF0; //maintain background color
    *(uint8_t *)(write_buffs[act_disp_term] + (print_inds[act_disp_term] << 1) + 1) |= TYPED_COLOR[color_scheme[act_disp_term]];
    *(uint8_t *)(write_buffs[act_disp_term] + ((print_inds[act_disp_term] << 1) + 1)) &= 0x0F; //clear background color
    *(uint8_t *)(write_buffs[act_disp_term] + ((print_inds[act_disp_term] << 1) + 1)) |= (BACKGROUND_COLOR[bg_scheme[act_disp_term]] << 4); //change to new background color
    if(input != '\0')
        print_inds[act_disp_term]++;
    check_scroll(act_disp_term);
    update_cursor(print_inds[act_disp_term], act_disp_term);
}

/*
* type_str
*   DESCRIPTION: Encapsulation of type_char to write an
*                entire null-terminated string to the terminal
*                in user-colored text.
*   INPUTS: input: pointer to the string to write.
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: Writes a string to the terminal.  May have
*                 to scroll terminal view to make space.
*/
void type_str(uint8_t* input)
{
    int i = 0;
    if(input == NULL)
        return;
    while(input[i] != '\0')
    {
        type_char(input[i]);
        i++;
    }
    type_char('\0');
}


/*
* ins_cmd_hist
*   DESCRIPTION: Inserts a command into the command history array
*                and advances the array like a queue.
*   INPUTS: input: string to insert as most recent command
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: completely changes the command history array
*                 queue.
*/
void ins_cmd_hist(uint8_t* input)
{
    if(input == NULL)
        return;
#if CMD_HIST_LEN < 2
    return;
#else
    newline_to_null(input);
    strcpy((int8_t*)cmd_hist[act_disp_term][0], (int8_t*)input);
    int i;
    for(i = (CMD_HIST_LEN - 2); i >= 0; i--)
    {
        strcpy((int8_t*)cmd_hist[act_disp_term][i+1], (int8_t*)cmd_hist[act_disp_term][i]);
    }
    active_command[act_disp_term] = 0;
    cmd_hist[act_disp_term][0][0] = '\0';
#endif
}


/*
* newline_to_null
*   DESCRIPTION: Converts the first newline in a string to a
*                null character, unless a null character
*                terminates the string first.
*   INPUTS: input: string to mess arround with
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: modifies string provided.
*/
void newline_to_null(uint8_t* input)
{
    if(input == NULL)
        return;
    int i = 0;
    while(input[i] != '\n' && input[i] != '\0') i++;
    input[i] = '\0';
}
