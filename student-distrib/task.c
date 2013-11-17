#include "task.h"

//all the definitions
#define TASK_ADD 0x08048000 

//From MP3 spec:
/*
 *First user-level program should be loaded at physical 8MB
 *Second one should at 12MB
 *program image itself is linked to execute at virtual address TASK_ADD
 *
 *Things to do:
 *set up a 4MB PDE that maps virtual address 0x08000000(128MB) to the rightphysical address (either 8MB or 12MB)
 *Then the program image must be copied to the correct offset(0x00048000) within that page
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
		
	int8_t* asm_input = _24_to_27;
	
	//modify some values according to the first 40 bytes if the file, modify the stack(the paras are in TSS)
    //thigs to set: EIP,CS,EFLAGS,ESP,SS

	
    //modify TSS	
	//tss;//this is the extern tss defined in x86_desc.h
	
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

/*
 * setup_task_page_dir
 *  DESCRIPTION: set up a PDE at address add 
 *  INPUTS: size: 0 indicates a 4KB-page, 1 indicates a 4MB-page, other values will cause failure;
 *          add: address of the start of the page
 *  OUTPUTS: none
 *  RETURN VALUE: 1 on success; 
 *               -1 on failure
 *  SIDE EFFECTS: modify page_dir(defined in paging.h)
 */
int setup_task_page_dir(uint8_t size, uint32_t* add){
    if ((size !=0)&&(size !=1))
        return -1;
	
	return -1;
}

/*
 * setup_task_page_table
 *  DESCRIPTION: set up a page table at address add 
 *  INPUTS: size:0 indicates a 4KB-page, 1 indicates a 4MB-page, other values will cause failure;
 *          add:address of the start of the page
 *  OUTPUTS: none
 *  RETURN VALUE: 1 on success; 
 *               -1 on failure
 *  SIDE EFFECTS: modify page_table(defined in paging.h)
 */
int setup_task_page_table(uint8_t size, uint32_t* add){
    if ((size !=0 )&&(size !=1 ))
        return -1;
	
	return -1;
}
