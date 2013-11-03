/*
 *  DON'T FORGET COMMENTS
 */

#include "keyboard.h"
#include "keyboard_asm.h"
#include "setup_idt.h"
#include "i8259.h"
#include "lib.h"
#include "types.h"

/*
 * term_read()
 *   DESCRIPTION: Copies buffer from kernel space to user space.
 *   INPUTS: buffer to be copied, number of bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void term_read(char* read_buffer, void* buf, unsigned long n){
	// if(buf == NULL) return; //make sure valid pointer

	// if(read_buf[buf_idx-1] == ENT_ASC) {
 //    	memcpy(buf, (void*) read_buffer, n);
	// }
}


/*
 * term_write()
 *   DESCRIPTION: Copies buffer from user space to kernel space.
 *   INPUTS: buffer to be copied, number of bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void term_write(void* buf, unsigned long n){


}
