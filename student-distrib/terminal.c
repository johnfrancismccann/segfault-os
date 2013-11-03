/*
 *  The terminal files allow the terminal to read and write to/from user/kernel space.
 */

#include "terminal.h"
#include "keyboard.h"
#include "keyboard_asm.h"
#include "setup_idt.h"
#include "i8259.h"
#include "lib.h"
#include "types.h"

/*
 * term_open()
 *   DESCRIPTION: Initialize terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t term_open() {
	
	return 0; //indicates successful open
}

/*
 * term_close()
 *   DESCRIPTION: Close terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t term_close() {
	
	return 0; //indicates successful close
}

/*
 * term_read()
 *   DESCRIPTION: Copies buffer from kernel space (read_buf) to user space (pointer).
 *   INPUTS: buffer where character buffer form keyboard will be copied
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t term_read(void* pointer) {	
	int bytes_copied;

	bytes_copied = get_read_buf(pointer);

	// void* read_buffer = (void*) get_read_buf(buf);
	// if(read_buffer == -1) return -1; //indicates nothing has been typed
	clear_read_buf();

    //memcpy(buf, (void*) read_buffer, buf_idx+1);
    //puts(pointer);
    return bytes_copied;
}


/*
 * term_write()
 *   DESCRIPTION: Copies buffer from user space to kernel space.
 *   INPUTS: buffer to be copied, number of bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void term_write(void* buf, int32_t n) {


}
