/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void
i8259_init(void)
{
	outb(0xFF, MASTER_8259_PORT + 1);	//Mask Master Interrupts
	outb(0xFF, SLAVE_8259_PORT + 1);	//Mask Slave Interrupts

	//Setup Master PIC
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_PORT + 1); //Map IRs to 0x20-0x27
	outb(ICW3_MASTER, MASTER_8259_PORT + 1); //Tell it it has a slave
	outb(ICW4, MASTER_8259_PORT + 1);		 //Normal EOI (not auto)

	//Setup Slave PIC
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);	//Map IRs to 0x28-0x2F
	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);	//Tell it it is a slave on IR2
	outb(ICW4, SLAVE_8259_PORT + 1);		//Normal EOI (not auto)

	outb(master_mask, MASTER_8259_PORT + 1); //Restore Interrupt mask for master
    outb(slave_mask, SLAVE_8259_PORT + 1);   //Restore Interrupt mask for slave
}

/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num)
{
	if(irq_num >= 0 && irq_num < 8)
	{
		master_mask &= ~(0x1 << irq_num);
	}
	else if(irq_num >= 8 && irq_num < 16)
	{
		slave_mask &= ~(0x1 << (irq_num - 8));
	}
}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
	if(irq_num >= 0 && irq_num < 8)
	{
		master_mask |= (0x1 << irq_num);
	}
	else if(irq_num >= 8 && irq_num < 16)
	{
		slave_mask |= (0x1 << (irq_num - 16));
	}
}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
    //Send EOI OR'd with irq number to both PICs
    outb(EOI | irq_num, MASTER_8259_PORT);
    outb(EOI | irq_num, SLAVE_8259_PORT);
}

