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

	master_mask = 0xFF;
	slave_mask = 0xFF;

    //Manual delay function
    __asm__("MOVL $50, %%eax \n\t"
            "_wait_loop_: DECL %%eax \n\t"
            "JGE _wait_loop_ \n\t"
            :
            :
            : "eax");

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
        outb(master_mask, MASTER_8259_PORT + 1);
	}
	else if(irq_num >= 8 && irq_num < 16)
	{
		slave_mask &= ~(0x1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_PORT + 1);
	}
}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
	if(irq_num >= 0 && irq_num < 8)
	{
		master_mask |= (0x1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 1);
	}
	else if(irq_num >= 8 && irq_num < 16)
	{
		slave_mask |= (0x1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_PORT + 1);
	}
}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
    if(irq_num > 15)
        return;
    else if(irq_num < 8)
    {
        //Send EOI OR'd with irq number to both PICs
        outb(EOI | irq_num, MASTER_8259_PORT);
        __asm__("MOVL $50, %%eax \n\t"
                "_wait_loop_2: DECL %%eax \n\t"
                "JGE _wait_loop_2 \n\t"
                :
                :
                : "eax");
    }
    else
    {
        //Send EOI OR'd with irq number to both PICs
        outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
        __asm__("MOVL $50, %%eax \n\t"
                "_wait_loop_3: DECL %%eax \n\t"
                "JGE _wait_loop_3 \n\t"
                :
                :
                : "eax");

        outb(EOI | 0x2, MASTER_8259_PORT);
        __asm__("MOVL $50, %%eax \n\t"
                "_wait_loop_4: DECL %%eax \n\t"
                "JGE _wait_loop_4 \n\t"
                :
                :
                : "eax");
    }
}

/* block_hw_interrupts:
 *   DESCRIPTION: Masks all HW interrupts and saves for the case of
 *                restoring HW interrupts.  Like cli with save flags.
 *   INPUTS: slave_backup:  pointer to location where mask can be saved
 *                          for slave PIC.  On NULL, doesn't save.
 *           master_backup: pointer to location where mask can be saved
 *                          for master PIC.  On NULL, doesn't save.
 *   OUTPUTS: Outputs to pointers in INPUTS.
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Masks all IRQs on both PICs and saves masks before
 *                 changing the masks.
 */

void block_hw_interrupts(uint8_t* slave_backup, uint8_t* master_backup)
{
    if(slave_backup != NULL)    //Only run if not NULL.
                                //NOTE: change to access check eventually
    {
        *slave_backup = slave_mask; //Save previous Mask
    }
    slave_mask = 0xFF;          //Set new mask
    outb(0xFF, SLAVE_8259_PORT + 1);    //Mask Slave Interrupts

    if(master_backup != NULL)    //Only run if not NULL.
                                 //NOTE: change to access check eventually
    {
        *master_backup = master_mask; //Save previous Mask
    }
    master_mask = 0xFF;           //Set new mask
    outb(0xFF, MASTER_8259_PORT + 1);    //Mask Master Interrupts
}

 /* restore_hw_interrupts:
 *   DESCRIPTION: Restores HW interrupt masks saved by block_hw_interrupts.
 *                Like restore flags function.
 *   INPUTS: slave_backup:  pointer to location where slave PIC mask is
 *                          saved.  On NULL, slave in not restored.
 *           master_backup: pointer to location where master PIC mask is
 *                          saved.  On NULL, master is not restored.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: May mask or unmask IRQs on both PICs.
 */
void restore_hw_interrupts(uint8_t* slave_backup, uint8_t* master_backup)
{
    if(slave_backup != NULL)    //Only run if not NULL.
                                //NOTE: change to access check eventually
    {
        slave_mask = *slave_backup; //Restore Mask
        outb(slave_mask, SLAVE_8259_PORT + 1);    //Mask Slave Interrupts
    }
    
    if(master_backup != NULL)    //Only run if not NULL.
                                 //NOTE: change to access check eventually
    {
        master_mask = *master_backup; //Restore Mask
        outb(master_mask, MASTER_8259_PORT + 1);    //Mask Master Interrupts
    }
}
