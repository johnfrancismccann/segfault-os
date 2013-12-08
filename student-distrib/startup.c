/*
 *  The startup files allow for printing a start-up screen upon starting the OS.
 */

#include "lib.h"
#include "types.h"
#include "rtc.h"
#include "keyboard.h"


uint16_t i, j; //loop iterators
static uint8_t* video_mem = (uint8_t *)VIDEO;
char start_screen[NUM_ROWS+1][NUM_COLS]; //25 rows of 80 columns each

/*
* print_start_screen()
*   DESCRIPTION: Display start screen on terminal
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: start screen temporarily on screen
*/
void print_start_screen() {

    /* start screen */
    //change background color
    remove_hw_cursor();

    strcpy(start_screen[0], "");
    strcpy(start_screen[1], "              ____             _____           _ _      ___  ____  ");
    strcpy(start_screen[2], "             / ___|  ___  __ _|  ___|_ _ _   _| | |_   / _ \\/ ___| ");
    strcpy(start_screen[3], "             \\___ \\ / _ \\/ _` | |_ / _` | | | | | __| | | | \\___ \\ ");
    strcpy(start_screen[4], "              ___) |  __/ (_| |  _| (_| | |_| | | |_  | |_| |___) |");
    strcpy(start_screen[5], "             |____/ \\___|\\__, |_|  \\__,_|\\__,_|_|\\__|  \\___/|____/ ");
    strcpy(start_screen[6], "                         |___/                                     ");
    strcpy(start_screen[7], "");
    strcpy(start_screen[8], "");
    strcpy(start_screen[9], "");
    strcpy(start_screen[10], "                                       ,--,_                    ");
    strcpy(start_screen[11], "                                __    _\\.---'-.                    ");
    strcpy(start_screen[12], "                                \\ '.-\"     // o\\                    ");
    strcpy(start_screen[13], "                                /_.'-._    \\   /                    ");
    strcpy(start_screen[14], "                                       `\"--(/\"`                    ");
    strcpy(start_screen[15], "                                     _,--,                    ");
    strcpy(start_screen[16], "                                  .-'---./_    __                    ");
    strcpy(start_screen[17], "                                 /o \\     \"-.'  /                    ");
    strcpy(start_screen[18], "                                 \\  //    _.-'._\\                    ");
    strcpy(start_screen[19], "                                  `\"\\)--\"`                    ");
    strcpy(start_screen[20], "");
    strcpy(start_screen[21], "");
    strcpy(start_screen[22], "");
    strcpy(start_screen[23], "       Christopher Maier      David Turner      John McCann      Wenyu Wu       ");
    strcpy(start_screen[24], "");



    for(i = 0; i <= STATUS_BAR_END; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' '; //clear screen
        *(uint8_t *)(video_mem + ((i << 1) + 1)) &= 0xF0; //maintain background color
        *(uint8_t *)(video_mem + (i << 1) + 1) = WHITE;
        *(uint8_t *)(video_mem + ((i << 1) + 1)) &= 0x0F; //clear background color
        *(uint8_t *)(video_mem + ((i << 1) + 1)) |= (BLUE << 4);
    }

    //print characters
    for(i = 0; i <= NUM_ROWS; i++) {
        for(j = 0; j < NUM_COLS; j++) {
            *(uint8_t *)(video_mem + ((i*NUM_COLS+j) << 1)) = start_screen[i][j];
        }
    }

    for(i = 0; i < 5; i++) rtc_read(0, 0);

}
