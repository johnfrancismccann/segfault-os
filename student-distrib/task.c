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
	int8_t buf[5000];//CHANGE THIS!!
	//call_loader(buf);

	int8_t _24_to_27[EIP_SIZE];
    int i;//iterator
	//copy the new EIP value to the temp buffer
	for (i = 0; i < EIP_SIZE; i++)
        _24_to_27[i] = buf[EIP_START+i];
		
	int8_t* new_eip = _24_to_27;
	
	//modify some values according to the first 40 bytes if the file
    //thigs to set: EIP,CS,EFLAGS,ESP,SS	
    //modify TSS	
	//tss.ESP = ;
	//tss.SS = ;//I don't know where these values come from
	
	//do the system call after everything is set up
    if (sys_execute() == -1)
	    return -1;//kill the task
	
	//return success if no exceptions
	return 1;
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
