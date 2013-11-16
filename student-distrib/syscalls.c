#include "lib.h"
#include "types.h"

#if 0  /*track current process. commented to prevent warnings */
static pcb_t* current_pcb=NULL;
#endif 

/*
 *
 */
int32_t sys_halt(void)
{
    uint8_t status;
    //Load locals from registers
    asm("\t movb %%bl,%0" : "=r"(status));
    return -1;
}

/*
 *
 */
int32_t sys_execute(void)
{
    uint8_t* command;
    //Load locals from registers
    asm("\t movl %%ebx,%0" : "=r"(command));
    return -1;
}

/* read, write functions don't do anything until current_pcb is initialized in
   sys_open */
int32_t sys_read(void)
{
    int32_t fd;
    void* buf;
    int32_t nbytes;
    //Load locals from registers
    asm("\t movl %%ebx,%0 \n"
        "\t movl %%ecx,%1 \n"
        "\t movl %%edx,%2" : "=r"(fd), "=r"(buf), "=r"(nbytes));
    printf("This is the read system call\n");
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
int32_t sys_write(void)
{
    int32_t fd;
    void* buf;
    int32_t nbytes;
    //Load locals from registers
    asm("\t movl %%ebx,%0 \n"
        "\t movl %%ecx,%1 \n"
        "\t movl %%edx,%2" : "=r"(fd), "=r"(buf), "=r"(nbytes));
    printf("This is the write system call\n");
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
int32_t sys_open(void)
{
    uint8_t* filename;
    //Load locals from registers
    asm("\t movl %%ebx,%0" : "=r"(filename));
    return -1;
}

/*
 *
 */
int32_t sys_close(void)
{
    uint32_t fd;
    //Load locals from registers
    asm("\t movl %%ebx,%0" : "=r"(fd));
    return -1;
}

/*
 *
 */
int32_t sys_getargs(void)
{
    uint8_t* buf;
    int32_t nbytes;
    //Load locals from registers
    asm("\t movl %%ebx,%0\n"
        "\t movl %%ecx,%1" : "=r"(buf), "=r"(nbytes));
    return -1;   
}

/*
 *
 */
int32_t sys_vidmap(void)
{
    uint8_t** screen_start;
    //Load locals from registers
    asm("\t movl %%ebx,%0" : "=r"(screen_start));
    return -1;
}

/*
 *
 */
int32_t sys_set_handler(void)
{
    int32_t signum;
    void* handler_address;
    //Load locals from registers
    asm("\t movl %%ebx,%0\n"
        "\t movl %%ecx,%1" : "=r"(signum), "=r"(handler_address));
    return -1;   
}

/*
 *
 */
int32_t sys_sigreturn(void)
{
    return -1;   
}

