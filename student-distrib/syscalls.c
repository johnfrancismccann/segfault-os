#include "lib.h"
#include "types.h"
#include "fs.h"

#define MAX_PROCESSES 2

#if 1  /*track current process. commented to prevent warnings */
    pcb_t* pcbs[MAX_PROCESSES];
    int8_t curprocess = -1;
#endif 

/*
 *
 */
int32_t sys_halt(void)
{
    uint8_t status;
    //Load arguments from registers
    //System calls can't use normal c calling convention
    asm("\t movb %%bl,%0" : "=r"(status));
    printf("This is the %s call\n",__func__);
    return -1;
}

/*
 *
 */
int32_t sys_execute(void)
{
    uint8_t* command;
    //Load arguments from registers
    //System calls can't use normal c calling convention
    asm("\t movl %%ebx,%0" : "=r"(command));
    //Return error on invalid argument
    if(command == NULL)
        return -1;
    printf("This is the %s call\n",__func__);
    return -1;
}

/* read, write functions don't do anything until current_pcb is initialized in
   sys_open */
int32_t sys_read(void)
{
    int32_t fd;
    void* buf;
    int32_t nbytes;
    //Load arguments from registers
    //System calls can't use normal c calling convention
    asm("\t movl %%ebx,%0 \n"
        "\t movl %%ecx,%1 \n"
        "\t movl %%edx,%2" : "=r"(fd), "=r"(buf), "=r"(nbytes));
    //Return error on invalid argument
    if(buf == NULL)
        return -1;
    printf("This is the %s call\n",__func__);
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
    //Load arguments from registers
    //System calls can't use normal c calling convention
    asm("\t movl %%ebx,%0 \n"
        "\t movl %%ecx,%1 \n"
        "\t movl %%edx,%2" : "=r"(fd), "=r"(buf), "=r"(nbytes));
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
int32_t sys_open(void)
{
    uint8_t* filename;
    dentry_t myfiledentry;
    //Load arguments from registers
    //System calls can't use normal c calling convention
    asm("\t movl %%ebx,%0" : "=r"(filename));
    printf("This is the %s call\n",__func__);
    //Return error on invalid argument
    if(filename == NULL || curprocess < 0 || curprocess > MAX_PROCESSES)
        return -1;
    //Error on invalid PCB for process
    if(pcbs[curprocess] == NULL)
        return -1;
    read_dentry_by_name(filename, &myfiledentry);
    switch(myfiledentry.ftype)
    {
        case TYPE_USER:
            printf("RTC file\n");
            return 0;
        case TYPE_DIR:
            printf("Directory file \n");
            return 0;
        case TYPE_REG:
            printf("Regular file \n");
            return 0;
        default:
            printf("INVALID FILE!\n");
            return -1;
    }
    return -1;
}

/*
 *
 */
int32_t sys_close(void)
{
    uint32_t fd;
    //Load arguments from registers
    //System calls can't use normal c calling convention
    asm("\t movl %%ebx,%0" : "=r"(fd));
    printf("This is the %s call\n",__func__);
    return -1;
}

/*
 *
 */
int32_t sys_getargs(void)
{
    uint8_t* buf;
    int32_t nbytes;
    //Load arguments from registers
    //System calls can't use normal c calling convention
    asm("\t movl %%ebx,%0\n"
        "\t movl %%ecx,%1" : "=r"(buf), "=r"(nbytes));
    //Return error on invalid argument
    if(buf == NULL)
        return -1;
    printf("This is the %s call\n",__func__);
    return -1;   
}

/*
 *
 */
int32_t sys_vidmap(void)
{
    uint8_t** screen_start;
    //Load arguments from registers
    //System calls can't use normal c calling convention
    asm("\t movl %%ebx,%0" : "=r"(screen_start));
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
int32_t sys_set_handler(void)
{
    int32_t signum;
    void* handler_address;
    //Load arguments from registers
    //System calls can't use normal c calling convention
    asm("\t movl %%ebx,%0\n"
        "\t movl %%ecx,%1" : "=r"(signum), "=r"(handler_address));
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

