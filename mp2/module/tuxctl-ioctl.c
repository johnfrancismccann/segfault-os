/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

//convert number and decimal point info to format usable by tux
int to_ssd(int num, int dp);

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
int flag = 0; //indicates which ldisc_put call is executing/just executed
char opc; //opcode to be passed to ldisc_put
unsigned bioc_byte1, bioc_byte2; //return packets from button presses
static spinlock_t slock = SPIN_LOCK_UNLOCKED;
unsigned long flags;
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	
	spin_lock_irqsave(&slock, flags);

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];
	
	// done resetting
	if(a == MTCP_RESET) {
		flag = 1;
		opc = MTCP_BIOC_ON; //button enable interrupt, returns MTCP_ACK
		tuxctl_ldisc_put(tty, &opc, 1);
	}
	// done enabling button interrupt after resetting
	if(a == MTCP_ACK && flag == 1) {
		flag = 0;
		opc = MTCP_LED_USR; //allow user to set LEDs
		tuxctl_ldisc_put(tty, &opc, 1);
	}
	if(a == MTCP_BIOC_EVENT) {
		bioc_byte1 = b;
		bioc_byte2 = c;
	}

	spin_unlock_irqrestore(&slock, flags);
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
	char opc; //opcode to be sent to line discipline
	char num_buf[6]; //buffer to store number in format for tux
   	int num, dp_on, led_on; //number to be displayed, decimal points to be on, LEDs to be on
   	int ones, tens, hund, thou; //each digit to be output to SSD
 	int num0, num1, num2, num3; //individual decimal number output to SSD
	int i; //for loop iterators and temporary variables
	int mask; //mask to put correct digits in buffer for ldisc_put
	int down, left; //for directional button booleans
	unsigned long int buttons;
	int new_bioc_byte2;
	
	spin_lock_irqsave(&slock, flags);
	switch (cmd) {
    	case TUX_INIT:
			opc = MTCP_RESET_DEV; //reset the device, generates MTCP_RESET event when finished
			tuxctl_ldisc_put(tty, &opc, 1);
			spin_lock_init(&slock); //initialize spin lock
    		break;
    
    	case TUX_BUTTONS:
			down = 0;
			left = 0;
			new_bioc_byte2 = bioc_byte2 & MSK0;
			if (!access_ok(VERIFY_READ, arg, 4)) return -EINVAL; //invalid pointer
			//combine BIOC_EVENT outputs and get rid of middle 2 bits of directional byte since different
			if((new_bioc_byte2 & 0x04) == 0) down = 1; //down button pressed
			if((new_bioc_byte2 & 0x02) == 0) left = 1; //left button pressed
			if((left == 1) && (down == 1)) new_bioc_byte2 |= 0x06;
			else if((left == 0) && (down == 0)) new_bioc_byte2 |= 0x06;
			else if(left == 1 && down == 0) new_bioc_byte2 = 0xB;
			else if(left == 0 && down == 1) new_bioc_byte2 = 0xD;
			buttons = (bioc_byte1 & 0xF) + ((new_bioc_byte2 & 0xF) << 4);
			copy_to_user((unsigned long int*) arg, &buttons, 1); //8 bits long
    		break;
    
    	/* arg[27:24] specifies which decimal points to turn on
    	 * arg[19:16] specifies which LEDs to turn on
    	 * arg[15:0] specifies the number that will be displayed on the 7-segment display (currently in hex)
    	*/
    	case TUX_SET_LED:
    		num = arg & MSK3; //get lowest 16 bits (number to display)
    		ones = num % 10; //to be displayed in ones place on SSD
    		tens = (num % 100) / 10; //tens place
    		hund = (num % 1000) / 100; //hundreds place
    		thou = (num % 10000) / 1000; //thousands place
    		
    		dp_on = arg >> 24; //want bits 27-24, so shift right 24
    		dp_on &= MSK0; //get lowest 4 bits
    
			//convert numbers (and decimal places) to seven segment display formats
    		num3 = to_ssd(thou, dp_on & 8); //thousands place
    		num2 = to_ssd(hund, dp_on & 4);
    		num1 = to_ssd(tens, dp_on & 2);
    		num0 = to_ssd(ones, dp_on & 1); //ones place
    
    		led_on = arg >> 16; //want bits 19-16
    		led_on &= MSK0;
    	
    		//led_on is byte 0, led3 is first byte after that, led2 next one, etc.
			num_buf[0] = num0;
			num_buf[1] = num1;
			num_buf[2] = num2;
			num_buf[3] = num3;
			opc = MTCP_LED_SET;
			tuxctl_ldisc_put(tty, &opc, 1);
			opc = led_on;
			tuxctl_ldisc_put(tty, &opc, 1);
			for(i = 0; i < 4; i++) {//maximum of 4 LEDs to be written
				switch(i) { //set bit masks to write to correct LED
					case 0:
						mask = 1;
						break;
					case 1:
						mask = 2;
						break;
					case 2:
						mask = 4;
						break;
					case 3:
						mask = 8;
						break;
					default:
						mask = 15;						
				}
				if((led_on & mask) > 0)
					tuxctl_ldisc_put(tty, &num_buf[i], 1);
			}
    		break;

    	default:
    	    return -EINVAL; //Linux invalid argument error code
    }
	spin_unlock_irqrestore(&slock, flags);
	return 0;
}

/* to_ssd
 * DESCRIPTION: find 8-bit value for MTCP_LED_SET to write number to 7 seg display	
 * INPUTS: number in decimal to be written, 1 or 0 to determine whether decimal point following number is on/off
 * OUTPUTS: 8-bit int for MTCP_LED_SET
 * SIDE EFFECTS: none
 */
int
to_ssd(int num, int dp) {
	int retval; //8-bit value to return
	switch(num) { //assuming decimal point off
		case 0:
			retval = 0xE7;	
			break;
		case 1:
			retval = 0x06;
			break;
		case 2:
			retval = 0xCB;
			break;
		case 3:
			retval = 0x8F;	
			break;
		case 4:
			retval = 0x2E;	
			break;
		case 5:
			retval = 0xAD;	
			break;
		case 6:
			retval = 0xED;	
			break;
		case 7:
			retval = 0x86;	
			break;
		case 8:
			retval = 0xEF;	
			break;
		case 9:
			retval = 0xAF;	
			break;
		default:
			retval = 0x00;
	}
	if (dp)
		retval |= 0x10; //set dp bit to 1;
	return retval;
}
