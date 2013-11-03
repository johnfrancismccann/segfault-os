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
void term_read(void* buf, unsigned long n){

    //memcpy( , buf, n)
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
