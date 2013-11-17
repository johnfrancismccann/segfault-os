#include "lib.h"
#include "types.h"
#include "fs.h"
#include "terminal.h"
#include "rtc.h"

#define MAX_PROCESSES 2

#if 1  /*track current process. commented to prevent warnings */
    pcb_t* pcbs[MAX_PROCESSES];
    int32_t curprocess = -1;
#endif 

/*
 *
 */
int32_t sys_halt(uint8_t status)
{
    printf("This is the %s call\n",__func__);
    return -1;
}

/*
 *
 */
int32_t sys_execute(const uint8_t* command)
{
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
    if(buf == NULL)
        return -1;
    printf("This is the %s call\n",__func__);
    printf("First arg: %d\nSecond arg: %s\nThird arg: %d\n", fd, buf, nbytes);
    return -1;

#if 0 /* prevent warnings */
    /* call the read function corresponding to the file type specified by fd */
    return ((syscall_read_t)(current_pcb->file_desc_arr[fd].file_ops_table[1]))
                 (buf, nbytes);
#endif
}

/*
 *
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes)
{
    //Return error on invalid argument
    if(buf == NULL)
        return -1;
    printf("This is the %s call\n",__func__);
    return -1;
#if 0 /* prevent warnings */
    /* call the write function corresponding to the file type specified by fd */
    return ((syscall_write_t)(current_pcb->file_desc_arr[fd].file_ops_table[2]))
                 (buf, nbytes);
#endif
}

/*
 *
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
            printf("RTC file\n");
            break;
        case TYPE_DIR:
            pcbs[curprocess]->file_desc_arr[fd].file_ops_table = dirfops_table;
            printf("Directory file \n");
            break;
        case TYPE_REG:
            pcbs[curprocess]->file_desc_arr[fd].file_ops_table = filefops_table;
            printf("Regular file \n");
            break;
        default:
            printf("INVALID FILE!\n");
            //free assigned fd
            pcbs[curprocess]->available_fds &= (!(1 << fd));
            return -1;
    }
    //Attempt to open file
    int32_t retval = ((syscall_open_t)(pcbs[curprocess]->file_desc_arr[fd].file_ops_table[FOPS_OPEN]))(NULL);
    if(retval == -1)
    {
        //on failure, release assigned fd and return error.
        pcbs[curprocess]->available_fds &= (!(1 << fd));
        return -1;
    }
    return fd;
}

/*
 *
 */
int32_t sys_close(int32_t fd)
{
    printf("This is the %s call\n",__func__);
    //Error on out-of-range fd
    if(fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;
    //Error on invalid PCB for process
    if(pcbs[(uint32_t)curprocess] == NULL)
        return -1;
    return -1;
}

/*
 *
 */
int32_t sys_getargs(uint8_t* buf, int32_t nbytes)
{
    //Return error on invalid argument
    if(buf == NULL)
        return -1;
    printf("This is the %s call\n",__func__);
    return -1;   
}

/*
 *
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
 */ 
int32_t sys_sigreturn(void)
{
    printf("This is the %s call\n",__func__);
    return -1;   
}

