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

#define MAX_PROCESSES 9
static uint32_t proc_page_dir[MAX_PROCESSES][PAGE_DIR_SIZE] __attribute__((aligned(PG_DIR_ALIGN)));

#define ELF_MAG_NUM         0x464C457F
#define KERNEL_STACK_SZ     EIGHT_KB
#define MB_128              0x8000000

#define TEST_MULT 1
#define SCED_ON 1

/* program time slice in milliseconds */
#define TIME_SLICE 20
#define HZ (1000 / TIME_SLICE)
#define CLOCK_TICK_RATE 1193182
#define LATCH (CLOCK_TICK_RATE/HZ)

#define PIT_CHAN_0_PORT 0x40
#define PIT_COMMAND_PORT 0x43

#define PIT_IRQ_NUM 0
#define PIT_IDT_NUM 0x20

/* terminal with the currently executing process */
static int32_t active_term = -1;

/* pointer to current pcb running in each terminal */
static pcb_t* cur_proc[NUM_TERMS] = {NULL};
/* total number of processes running */
static uint32_t num_proc = 0;

/* number of processes running in each terminal */
//static uint32_t term_proc_num[NUM_TERMS] = {0};

void schedul();

/* */
void init_pit()
{
    cli();

    /* select channel 0 */
    outb(0x36, PIT_COMMAND_PORT);
    //nop();

    /* load low byte reload value */
    outb(LATCH & 0xff, PIT_CHAN_0_PORT);
    //nop();
    /* load high byte reload value */
    outb(LATCH >> 8, PIT_CHAN_0_PORT);
    //nop();

    /* set up pit interrup in idt */
    set_interrupt_gate(PIT_IDT_NUM, pit_wrapper);
    /* enable pit interrupt */
    enable_irq(PIT_IRQ_NUM);

    sti();

}

/* */
void schedul()
{

    send_eoi(PIT_IRQ_NUM);

    pcb_t* new_proc = NULL;
    pcb_t* old_proc = NULL; 
    /* if old process exists, get its pcb */
    if(active_term != -1)
        old_proc = cur_proc[active_term];
 
    /* move to process in next terminal */
    active_term++;
    /* wrap to first terminal if on last terminal */
    if(active_term == NUM_TERMS)
        active_term = 0;
    /* set active operations terminal to terminal of new process */
    //set_act_ops_term(active_term);
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

void launch_scheduler()
{
#if SCED_ON
    init_pit();
#else
    active_term = 0;
    test_execute((uint8_t*)"shell");
#endif
}

pcb_t* get_cur_proc()
{
    return cur_proc[active_term];
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
    child_proc->par_proc = cur_proc[active_term];
    child_proc->available_fds = 3;

    /* init child's standard i/o */
    child_proc->file_desc_arr[STDOUT].file_ops_table = termfops_table;
    child_proc->file_desc_arr[STDIN].file_ops_table = termfops_table;
    child_proc->file_desc_arr[STDOUT].flags = 1;
    child_proc->file_desc_arr[STDIN].flags = 1;
    /* set child proc as not video mapped */
    child_proc->vid_mapped = 0;

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

    return 0;
}

void destroy_proc()
{
    num_proc--;
    if(cur_proc[active_term]->par_proc) {

        /* execution of child process completed. null par_proc handled in halt. 
           restore child's parent process to be current process */
        cur_proc[active_term] = cur_proc[active_term]->par_proc;
        /* restore parent process' initial kernel stack location in tss */
        tss.esp0 = cur_proc[active_term]->tss_kstack;
        /* restore parent process' paging */
        set_CR3((uint32_t)cur_proc[active_term]->page_dir);
    }
    else {
        /* no process left to resume */
        cur_proc[active_term] = NULL;
        //num_proc = 0;
        /* restart shell */
        test_execute((uint8_t*)"shell");
    }
}

void set_arguments(const int8_t* arguments, uint8_t size)
{
    strcpy((int8_t*)cur_proc[active_term]->arg_buffer, arguments);
    cur_proc[active_term]->arg_buffer_size = size;
}

void set_par_ebp(uint32_t par_ebp)
{
    if(cur_proc[active_term]->par_proc)
        cur_proc[active_term]->par_proc->top_kstack = par_ebp;
}

/* get the current process' ebp */
uint32_t get_ebp()
{
    return cur_proc[active_term]->top_kstack;
}

/* get the page directory of the process running in terminal term_ind */
uint32_t get_page_dir(uint32_t term_ind)
{
    return (uint32_t)cur_proc[active_term]->page_dir;
}


