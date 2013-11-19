#include "lib.h"
#include "types.h"
#include "fs.h"
#include "terminal.h"
#include "rtc.h"
#include "paging.h"
#include "x86_desc.h"
#include "test_syscalls.h"

#define MAX_PROCESSES 2
//#define VID_VIRT_ADDR           0x10000000 //256 MB
#define VID_VIRT_ADDR 0x8400000 //132MB

/*track current process.*/
pcb_t* pcbs[MAX_PROCESSES];
int32_t curprocess = -1;
pcb_t blahprocess;

// pcbs[0] = (pcb_t*)(EIGHT_MB - sizeof(pcb_t));
// pcbs[1] = (pcb_t*)(pcbs[0] - EIGHT_KB - sizeof(pcb_t));

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
    test_execute((uint8_t*)"shell");
    //printf("This is the %s call\n",__func__);
    return 0;
}

uint32_t proc_page_dir[MAX_PROCESSES][PAGE_DIR_SIZE] __attribute__((aligned(PG_DIR_ALIGN)));
#define MB_128              0x8000000
#define USR_PRGRM_VIRT_LC   MB_128+0x48000
#define MAX_PRGRM_SZ        FOUR_MB
#define ELF_MAG_NUM         0x464C457F

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
    /* can be used later */
    //Set up process out of nowhere!
    if(1)
    {
        curprocess = 0;
        pcbs[curprocess] = &blahprocess;
        pcbs[curprocess]->available_fds = 3;
    }
    //Already have both processes running
    if(curprocess == 1)
        return -1;
    else if(curprocess == 0);

    /* testing code below now */
    //Return error on invalid argument
    if(command == NULL)
        return -1;
    // printf("This is the %s call\n",__func__);

    /* set location of program image */
    uint32_t prog_loc = USR_PRGRM_VIRT_LC;

    /* load file into contiguous memory */
    int8_t* mycommand[MAX_ARG_BUFFER];
    uint8_t temp_size;
    int8_t* arguments[MAX_ARG_BUFFER];
    strcpy((int8_t*)mycommand, (const int8_t*)command);
    strip_buf_whitespace((int8_t*)mycommand, &temp_size);
    parse_command((int8_t*)mycommand, (int8_t*)arguments, &temp_size);

    //Error on inability to open filename
    file_desc_t temp_file;
    cur_file = &temp_file;
    if(-1 == fs_open_file((uint8_t*)mycommand))
        return -1;

    //Error on inability to read file
    uint8_t* tempbuf[4];
    if(-1 == fs_read_file((void*)tempbuf, 4))
        return -1;

    //Error on no executable magic number
    if(((uint32_t*)tempbuf)[0] != ELF_MAG_NUM)
        return -1;

    /* set up paging for process */
    get_proc_page_dir(proc_page_dir[curprocess], EIGHT_MB, MB_128);
    set_CR3((uint32_t)proc_page_dir[curprocess]);
    
    //Error on failed load
    if(-1 ==load_file((int8_t*)mycommand, (void*)prog_loc, MAX_PRGRM_SZ))
        return -1;

    /* check for magic constant indicating executable file */
    if(((uint32_t*)prog_loc)[0] != ELF_MAG_NUM) {
        // printf("Magic number not found\n");
        return -1;
    }
    else {
        // printf("Magic number: %u\n", ((uint32_t*)prog_loc)[0]);
        // printf("Bytes read: %u\n", bytes_read);
    }

    //store arguments
    strcpy((int8_t*)pcbs[curprocess]->arg_buffer, (const int8_t*)arguments);
    pcbs[curprocess]->arg_buffer_size = temp_size;

    /* initialize standard output */
    pcbs[curprocess]->file_desc_arr[STDOUT].file_ops_table = termfops_table;
    pcbs[curprocess]->file_desc_arr[STDIN].file_ops_table = termfops_table;
    pcbs[curprocess]->available_fds = 3;
    pcbs[curprocess]->file_desc_arr[STDOUT].flags = 1;
    pcbs[curprocess]->file_desc_arr[STDIN].flags = 1;
    /* initialize kernel stack for when user program makes system call */
    tss.esp0 = 0x7FFFFC;
    tss.ss0 =  KERNEL_DS;

    /* pass location of user program's first instruction to be executed 
       and jump to procedure to issue iret */
    asm volatile(
        "movl %0, %%eax\n\t"
        "jmp do_iret\n\t"
        :
        :"r" (*((uint32_t*)(prog_loc+24))) /* location of first instruction */
        :"%eax"
        );

    return -1;
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
    //Return error on invalid argument
    if(buf == NULL)
        return -1;
    //Error on out-of-range fd
    if(fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;
    //Error on invalid PCB for process
    if(pcbs[curprocess] == NULL)
        return -1;
    //Error on unopened file
    if((pcbs[curprocess]->available_fds & (1 << fd)) == 0)
        return -1;
    //Error on negative number of bytes
    if(nbytes < 0)
        return -1;
    //Set current file for read function to use
    cur_file = &(pcbs[curprocess]->file_desc_arr[fd]);
    //Call read function
    return((syscall_read_t)(pcbs[curprocess]->file_desc_arr[fd].file_ops_table[FOPS_READ]))
            (buf,nbytes);
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
    //Return error on invalid argument
    if(buf == NULL)
        return -1;
    //Error on out-of-range fd
    if(fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;
    //Error on invalid PCB for process
    if(pcbs[curprocess] == NULL)
        return -1;
    //Error on unopened file
    if((pcbs[curprocess]->available_fds & (1 << fd)) == 0)
        return -1;
    //Error on negative number of bytes
    if(nbytes < 0)
        return -1;
    //Set current file for read function to use
    cur_file = &(pcbs[curprocess]->file_desc_arr[fd]);
    //Call read function
    return((syscall_write_t)(pcbs[curprocess]->file_desc_arr[fd].file_ops_table[FOPS_WRITE]))
            (buf,nbytes);
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
    dentry_t myfiledentry;
    // printf("This is the %s call\n",__func__);
    //Return error on invalid argument
    if(filename == NULL || curprocess < 0 || curprocess > MAX_PROCESSES)
        return -1;
    //Error on invalid PCB for process
    if(pcbs[(uint32_t)curprocess] == NULL)
        return -1;
    int i;
    int32_t fd = -1;
    uint8_t available = pcbs[curprocess]->available_fds;
    for (i = 0; i < MAX_OPEN_FILES; i++)
    {
        if(!(available & (1 << i)))
        {
            fd = i;
            pcbs[curprocess]->available_fds |= (1 << i);
            break;
        }
    }
    //failed to find empty file descriptor
    if(fd == -1)
        return -1;

    if(-1 == read_dentry_by_name(filename, &myfiledentry))
        return -1;
    switch(myfiledentry.ftype)
    {
        case TYPE_USER:
            pcbs[curprocess]->file_desc_arr[fd].file_ops_table = rtcfops_table;
            // printf("RTC file\n");
            break;
        case TYPE_DIR:
            pcbs[curprocess]->file_desc_arr[fd].file_ops_table = dirfops_table;
            // printf("Directory file \n");
            break;
        case TYPE_REG:
            pcbs[curprocess]->file_desc_arr[fd].file_ops_table = filefops_table;
            // printf("Regular file \n");
            break;
        default:
            // printf("INVALID FILE!\n");
            //free assigned fd
            pcbs[curprocess]->available_fds &= (!(1 << fd));
            return -1;
    }
    //Attempt to open file
    cur_file = &(pcbs[curprocess]->file_desc_arr[fd]);
    int32_t retval = ((syscall_open_t)(pcbs[curprocess]->file_desc_arr[fd].file_ops_table[FOPS_OPEN]))
                     (filename);
    if(retval == -1)
    {
        //on failure, release assigned fd and return error.
        pcbs[curprocess]->available_fds &= (~(1 << fd));
        return -1;
    }
    //Set iNode, beginning of file, and in use
    pcbs[curprocess]->file_desc_arr[fd].inode_ptr = myfiledentry.index_node;
    pcbs[curprocess]->file_desc_arr[fd].file_pos = 0;
    pcbs[curprocess]->file_desc_arr[fd].flags = 1;
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
    // printf("This is the %s call\n",__func__);
    //Error on out-of-range fd
    if(fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;
    //Error on invalid PCB for process
    if(pcbs[curprocess] == NULL)
        return -1;
    //Error on unopened file
    if((pcbs[curprocess]->available_fds & (1 << fd)) == 0)
        return -1;
    pcbs[curprocess]->available_fds &= (~(1 << fd));
    cur_file = &(pcbs[curprocess]->file_desc_arr[fd]);
    int32_t retval = ((syscall_close_t)(pcbs[curprocess]->file_desc_arr[fd].file_ops_table[FOPS_CLOSE]))
                      (fd);
    //Error on failed close; must undo mark as available fd
    if(retval == -1)
    {
        pcbs[curprocess]->available_fds |= (1 << fd);
        return -1;
    }
    //mark as unused
    pcbs[curprocess]->file_desc_arr[fd].flags = 0;
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
    //Return error on invalid argument
    if(buf == NULL)
        return -1;
    //Error on invalid PCB for process
    if(pcbs[curprocess] == NULL)
        return -1;
    //Prep buffer for delivery by removing preceding whitespace
    //and counting size of buffer
    strip_buf_whitespace((int8_t*)pcbs[curprocess]->arg_buffer, &pcbs[curprocess]->arg_buffer_size);
    //Error on larger buffer than can fit
    if(pcbs[curprocess]->arg_buffer_size > nbytes)
        return -1;
    strcpy((int8_t*) buf, (int8_t*) pcbs[curprocess]->arg_buffer);
    // printf("This is the %s call\n",__func__);
    return 0;   
}

/* page directory memory */
    uint32_t vid_pg_dir_ent;
    /* page table for first 4MB of memory */
    uint32_t vid_pg_tbl[PAGE_TABLE_SIZE] __attribute__((aligned(PG_TBL_ALIGN))); //needs to be static so can align properly (so that not placed on stack)

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
    uint32_t video_virt_addr = (uint32_t) VID_VIRT_ADDR; //virtual address of video memory

    vid_pg_tbl[0] = VIDEO | SET_PAGE_PRES | SET_PAGE_RW | SET_PAGE_USER;

    /* add page mapping for video memory */
    vid_pg_dir_ent = (uint32_t)vid_pg_tbl;
    vid_pg_dir_ent |= (SET_PAGE_PRES | SET_PAGE_RW | SET_PAGE_USER);

    proc_page_dir[curprocess][video_virt_addr/FOUR_MB] = (uint32_t) vid_pg_dir_ent;

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
    while(command[i] != ' ' && command[i] != '\n') i++;
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
