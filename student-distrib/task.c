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

//to load TSS, load the address of TSS descriptor to TR(Task Register)
//asm code is LTR
void task_execute(struct_task cur_task){

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
