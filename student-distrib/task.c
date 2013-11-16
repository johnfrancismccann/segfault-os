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
