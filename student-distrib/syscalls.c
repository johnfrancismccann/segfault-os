#include "lib.h"
#include "types.h"
#include "fs.h"
#include "terminal.h"
#include "rtc.h"

#define MAX_PROCESSES 2

/*track current process.*/
pcb_t* pcbs[MAX_PROCESSES];
int32_t curprocess = -1;

//for testing with dysfunctional execute
pcb_t blahprocess;

file_desc_t* cur_file = NULL;

void strip_buf_whitespace(uint8_t* buf, uint8_t* size);


/*
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t sys_halt(uint8_t status)
{
    printf("This is the %s call\n",__func__);
    return -1;
}

/*
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t sys_execute(const uint8_t* command)
{
    //Set up process out of nowhere!
    if(1)
    {
        curprocess = 0;
        pcbs[curprocess] = &blahprocess;
        pcbs[curprocess]->available_fds = 3;
    }
    //Return error on invalid argument
    if(command == NULL)
        return -1;
    printf("This is the %s call\n",__func__);
    return -1;
}

/* read, write functions don't do anything until current_pcb is initialized in
   sys_open */
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
    printf("This is the %s call\n",__func__);
}

/*
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
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
    printf("This is the %s call\n",__func__);
}

/*
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t sys_open(const uint8_t* filename)
{
    dentry_t myfiledentry;
    printf("This is the %s call\n",__func__);
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
    read_dentry_by_name(filename, &myfiledentry);
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
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t sys_close(int32_t fd)
{
    printf("This is the %s call\n",__func__);
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
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
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
    strip_buf_whitespace(pcbs[curprocess]->arg_buffer, &pcbs[curprocess]->arg_buffer_size);
    //Error on larger buffer than can fit
    if(pcbs[curprocess]->arg_buffer_size > nbytes)
        return -1;
    strcpy((int8_t*) buf, (int8_t*) pcbs[curprocess]->arg_buffer);
    printf("This is the %s call\n",__func__);
    return -1;   
}

/*
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t sys_vidmap(uint8_t** screen_start)
{
    //Return error on invalid argument
    if(screen_start == NULL)
        return -1;
    else if(*screen_start == NULL)
        return -1;
    printf("This is the %s call\n",__func__);
    return -1;
}

/*
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t sys_set_handler(int32_t signum, void* handler_address)
{
    //Return error on invalid argument
    if(handler_address == NULL)
        return -1;
    printf("This is the %s call\n",__func__);
    return -1;   
}

/*
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */ 
int32_t sys_sigreturn(void)
{
    printf("This is the %s call\n",__func__);
    return -1;   
}

/*
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
void strip_buf_whitespace(uint8_t* buf, uint8_t* size)
{
    //Exit on invalid buffer
    if(buf == NULL)
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
