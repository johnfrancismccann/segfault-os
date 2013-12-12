#include "lib.h"
#include "types.h"
#include "fs.h"
#include "terminal.h"
#include "rtc.h"
#include "paging.h"
#include "x86_desc.h"
#include "test_syscalls.h"
#include "keyboard.h"
#include "process.h"

//#define VID_VIRT_ADDR           0x10000000 //256 MB
#define VID_VIRT_ADDR 0x8400000 //132MB

/* return value of user level program */
int32_t proc_retval;

file_desc_t cur_file_obj;
file_desc_t* cur_file = NULL;

void strip_buf_whitespace(int8_t* buf, uint8_t* size);
void parse_command(int8_t* command, int8_t* args, uint8_t* size);

int32_t sys_halt(uint8_t status);
int32_t sys_execute(const uint8_t* command);
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t sys_open(const uint8_t* filename);
int32_t sys_close(int32_t fd);
int32_t sys_getargs(uint8_t* buf, int32_t nbytes);
int32_t sys_vidmap(uint8_t** screen_start);
int32_t sys_set_handler(int32_t signum, void* handler_address);
int32_t sys_sigreturn(void);

/*
 * sys_halt
 *   DESCRIPTION: Ends current process and returns to parent process.
 *   INPUTS:    status: Return value for the corresponding execute.
 *                      Should be 0-255 for normal exits, 256 for
 *                      faults
 *   OUTPUTS:   NONE
 *   RETURN VALUE: returns -1 on failure, doesn't return on success.
 *   SIDE EFFECTS: kills current process.
 */
int32_t sys_halt(uint8_t status)
{

    proc_retval = (int32_t)status;

    /* process destroyed. current process is either parent
       or new shell */
    destroy_proc();

    /* reload new current process' ebp to return to its location
       in the kernel */
    asm volatile(
        //"movl %%ebx,%0\n\t"
        "movl %0, %%ebp\n\t"
        "jmp ret_from_user\n\t"
        :
        :"r"(get_ebp())
        //:"=r"(status)
        );

    //printf("This is the %s call\n",__func__);
    return 0;
}

#define ELF_MAG_NUM         0x464C457F
#define MB_128              0x8000000
#define USR_PRGRM_VIRT_LC   MB_128+0x48000
#define MAX_PRGRM_SZ        FOUR_MB

/*
 * sys_execute
 *   DESCRIPTION: Runs user-level program
 *   INPUTS:    command: string containing program name and any
 *                       arguments.
 *   OUTPUTS:   NONE
 *   RETURN VALUE: returns the exit code from the cooresponding
 *                 halt call for the process. -1 on error.
 *   SIDE EFFECTS: starts new user-level program, switches stacks,
 *                 creates new process/task.
 */
int32_t sys_execute(const uint8_t* command)
{
    int32_t flags;
    /* check for invalid argument */
    if(command == NULL)
        return -1;
    /* check that max number of processes isn't exceeded */

    /* extract filename from command */
    int8_t* mycommand[MAX_ARG_BUFFER];
    uint8_t temp_size;
    int8_t* arguments[MAX_ARG_BUFFER];
    strcpy((int8_t*)mycommand, (const int8_t*)command);
    strip_buf_whitespace((int8_t*)mycommand, &temp_size);
    parse_command((int8_t*)mycommand, (int8_t*)arguments, &temp_size);
    /* although temp_file is allocated on stack, only needs to exist for 
       the two subsequent fs_open_file, fs_read_file calls */
    file_desc_t temp_file;
    cli_and_save(flags);
    cur_file = &temp_file;
    /* try to open extracted filename */
    if(-1 == fs_open_file((uint8_t*)mycommand))
        return -1;
    /* read extracted file's first 4 bytes that will be checked for magic
       constant */
    uint8_t* tempbuf[4];
    if(-1 == fs_read_file((void*)tempbuf, 4))
        return -1;
    /* check that extracted file is an executable */
    if(((uint32_t*)tempbuf)[0] != ELF_MAG_NUM)
        return -1;

    /* new child process created that is set as current process */
    if(-1 == create_proc())
        return -1;

    /* save parent's ebp */
    uint32_t par_ebp;
    asm volatile(
                "movl %%ebp, %0\n\t"
                :"=r"(par_ebp)
                );

    set_par_ebp(par_ebp);
    set_arguments((const int8_t*)arguments, temp_size);

    /* set base location of child's program image */
    uint32_t prog_loc = USR_PRGRM_VIRT_LC;
    /* load child's executable into contiguous memory at base location */
    load_file((int8_t*)mycommand, (void*)prog_loc, MAX_PRGRM_SZ);

    restore_flags(flags);

    /* begin execution of new process with first program instruction */
    asm volatile(
        "movl %0, %%eax\n\t"
        "jmp do_iret\n\t"
        "ret_from_user:\n\t"
        :
        :"r" (*((uint32_t*)(prog_loc+24))) /* location of first instruction */
        :"%eax"
        );
    /* the child process has finished execution, and the current process is the 
       child's parent */

    /* return to parent */
    return proc_retval;
}

/*
 * sys_read
 *   DESCRIPTION: runs the read fops function for a given file
 *   INPUTS:    fd: file descriptor assigned for the current
 *                  file opened by process.
 *              buf: buffer to read bytes into.
 *              nbytes: number of bytes requested to read.
 *   OUTPUTS: outputs data to the provided buffer
 *   RETURN VALUE: returns the number of bytes successfully read.
 *                 -1 on error.
 *   SIDE EFFECTS: None.
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes)
{
    int32_t flags;
    int32_t retval;

    pcb_t* cur_proc = get_cur_proc();

    //Return error on invalid argument
    if(buf == NULL)
        return -1;
    //Error on out-of-range fd
    if(fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;
    //Error on invalid PCB for process
    if(cur_proc == NULL)
        return -1;
    //Error on unopened file
    if((cur_proc->available_fds & (1 << fd)) == 0)
        return -1;
    //Error on negative number of bytes
    if(nbytes < 0)
        return -1;
    cli_and_save(flags);
    //Set current file for read function to use
    cur_file = &(cur_proc->file_desc_arr[fd]);
    //Call read function
    retval = ((syscall_read_t)(cur_proc->file_desc_arr[fd].file_ops_table[FOPS_READ]))
            (buf,nbytes);
    restore_flags(flags);
    return retval;
    // printf("This is the %s call\n",__func__);
}

/*
 * sys_write
 *   DESCRIPTION: runs the write fops function for a given file
 *   INPUTS:    fd: file descriptor assigned for the current
 *                  file opened by process.
 *              buf: buffer to write bytes from.
 *              nbytes: number of bytes requested to write.
 *   OUTPUTS: None.
 *   RETURN VALUE: returns number of bytes written or -1 on error.
 *   SIDE EFFECTS: may modify files/hardware depending on the
 *                 specific write function.
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes)
{
    int32_t flags;
    int32_t retval;

    pcb_t* cur_proc = get_cur_proc();

    //Return error on invalid argument
    if(buf == NULL)
        return -1;
    //Error on out-of-range fd
    if(fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;
    //Error on invalid PCB for process
    if(cur_proc == NULL)
        return -1;
    //Error on unopened file
    if((cur_proc->available_fds & (1 << fd)) == 0)
        return -1;
    //Error on negative number of bytes
    if(nbytes < 0)
        return -1;
    //Set current file for read function to use
    cli_and_save(flags);
    cur_file = &(cur_proc->file_desc_arr[fd]);
    //Call read function
    retval = ((syscall_write_t)(cur_proc->file_desc_arr[fd].file_ops_table[FOPS_WRITE]))
            (buf,nbytes);
    restore_flags(flags);
    return retval;
    // printf("This is the %s call\n",__func__);
}

/*
 * sys_open
 *   DESCRIPTION: Opens a file for a process and assigns
 *                a unique file identifier number.
 *   INPUTS:    filename: string name of the file to open.
 *   OUTPUTS: NONE
 *   RETURN VALUE: returns file descriptor or -1 on failure.
 *   SIDE EFFECTS: adds file to open file table of process.
 */
int32_t sys_open(const uint8_t* filename)
{
    int32_t flags;
    pcb_t* cur_proc = get_cur_proc();

    dentry_t myfiledentry;
    // printf("This is the %s call\n",__func__);

    //Return error on invalid argument
    if(filename == NULL)
        return -1; 

    //Error on invalid PCB for process
    if(cur_proc == NULL)
        return -1;
    int i;
    int32_t fd = -1;
    uint8_t available = cur_proc->available_fds;
    for (i = 0; i < MAX_OPEN_FILES; i++)
    {
        if(!(available & (1 << i)))
        {
            fd = i;
            cur_proc->available_fds |= (1 << i);
            break;
        }
    }
    //failed to find empty file descriptor
    if(fd == -1)
        return -1;

    if(-1 == read_dentry_by_name(filename, &myfiledentry))
    {
        //on failure, release assigned fd and return error.
        cur_proc->available_fds &= (~(1 << fd));
        return -1;
    }
    switch(myfiledentry.ftype)
    {
        case TYPE_USER:
            cur_proc->file_desc_arr[fd].file_ops_table = rtcfops_table;
            // printf("RTC file\n");
            break;
        case TYPE_DIR:
            cur_proc->file_desc_arr[fd].file_ops_table = dirfops_table;
            // printf("Directory file \n");
            break;
        case TYPE_REG:
            cur_proc->file_desc_arr[fd].file_ops_table = filefops_table;
            // printf("Regular file \n");
            break;
        default:
            // printf("INVALID FILE!\n");
            //free assigned fd
            cur_proc->available_fds &= (!(1 << fd));
            return -1;
    }
    //Attempt to open file
    cli_and_save(flags);
    cur_file = &(cur_proc->file_desc_arr[fd]);
    int32_t retval = ((syscall_open_t)(cur_proc->file_desc_arr[fd].file_ops_table[FOPS_OPEN]))
                     (filename);
    restore_flags(flags);
    if(retval == -1)
    {
        //on failure, release assigned fd and return error.
        cur_proc->available_fds &= (~(1 << fd));
        return -1;
    }
    //Set iNode, beginning of file, and in use
    cur_proc->file_desc_arr[fd].inode_ptr = myfiledentry.index_node;
    cur_proc->file_desc_arr[fd].file_pos = 0;
    cur_proc->file_desc_arr[fd].flags = 1;
    return fd;
}

/*
 * sys_close
 *   DESCRIPTION: closes open file owned by process and frees
 *                that file's file descriptor index.
 *   INPUTS:    fd: the file descriptor index of the file to
 *                  be closed.
 *   OUTPUTS: NONE
 *   RETURN VALUE: returns 0 on success, 1 on failure.
 *   SIDE EFFECTS: closes an open file making it unavailable to
 *                 the process until re-opened.
 */
int32_t sys_close(int32_t fd)
{
    int32_t flags;
    pcb_t* cur_proc = get_cur_proc();
    // printf("This is the %s call\n",__func__);
    //Error on out-of-range fd
    if(fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;
    //Error on invalid PCB for process
    if(cur_proc == NULL)
        return -1;
    //Error on unopened file
    if((cur_proc->available_fds & (1 << fd)) == 0)
        return -1;
    cur_proc->available_fds &= (~(1 << fd));
    cli_and_save(flags);
    cur_file = &(cur_proc->file_desc_arr[fd]);
    int32_t retval = ((syscall_close_t)(cur_proc->file_desc_arr[fd].file_ops_table[FOPS_CLOSE]))
                      (fd);
    restore_flags(flags);
    //Error on failed close; must undo mark as available fd
    if(retval == -1)
    {
        cur_proc->available_fds |= (1 << fd);
        return -1;
    }
    //mark as unused
    cur_proc->file_desc_arr[fd].flags = 0;
    return retval;
}

/*
 * sys_getargs
 *   DESCRIPTION: returns the arguments supplied to a program
 *                when it was first executed.
 *   INPUTS:    buf: buffer into which to pass the arguments
 *              nbytes: maximum number of bytes to copy into the
 *                      buffer.
 *   OUTPUTS: Sends out the buffer with arguments
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: None.
 */
int32_t sys_getargs(uint8_t* buf, int32_t nbytes)
{

    pcb_t* cur_proc = get_cur_proc();

    //Return error on invalid argument
    if(buf == NULL)
        return -1;
    //Error on invalid PCB for process
    if(cur_proc == NULL)
        return -1;
    //Prep buffer for delivery by removing preceding whitespace
    //and counting size of buffer
    strip_buf_whitespace((int8_t*)cur_proc->arg_buffer, &cur_proc->arg_buffer_size);
    //Error on larger buffer than can fit
    if(cur_proc->arg_buffer_size > nbytes)
        return -1;
    strcpy((int8_t*) buf, (int8_t*) cur_proc->arg_buffer);
    // printf("This is the %s call\n",__func__);
    return 0;   
}



/* page directory memory */
uint32_t vid_pg_dir_ent;
/* page table for first 4MB of memory */
static uint32_t vid_pg_tbl[PAGE_TABLE_SIZE] __attribute__((aligned(PG_TBL_ALIGN))); //needs to be static so can align properly (so that not placed on stack)

/*
 * sys_vidmap
 *   DESCRIPTION: maps video memory for user program use.
 *   INPUTS:    screen_start: address to pass the pointer to the
 *                            video memory to?
 *   OUTPUTS: screen_start.
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: None.
 */
int32_t sys_vidmap(uint8_t** screen_start)
{

    pcb_t* cur_proc = get_cur_proc();

    /* check input parameter */
    if(screen_start == NULL) 
        return -1;

    /* set the address of virtual video memory */
    uint32_t video_virt_addr = (uint32_t) VID_VIRT_ADDR; //virtual address of video memory
    /* get the video display memory from the keyboard driver. either video or store */
    uint32_t vid_phys_addr = get_act_ops_disp();

    //vid_pg_tbl[0] = VIDEO | SET_PAGE_PRES | SET_PAGE_RW | SET_PAGE_USER;
    /* map first entry of video page table to the video memory */
    vid_pg_tbl[0] = vid_phys_addr | SET_PAGE_PRES | SET_PAGE_RW | SET_PAGE_USER;

    /* set corresponding page directory entry to video page table */
    vid_pg_dir_ent = (uint32_t)vid_pg_tbl;
    vid_pg_dir_ent |= (SET_PAGE_PRES | SET_PAGE_RW | SET_PAGE_USER);
    (cur_proc->page_dir)[video_virt_addr/FOUR_MB] = (uint32_t) vid_pg_dir_ent;
    set_CR3((uint32_t)cur_proc->page_dir);
    /* set the current process as video mapped */
    cur_proc->vid_mapped = 1;
    /* supply the location of virtual video memory to the user */
    *screen_start = (uint8_t*) VID_VIRT_ADDR;

    return 0;
}

/*
 * sys_set_handler
 *   DESCRIPTION: Sets handler for signal
 *   INPUTS:    signum: Signal number to be handled.
 *              handler_address: address of handler function.
 *   OUTPUTS: None.
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Sets new signal handler.
 */
int32_t sys_set_handler(int32_t signum, void* handler_address)
{
    //Return error on invalid argument
    if(handler_address == NULL)
        return -1;
    // printf("This is the %s call\n",__func__);
    return -1;   
}

/*
 * sys_sigreturn
 *   DESCRIPTION: sends signal?
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: -1 on failure
 *   SIDE EFFECTS: NONE
 */ 
int32_t sys_sigreturn(void)
{
    // printf("This is the %s call\n",__func__);
    return -1;   
}

/*
 * strip_buf_whitespace
 *   DESCRIPTION: removes spaces from beginning of string.
 *   INPUTS:    buf: string for which you desire spaces
 *                   to be stripped.
 *              size: size of output buffer
 *   OUTPUTS: outputs the two inputs.
 *   RETURN VALUE: None
 *   SIDE EFFECTS: None
 */
void strip_buf_whitespace(int8_t* buf, uint8_t* size)
{
    //Exit on invalid buffer
    if(buf == NULL || size ==NULL)
        return;
    uint8_t tempbuf[MAX_ARG_BUFFER];
    (*size) = 0;
    int i=0;
    //Skip leading spaces
    while(buf[i] == ' ') i++;
    //copy up to MAX_ARG_BUFFER characters into temporary buffer
    //starting after preceding spaces.
    while(i < MAX_ARG_BUFFER)
    {
        if(buf[i] == '\0')
            break;
        tempbuf[*size] = buf[i];
        (*size)++;
        i++;
    }
    //Add ending null character
    if(*size >= MAX_ARG_BUFFER)
    {
        *size = MAX_ARG_BUFFER - 1;
    }
    tempbuf[*size] = '\0';
    //Copy over to actual buffer for output
    strcpy((int8_t*)buf, (int8_t*)tempbuf);
    //Increase size to reflect presence of null terminator
    (*size)++;
}

/*
 * parse_command
 *   DESCRIPTION: Splits an input command string for execute
 *                into a command word and the arguments.
 *   INPUTS:    command: the input full execute command string
 *                       which becomes the command word output.
 *              args: the arguments from the command string.
 *              size: the size of the arguments string.
 *   OUTPUTS: outputs all inputs.
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: NONE
 */
void parse_command(int8_t* command, int8_t* args, uint8_t* size)
{
    //Exit on invalid buffer
    if(command == NULL || args == NULL || size == NULL)
        return;
    (*size) = 0;
    int i=0;
    //First word becomes command
    while(command[i] != ' ' && command[i] != '\n' && command[i] != '\0') i++;
    //copy up to MAX_ARG_BUFFER characters into temporary buffer
    //starting after preceding spaces.
    while(i < MAX_ARG_BUFFER)
    {
        if(command[i] == '\0' || command[i] == '\n')
            break;
        args[*size] = command[i];
        command[i] = '\0';
        (*size)++;
        i++;
    }
    //Add ending null character
    if(*size >= MAX_ARG_BUFFER)
    {
        *size = MAX_ARG_BUFFER - 1;
    }
    args[*size] = '\0';
    command[i] = '\0';
    //Increase size to reflect presence of null terminator
    (*size)++;
}
