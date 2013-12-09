#include "process.h"
#include "x86_desc.h"
#include "paging.h"
#include "terminal.h"
#include "test_syscalls.h"
#include "lib.h"
#include "keyboard.h"
#include "setup_idt.h"
#include "i8259.h"
#include "pit_asm.h"


static uint32_t proc_page_dir[MAX_PROCESSES][PAGE_DIR_SIZE] __attribute__((aligned(PG_DIR_ALIGN)));

#define ELF_MAG_NUM         0x464C457F
#define KERNEL_STACK_SZ     EIGHT_KB
#define MB_128              0x8000000

#define TEST_MULT 1
#define SCED_ON 1

#define NON_VID_MAPPED 0
#define VID_MAPPED 1

/* program time slice in milliseconds */
#define TIME_SLICE 20
#define HZ (1000 / TIME_SLICE)
#define CLOCK_TICK_RATE 1193182
#define LATCH (CLOCK_TICK_RATE/HZ)

#define PIT_CHAN_0_PORT  0x40
#define PIT_COMMAND_PORT 0x43
#define PIT_IRQ_NUM      0
#define PIT_IDT_NUM      0x20
#define PIT_LO_HIGH_ACC  0x30
#define PIT_MODE_3       0x6
#define PIT_CHANNEL_0    0x0
/* */
#define LOW_BYTE 0xff
#define BYTE_SZ 8

#define NON_EXIST_TERM -1
#define FIRST_TERM 0



/* terminal with the currently executing process */
static int32_t active_term = NON_EXIST_TERM;

/* pointer to current pcb running in each terminal */
static pcb_t* cur_proc[NUM_TERMS] = {NULL};

/* total number of processes running */
static uint32_t num_proc = 0;

/* number of processes running in each terminal */
//static uint32_t term_proc_num[NUM_TERMS] = {0};

void schedul();

/*
 *   init_pit
 *   DESCRIPTION: initialize the programmable interval timer 
 *                so that it generates an interrupt every
 *                TIME_SLICE milliseconds
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: the pit will be generating interrupts at the
 *                 specified rate, calling the interrupt handler
 *                 schedul to preempt the current process and 
 *                 schedulule another one
 *
 */
void init_pit()
{
    cli();
    /* for pit, select channel 0, accessmode mode lobyte/highbyte, and 
       and mode 3 (square wave generator) */
    outb(PIT_CHANNEL_0 | PIT_LO_HIGH_ACC | PIT_MODE_3, PIT_COMMAND_PORT);
    /* reload value is contained in LATCH. load low byte reload value */
    outb(LATCH & LOW_BYTE, PIT_CHAN_0_PORT);
    /* load high byte reload value */
    outb(LATCH >> BYTE_SZ, PIT_CHAN_0_PORT);
    /* set up pit interrup in idt */
    set_interrupt_gate(PIT_IDT_NUM, pit_wrapper);
    /* enable pit interrupt */
    enable_irq(PIT_IRQ_NUM);
    sti();

}

/*
 *   schedul
 *   DESCRIPTION: preempts the currently executing process, and 
 *                scheduls the process that has been waiting the 
 *                the longest to execute. this is equivalent to swithcing
 *                between the terminals' prrocesses in round robin fashion.
 *                if a process doesn't exist within a terminal, the shell is
 *                launched in that shell
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: setup as an interrupt handler for the pit, which
 *                 is the highest priority interrupt. as such, will
 *                 will be triggered in any context in which interrupts
 *                 are not disabled, and cause a context switch to occur. 
 *
 */
void schedul()
{

    send_eoi(PIT_IRQ_NUM);

    pcb_t* new_proc = NULL;
    pcb_t* old_proc = NULL; 
    /* if old process exists, get its pcb */
    if(active_term != NON_EXIST_TERM)
        old_proc = cur_proc[active_term];
 
    /* move to process in next terminal */
    active_term++;
    /* wrap to first terminal if on last terminal */
    if(active_term == NUM_TERMS)
        active_term = FIRST_TERM;
    /* set active operations terminal to terminal of new process */
    set_act_ops_term(active_term);
    /* get new process pcb */
    new_proc = cur_proc[active_term];

    /* if old process exists, save its ebp */
    if(old_proc) {
        /* save old proc's base pointer in its pcb */
        asm volatile(
                    "movl %%ebp, %0\n\t"
                    :"=r"(old_proc->top_kstack)
                    );
    }

    /* execute shell in terminal if new process doesn't exist */
    if(!new_proc) {
        test_execute((uint8_t*)"shell");
    }
    else {
        /*  */
        tss.esp0 = new_proc->tss_kstack;
        /* set new process' page directory in cr3 */
        set_CR3((uint32_t)new_proc->page_dir);

        /* or, new process does exist. load its base pointer in esp 
           to switch to its kernel stack */
        asm volatile(
                    "movl %0, %%ebp\n\t"
                    :
                    :"r" (new_proc->top_kstack)
                    );
    }
}

/*
 *   launch_scheduler
 *   DESCRIPTION: calls the init_pit function to initialize the pit,
 *                and set up the scheduler function as an interrupt 
 *                handler for PIT interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: in enabling the pit to generate interrupts and thus
 *                 call the scheduler, this function also launches an 
 *                 instance of shell in each terminal
 *
 */
void launch_scheduler()
{
#if SCED_ON
    init_pit();
#else
    active_term = FIRST_TERM;
    test_execute((uint8_t*)"shell");
#endif
}

/*
 *   get_cur_proc
 *   DESCRIPTION: returns a pointer to the currently executing 
 *                process' pcb structure
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: in returning a pointer to the currently executing
 *                 process, this function enables the caller to modify
 *                 the current pcb, which could have significant consequences
 *                 on the execution of that process
 */
pcb_t* get_cur_proc()
{
    if(active_term < FIRST_TERM || active_term >= NUM_TERMS)
        return NULL;
    return cur_proc[active_term];
}

/*
 *   create_proc
 *   DESCRIPTION: creates a new child process of the currently executing
 *                process, and sets that child process as the currently
 *                executing process 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on successful creation of new process. -1 on failure
 *   SIDE EFFECTS: causes a new process to be created and loads the processor
 *                 control register 3 with the process' new page directory
 *                 interrupts are disabled for most of the duration of this
 *                 function. increments the number of processes running
 */
int32_t create_proc()
{
    /* check that max number of processes isn't exceeded */
    if(num_proc >= MAX_PROCESSES)
        return -1;
    int32_t flags;
    cli_and_save(flags);
    num_proc++;
    /* create new pcb for child in memory */
    pcb_t* child_proc = (pcb_t*)(EIGHT_MB-(num_proc)*KERNEL_STACK_SZ);
    child_proc->pid = num_proc-1;
    child_proc->par_proc = cur_proc[active_term];
    //Set keyboard/display as already used on fd 0 & 1
    child_proc->available_fds = STDIN_MSK | STDOUT_MSK;

    /* init child's standard i/o */
    child_proc->file_desc_arr[STDOUT].file_ops_table = termfops_table;
    child_proc->file_desc_arr[STDIN].file_ops_table = termfops_table;
    child_proc->file_desc_arr[STDOUT].flags = FL_IN_USE;
    child_proc->file_desc_arr[STDIN].flags = FL_IN_USE;
    /* set child proc as not video mapped */
    child_proc->vid_mapped = NON_VID_MAPPED;

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
    cur_proc[active_term] = child_proc; 

    restore_flags(flags);

    print_status_bar();

    return 0;
}

/*
 *   destroy_proc
 *   DESCRIPTION: destroys the currently executing process in the active terminal,
 *                and sets the destroyed process' parent process as the currently 
 *                executing process in the active terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: decrements the number of processes running and loads the tss 
 *                 esp0 and cr3 with the corresponding paramters of destroyed 
 *                 process' parent. interrupts are disabled for almost all of this
 *                 function's execution
 */
void destroy_proc()
{
    int32_t flags;
    cli_and_save(flags);
    //Error case
    if(num_proc == 0)
        return;
    int i;
    //ensure all files closed except std i/o
    for(i = 2; i < MAX_OPEN_FILES; i++)
        test_close(i);
    num_proc--;
    if(cur_proc[active_term]->par_proc) {

        /* execution of child process completed. null par_proc handled in halt. 
           restore child's parent process to be current process */
        cur_proc[active_term] = cur_proc[active_term]->par_proc;
        /* restore parent process' initial kernel stack location in tss */
        tss.esp0 = cur_proc[active_term]->tss_kstack;
        /* restore parent process' paging */
        set_CR3((uint32_t)cur_proc[active_term]->page_dir);
        restore_flags(flags);
    }
    else {
        /* no process left to resume */
        cur_proc[active_term] = NULL;
        //num_proc = 0;
        /* restart shell */
        restore_flags(flags);
        test_execute((uint8_t*)"shell");
    }

    print_status_bar();
}

/*
 *   set_arguments
 *   DESCRIPTION: sets the arguments in the currenlty executing 
 *                process' pcb struct
 *   INPUTS: arguments--buffer containing the arguments to set the
 *                      process' pcb
 *           size--size of arguments in bytes
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: the currently executing process has its arguments
 *                 field according to the supplied parameters
 */
void set_arguments(const int8_t* arguments, uint8_t size)
{
    strcpy((int8_t*)cur_proc[active_term]->arg_buffer, arguments);
    cur_proc[active_term]->arg_buffer_size = size;
}

/*
 *   set_par_ebp 
 *   DESCRIPTION: sets the top_kstack field of the currently executing
 *                process' parent's pcb
 *   INPUTS: par_ebp--unsigned integer containing the value to set to the 
 *                    current proc's parent's top_kstack field
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets the parents top_kstack field so that the parent
 *                 can be returned to after the current process finishes
 *                 execution
 */
void set_par_ebp(uint32_t par_ebp)
{
    if(cur_proc[active_term]->par_proc)
        cur_proc[active_term]->par_proc->top_kstack = par_ebp;
}

/*
 *   get_ebp
 *   DESCRIPTION: returns the currently executing process' pcb's top_kstack 
 *                field
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: the currently executing process' pcb's top_kstack field
 *   SIDE EFFECTS: the currently executing process' pcb's top_kstack field 
 *                 is returned to the caller, giving it access to the process'
 *                 kernel stack
 */
uint32_t get_ebp()
{
    return cur_proc[active_term]->top_kstack;
}

/*
 *   get_page_dir
 *   DESCRIPTION: returns the page_dir field of the process executing in the
 *                term_index terminal
 *   INPUTS: term_ind--the terminal containing the process whose page directory
 *                     will be returned
 *   OUTPUTS: none
 *   RETURN VALUE: the address of the page directory
 *   SIDE EFFECTS: the page directory address of the process in the requested
 *                 terminal is returned, enabling the caller to change the 
 *                 system's paging
 */
uint32_t get_page_dir(uint32_t term_ind)
{
    return (uint32_t)cur_proc[active_term]->page_dir;
}

/*
 *   get_vid_mapped
 *   DESCRIPTION: returns the video mapped status of the process
 *                executing in the term_index terminal
 *   INPUTS: term_index-- the terminal containing the process whose 
 *                        video mapped status is requested
 *   OUTPUTS: none
 *   RETURN VALUE: the video mapped status
 *   SIDE EFFECTS: the video mapped status of the currently executing
 *                 terminal is made known to the caller of this function
 */
uint32_t get_vid_mapped(uint32_t term_index)
{
    /* check if process in term_index exists */
    if(cur_proc[term_index])
        return cur_proc[term_index]->vid_mapped;
    /* return not video mapped if nonexistent */
    else
        return 0;
}

/*
 *   remap_4KB_user_page
 *   DESCRIPTION: remaps the 4KB page starting at virt_addr to the 4KB 
 *                phys_addr for the page directory of the process executing 
 *                in terminal term_index
 *   INPUTS: term_index: the terminal whose process' page directory will be 
 *                       changed 
 *           phys_addr: the base address of a 4KB physical page that will be 
 *                      mapped to the base address of the 4KB virtual page
 *                      virt_addr 
 *           virt_addr: the base address of a 4KB virtual page that will be 
 *                      mapped to the base address of the 4KB physical page
 *                      phys_addr
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: the page directory of the process of the requested terminal
 *                 is changed. this page directory is set to control register 
 *                 3 by the processor
 */
int32_t remap_4KB_user_page(uint32_t term_index, uint32_t phys_addr, uint32_t virt_addr)
{
    uint32_t page_table;
    /* get virt_addr's page table */
    page_table = (cur_proc[term_index]->page_dir[virt_addr/FOUR_MB]) & PG_TBL_ADDR_MSK;

    /* set corresponding entry in page table to physical address provided */
    ((uint32_t*)page_table)[(virt_addr>>PG_TBL_FIELD_SZ) & (PG_TBL_FIELD_MSK)] 
    = phys_addr | SET_PAGE_PRES | SET_PAGE_RW | SET_PAGE_USER;

    /* just for testing when not running scheduler */
    //set_CR3((uint32_t)cur_proc[term_index]->page_dir);
    return 0;
}

/*
 *   get_num_processes
 *   DESCRIPTION: returns the total number of processes running
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: the total number of process running
 *   SIDE EFFECTS: the caller of this function is made known as to 
 *                 the total number of processes currently running
 */
uint32_t get_num_processes()
{
    return num_proc;
}
