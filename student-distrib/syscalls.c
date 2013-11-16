#include "lib.h"
#include "types.h"

#if 0  /*track current process. commented to prevent warnings */
static pcb_t* current_pcb=NULL;
#endif 

/*
 *
 */
int32_t sys_halt(uint8_t status)
{
    
    return 0;
}

/*
 *
 */
int32_t sys_execute(const uint8_t* command)
{
    
    return 0;
}

/* read, write functions don't do anything until current_pcb is initialized in
   sys_open */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes)
{

    printf("This is the read system call\n");
    return 0;
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
    printf("This is the write system call\n");
    return 0;
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
    return 0;
}

/*
 *
 */
int32_t sys_close(int32_t fd)
{
    
    return 0;
}

/*
 *
 */
int32_t sys_getargs(uint8_t* buf, int32_t nbytes)
{
    
    return 0;   
}

/*
 *
 */
int32_t sys_vidmap(uint8_t** screen_start)
{
    
        return 0;
}

/*
 *
 */
int32_t sys_set_handler(int32_t signum, void* handler_address)
{
        return 0;   
}

/*
 *
 */
int32_t sys_sigreturn(void)
{
        return 0;   
}

