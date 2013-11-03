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
 *   OUTPUTS: 0 on success, -1 on fail
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
 *   OUTPUTS: 0 on success, -1 on fail
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t term_close() {
	
	return 0; //indicates successful close

}

/*
 * term_read()
 *   DESCRIPTION: Copies buffer from kernel space (terminal) to user space (read_tr).
 *   INPUTS: pointer to which character buffer form keyboard will be copied
 *   OUTPUTS: number of bytes read in
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t term_read(void* read_ptr) {	
	int bytes_copied;

	bytes_copied = get_read_buf(read_ptr);

	clear_read_buf();

    return bytes_copied;
}

/*
 * term_write()
 *   DESCRIPTION: Copies buffer from user space (wrt_ptr) to kernel space (terminal).
 *   INPUTS: pointer to buffer to be written
 *   OUTPUTS: number of bytes written
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t term_write(const void* wrt_ptr, int32_t nbytes) {
	int32_t bytes_copied;
	cli(); //disable interrupts so input printed cleanly
	bytes_copied = print_write_buf(wrt_ptr, nbytes);
	sti(); //re-enable interrupts
	return bytes_copied;
}
