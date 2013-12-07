#include "types.h"
#include "keyboard.h"
#include "process.h"
#include "error_codes.h"
#include "paging.h"
#include "fs.h"
#include "syscalls.h"

#define MAX_PROCESSES 30
#define MB_128              0x8000000
#define USR_PRGRM_VIRT_LC   MB_128+0x48000
#define MAX_PRGRM_SZ        FOUR_MB
#define ELF_MAG_NUM         0x464C457F
#define KERNEL_STACK_SZ     EIGHT_KB

uint32_t num_proc = 0;

uint32_t proc_by_term[NUM_TERMS] = {0};

pcb_t* term_top_proc[NUM_TERMS] = {NULL};

pcb_t* processes[MAX_PROCESSES] = {NULL};

uint32_t proc_page_dir[MAX_PROCESSES][PAGE_DIR_SIZE] __attribute__((aligned(PG_DIR_ALIGN)));

uint32_t get_available_pid();

/*
 * new_process
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
uint32_t new_process()
{
    // If max processes exceded, don't make new process
    if((num_proc + 1) > MAX_PROCESSES)
        return PID_ERR;
    num_proc++;
    uint32_t curterm = get_active_terminal();
    pcb_t* child_proc = (pcb_t*)(EIGHT_MB-(num_proc)*KERNEL_STACK_SZ);
    uint32_t pid = get_available_pid();
    // If no available PID, return error.
    if(pid == PID_ERR)
    {
        num_proc --;
        return PID_ERR;
    }
    processes[pid-1] = child_proc;
    child_proc->term_num = curterm;
    child_proc->pid = pid;
    child_proc->par_proc = term_top_proc[curterm];
    term_top_proc[curterm] = child_proc;
    proc_by_term[curterm]++;

    child_proc->page_dir = proc_page_dir[child_proc->pid - 1];
    get_proc_page_dir(child_proc->page_dir, EIGHT_MB+(num_proc-1)*FOUR_MB,
                      MB_128);
    child_proc->tss_kstack = EIGHT_MB-(num_proc-1)*KERNEL_STACK_SZ-BYTE;

    return pid;
}

/*
 * get_process
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
pcb_t* get_process(uint32_t pid)
{
    // If too high number, return NULL
    if(pid > MAX_PROCESSES)
        return NULL;
    // Else, return pointer to process if it exists
    return processes[pid-1];
}

/*
 * get_curprocess
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
uint32_t get_curprocess()
{
    uint32_t curterm = get_active_terminal();
    //If no active process, return 0;
    if(term_top_proc[curterm] == NULL)
        return 0;
    //Else return pid for current active process
    return(term_top_proc[curterm]->pid);
}


/*
 * del_process
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
uint32_t del_process()
{
    // If no active processes, don't attempt deletion
    if(num_proc == 0)
        return 0;
    uint32_t curterm = get_active_terminal();
    // If no active processes on active terminal, don't attempt deletion
    if(proc_by_term[curterm] == 0)
        return 0;
    proc_by_term[curterm]--;
    num_proc--;
    pcb_t* curproc = term_top_proc[curterm];
    term_top_proc[curterm] = curproc->par_proc;
    processes[curproc->pid] = NULL;
    if(term_top_proc[curterm] == NULL)
        return 0;
    return term_top_proc[curterm]->pid;
}


/*
 * get_available_pid
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
uint32_t get_available_pid()
{
    int i;
    //Find first available PID.  0 is reserved for Kernel (unused)
    for(i=1; i <= MAX_PROCESSES; i++)
    {
        //On first available PID, return PID number
        if(processes[i-1] == NULL)
            return i;
    }
    //No available PIDs, return pid_err.
    return PID_ERR;
}
