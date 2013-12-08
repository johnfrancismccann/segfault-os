#include "process.h"
#include "x86_desc.h"
#include "paging.h"
#include "terminal.h"
#include "test_syscalls.h"
#include "lib.h"

#define MAX_PROCESSES 2
static uint32_t proc_page_dir[MAX_PROCESSES][PAGE_DIR_SIZE] __attribute__((aligned(PG_DIR_ALIGN)));

#define ELF_MAG_NUM         0x464C457F
#define KERNEL_STACK_SZ     EIGHT_KB
#define MB_128              0x8000000

/* pointer to current pcb */
static pcb_t* cur_proc = NULL;
/* number of processes */
static int32_t num_proc = 0;

pcb_t* get_cur_proc()
{
    return cur_proc;
}

int32_t create_proc()
{
    /* check that max number of processes isn't exceeded */
    if(num_proc >= MAX_PROCESSES)
        return -1;
    
    num_proc++;
    /* create new pcb for child in memory */
    pcb_t* child_proc = (pcb_t*)(EIGHT_MB-(num_proc)*KERNEL_STACK_SZ);
    child_proc->pid = num_proc-1;
    child_proc->par_proc = cur_proc;
    child_proc->available_fds = 3;

    /* init child's standard i/o */
    child_proc->file_desc_arr[STDOUT].file_ops_table = termfops_table;
    child_proc->file_desc_arr[STDIN].file_ops_table = termfops_table;
    child_proc->file_desc_arr[STDOUT].flags = 1;
    child_proc->file_desc_arr[STDIN].flags = 1;

    /* set up child's paging. set processor to child's page directory */
    child_proc->page_dir = proc_page_dir[child_proc->pid];
    get_proc_page_dir(child_proc->page_dir, EIGHT_MB+(num_proc-1)*FOUR_MB,
                      MB_128);
    set_CR3((uint32_t)child_proc->page_dir);

    /* set child's initial kernel stack in pcb and in tss */
    child_proc->tss_kstack = EIGHT_MB-(num_proc-1)*KERNEL_STACK_SZ-BYTE;
    tss.esp0 = child_proc->tss_kstack;
    tss.ss0 =  KERNEL_DS;

    /* finally, set the child process as the current process */
    cur_proc = child_proc; 

    return 0;

}

void destroy_proc()
{

    num_proc--;
    if(cur_proc->par_proc) {

        /* execution of child process completed. null par_proc handled in halt. 
           restore child's parent process to be current process */
        cur_proc = cur_proc->par_proc;
        /* restore parent process' initial kernel stack location in tss */
        tss.esp0 = cur_proc->tss_kstack;
        /* restore parent process' paging */
        set_CR3((uint32_t)cur_proc->page_dir);
    }
    else {
        /* no process left to resume */
        cur_proc = NULL;
        num_proc = 0;
        /* restart shell */
        test_execute((uint8_t*)"shell");
    }
}

void set_arguments(const int8_t* arguments, uint8_t size)
{
    strcpy((int8_t*)cur_proc->arg_buffer, arguments);
    cur_proc->arg_buffer_size = size;
}

void set_par_ebp(uint32_t par_ebp)
{
    if(cur_proc->par_proc)
        cur_proc->par_proc->top_kstack = par_ebp;
}

uint32_t get_ebp()
{
    return cur_proc->top_kstack;
}

