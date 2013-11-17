#include "task.h"

//all the definitions
#define TASK_ADD 0x08048000 

//From MP3 spec:
/*
 *First user-level program should be loaded at physical 8MB
 *Second one should at 12MB
 *program image itself is linked to execute at virtual address TASK_ADD
 *
 */

#define EIP_START 24
#define EIP_SIZE 4
 
/*
 * task_execute
 *  DESCRIPTION: execute the task 
 *  INPUTS: size: cur_task: the tusk to be executed
 *  OUTPUTS: none
 *  RETURN VALUE: 1 on success; 
 *               -1 on failure
 *  SIDE EFFECTS: modify contents in TSS structure
 */
int task_execute(){
    //call the loader function first
	uint8_t buf[5000];//CHANGE THIS!!
	//call_loader(buf);

	uint8_t _24_to_27[EIP_SIZE];
    int i;//iterator
	//copy the new EIP value to the temp buffer
	for (i = 0; i < EIP_SIZE; i++)
        _24_to_27[i] = buf[EIP_START+i];
		
	uint8_t* new_eip = _24_to_27;
	
	//modify some values according to the first 40 bytes if the file
    //thigs to set: EIP,CS,EFLAGS,ESP,SS	
    //modify TSS	
	tss.eip = (uint32_t)new_eip;
	//tss.esp = ;
	//tss.ss = ;//I don't know where these values come from
	
	//do the system call after everything is set up
//    if (sys_execute() == -1)
//	    return -1;//kill the task
	
	//return success if no exceptions
	return 1;
}

/*
 * switch_to_user_mode
 *  DESCRIPTION: push values to the stack, and execure IRET at last 
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none?
 */
void switch_to_user_mode()
{
    // Set up a stack structure for switching to user mode.
    asm volatile(
                //data segment selector is (0x20 | 0x3 = 0x23)
                //0x20: User data segment, and 0x3 is setting RPL to 3				
                "cli\n\t"  
                "mov $0x23, %%ax\n\t" 
                "mov %%ax, %%ds\n\t" 
                "mov %%ax, %%es\n\t" 
	            "mov %%ax, %%fs\n\t" 
                "mov %%ax, %%gs\n\t" 

                "mov %%esp, %%eax\n\t" 
                "pushl $0x23\n\t"
                "pushl %%eax\n\t" 
                "pushf\n\t"
				"pop %%eax\n\t"        //Get EFLAGS back into EAX. The only way to read EFLAGS is to pushf then pop.
                "or $0x200, %%eax\n\t" //Set the IF flag, IF flag has a mask of 0x200
                "push %%eax\n\t"        //Push the new EFLAGS value back onto the stack.
                "pushl $0x1B\n\t" 
                "push $1f\n\t" 
                "iret\n\t" 
              "1:\n\t" 
			    :        //no input
				:        //no output
				: "%eax" //eax is clobbered
                );
}

/*
 * load_TR
 *  DESCRIPTION: load the 16-bit add to TR(task register)
 *  INPUTS: TSS_D_add: address of the TSS descriptor
 *  OUTPUTS: none
 *  RETURN VALUE: 1 on success
 *  SIDE EFFECTS: TR is modified
 */
int load_TR(uint16_t* TSS_D_add){
    asm volatile(
                "ltr (%0)"
                :                    // no output
                :   "r"(TSS_D_add)   // input is pointer to TSS descriptor
                );
	return 1;//success
}
//a similar function is defined in x86_desc.h, I'm not sure which one should be used
