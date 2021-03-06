/*
 * Tests for basic functionality
 */

#include "types.h"
#include "lib.h"
#include "fs.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"
#include "test_syscalls.h"

void test_handin_code()
{
    /*clear();
    reset_screen_pos();
    char buf[1024];
    int size;
    fs_open_file("frame0.txt");
    size = fs_read_file(buf, 1024);
    fs_close_file("frame0.txt");

    printf("size: %d\n", size);
    int i;
    for (i = 0; i < size; i++)
        printf("%c", buf[i]);

    asm volatile(".2: hlt; jmp .2;");*/
    /*clear();
    int i;
    for (i = 0; i < 15; i++)
    {
        rtc_read(NULL, 0);
        printf("a\n");
    }
    uint32_t freq = 16; //fix was to change from uint8_t to uint32_t
    rtc_write(&freq, 1);
    for (i = 0; i < 5; i++)
    {
        rtc_read(NULL, 0);
        printf("b\n");
    }
    rtc_close();*/
}

/*
 * test_terminal_read:
 *     DESCRIPTION: Tests terminal_read function
 *     INPUTS: None
 *     OUTPUTS: None
 *     RETURN VALUE: None
 *     SIDE EFFECTS: None
 */
void test_terminal_read()
{
    int8_t ptr[128]; //pointer to copy char buffer from keyboard to user space
    int copied_bytes;
    int bytes_to_copy;

    // copied_bytes = term_read((void*) ptr);
    // puts(ptr);

    // while(1) {
    // copied_bytes = term_read((void*) ptr);
    // term_write(ptr, copied_bytes);
    // }

    bytes_to_copy = 10;
    while(1) {
        copied_bytes = term_read((void*) ptr, bytes_to_copy);
        ptr[bytes_to_copy] = '\0'; //make sure to account for string null-termination
        puts(ptr);
    }
}

/*
 * test_terminal_write:
 *     DESCRIPTION: Tests terminal_write function
 *     INPUTS: None
 *     OUTPUTS: None
 *     RETURN VALUE: None
 *     SIDE EFFECTS: None
 */
void test_terminal_write()
{
    //char ptr = 0;

    char ptr[] = "Hello";
    int copied_bytes;

    clear();

    // while(1) {
    //     ptr++;
    //     copied_bytes = term_write((const void*) (&ptr), 1);
    // }

    copied_bytes = term_write((const void*) ptr, 5);
    printf("bytes copied %d\n", copied_bytes);
}

/* filesystem test macros */
#define TST_FS_R_TXT            1
#define TST_FS_R_LRG_FILE       0
#define TST_FS_R_NON_TXT_FILE   0
#define TST_FS_R_DIR            0
#define TST_FS_OTHER            0
/* test paramters */
#define NUM_TERM_CHARS      2000
#define MAX_FILE_SZ         10000
#define MX_DIR_ENTRY_SZ     33
/* for large files, type determines print set size */
#if TST_FS_R_NON_TXT_FILE
    #define PRINT_SET_SZ    4
#else
    #define PRINT_SET_SZ    400
#endif
#define PRINT_SET_ROW       16
/*filesystem test functions */
void test_rd_txt_fl();
void test_rd_lrg_fl();
void test_rd_dir();
void test_other();
void print_hex_lines(uint8_t* buf, uint32_t n_bytes);

/* test_fs:
 *      DESCRIPTION: tests component of file system specified by macros
 *      INPUTS: None
 *      OUTPUTS: None
 *      RETURN VALUE: None
 *      SIDE EFFECTS: filesystem test is printed to screen.
 */
void test_fs()
{
    clear();
    reset_screen_pos();

#if TST_FS_R_TXT
    test_rd_txt_fl();
#endif /* read text file */

#if TST_FS_R_LRG_FILE
    test_rd_lrg_fl();
#endif /*  read large file. could be non text file */

#if TST_FS_R_DIR
    test_rd_dir();
#endif /* read directory */

#if TST_FS_OTHER
    test_other();
#endif /* test anything written in test_other */

}


/*
 * test_rd_txt_fl
 * DESCRIPTION: test reading of text file from fs.
 *              use for small files
 * INPUTS: none
 * OUTPUTS: none
 * RETURN_VALUE: none
 * SIDE_EFFECTS: text file is printed to screen. 
 */
void test_rd_txt_fl()
{

    uint8_t* filename;
    int32_t _length = NUM_TERM_CHARS;
    uint8_t _buf[_length];
    int32_t bytes_read;
    
    /* set filename to read */
    filename = (uint8_t*) "frame1.txt";
    /* print header */
  printf("Testing reading of text file: %s\n", filename);

    /* open and read file */
    fs_open_file(filename);  
    bytes_read = fs_read_file(_buf, _length);
    fs_close_file(filename);
   
    /* print file */
    printf("%d bytes read. File contents:\n\n", bytes_read);
    printf("%s", _buf);
}

/*
 * test_rd_lrg_fl
 * DESCRIPTION: test reading of large file. large file can be 
 *              txt or non-txt file depending on whether 
 *              TST_FS_R_NON_TXT_FILE is set to 1 or 0
 * INPUTS: none
 * OUTPUTS: none
 * RETURN_VALUE: none
 * SIDE_EFFECTS: beginning and ending contents of a large 
 *               file are written to the screen. if a non-txt
 *               file is being tested, hex characters are ouput.
 *               otherwise, ascii characters are output
 */
void test_rd_lrg_fl()
{
    uint8_t* filename;
    int32_t _length = MAX_FILE_SZ;
    uint8_t _buf[_length];
    int32_t bytes_read;
    int32_t tot_bytes_read;
    
/* set filename to either text or non-text file */
#if TST_FS_R_NON_TXT_FILE
    filename = (uint8_t*) "grep";
    //filename = (uint8_t*) "ls";
#else
    filename = (uint8_t*) "verylargetxtwithverylongname.tx";
    //filename = (uint8_t*) "frame0.txt";
#endif /* R_NON_TEXT_FILE */

    /* print header */
  printf("Testing reading of large file: %s\n", filename);

    /* open file and read first set of bytes */
    fs_open_file(filename);  
    tot_bytes_read = bytes_read = fs_read_file(_buf, PRINT_SET_SZ);
    /* print first set of bytes */
    printf("First %d bytes read. Contents:\n", bytes_read);
    
/* print hex characters if non-text file, else, print ascii characters */
#if TST_FS_R_NON_TXT_FILE
    print_hex_lines(_buf, bytes_read);
#else
    _buf[bytes_read] = '\0';
    printf("%s", _buf);
#endif /* TST_FS_R_NON_TXT_FILE */

    /* read until EOF */
    do {
        bytes_read = fs_read_file(_buf, PRINT_SET_SZ);
        tot_bytes_read += bytes_read;
    } while(bytes_read == PRINT_SET_SZ);
    
    fs_close_file();
    /* print last set of bytes */
    printf("\n\nLast %d bytes read. Contents:\n", bytes_read);
    
/* print hex characters if non-text file, else, print ascii characters */
#if TST_FS_R_NON_TXT_FILE
    print_hex_lines(_buf, bytes_read);
#else
    _buf[bytes_read] = '\0';
    printf("%s", _buf);
#endif /* TST_FS_R_NON_TXT_FILE */
    
    /* print total bytes read */
    printf("\n\nTotal bytes read: %d\n", tot_bytes_read);
}

/*
 * print_hex_lines
 * DESCRIPTION: print rows of PRINT_SET_ROW characters to screen in 
 *              hexadecimal format.  
 * INPUTS: buf--buffer containing charcters to print
 *         n_bytes--number of characters in buf
 * OUTPUTS: none
 * RETURN_VALUE: none
 * SIDE_EFFECTS: characters passed in buffer are printed to screen,
 *               formatted in hexadecimal
 */
void print_hex_lines(uint8_t* buf, uint32_t n_bytes)
{
    uint32_t i;
    /* iterate through bytes */
    for(i=0; i<n_bytes; i++) {
        /* print PRINT_SET_ROW characters per display line */
        if( !(i%PRINT_SET_ROW) && i>0)
            printf("\n");
        /* print hex characters to compare with hex dump */
        printf("%x ", buf[i]);
    }
}

/*
 * test_rd_dir
 * DESCRIPTION: test reading of directory. because the filesystem is flat,
 *              only the pwd, ".", can be read 
 * INPUTS: none
 * OUTPUTS: none
 * RETURN_VALUE: none
 * SIDE_EFFECTS: directory contents are read and printed to screen
 *
 */
void test_rd_dir()
{
    uint8_t* dir_name;
    uint8_t dir_buf[MX_DIR_ENTRY_SZ];
    uint32_t dir_entry_sz;
    uint32_t i;
    
    /* set directory name and print header */
    dir_name = (uint8_t*)".";
  printf("Testing reading of directory: %s\nDirectories:\n", dir_name);
    /* open directory */
    fs_open_dir(dir_name);
    i = 0;
    /* read directories one at a time */
    while(0 != (dir_entry_sz = fs_read_dir(dir_buf, MX_DIR_ENTRY_SZ))) {
        /* print each directory entry on one line */
        dir_buf[dir_entry_sz] = '\0';
        printf("%s\n", dir_buf);
        i++;
    }
    /* print number of directories and close the directory */
    printf("\nDirectory entries read: %u\n", i);
    fs_close_dir();
}

/*
 * test_other
 * DESCRIPTION: test any desired behavior 
 * INPUTS: none
 * OUTPUTS: none
 * RETURN_VALUE: none
 * SIDE_EFFECTS: desired test is performed. the return value of the
 *               test is also printed
 */
void test_other()
{
    int32_t ret_val;
    
    /* begin tests */
    //ret_val = fs_close_file();
    ret_val = fs_open_file((uint8_t*)"frame0.tx");
    /* end tests */
    
    /* print results */
    printf("Return value: %d\n", ret_val);
}

/*
 * test_rwrtc:
 *     DESCRIPTION: Tests RTC read and write functions
 *     INPUTS: None
 *     OUTPUTS: None
 *     RETURN VALUE: None
 *     SIDE EFFECTS: Changes RTC interrupt frequency
 */
void test_rwrtc()
{
/* Attempts to set various frequencies for RTC interrupts
 * and then does read loops to print to screen. */
    uint32_t tester = 0;
    uint32_t power_two = 1;
    clear();
    reset_screen_pos();
    if(rtc_write(&tester, 1) == -1) //attempt to set frequency of 0Hz
        printf("\tRTC 0Hz FAIL");
    else
    {
        printf("\tRTC 0Hz SUCCESS\n");
        for(tester = 0; tester < 10; tester ++)
        {
            rtc_read(NULL, 0);
            printf("%d\t", tester);
        }
    }
    /* 1 should fail, 2-1024 should succeed, 2048-8192 should fail.
     * For each success, 10 interrupts will be processed,
     * for each failure, the failure will be acknowledged.
     */
    for(power_two = 1; power_two < 16384; power_two <<= 1)
    {
        if(rtc_write(&power_two, 1) == -1) //attempt to set frequency
            printf("\n\tRTC %dHz FAIL", power_two);
        else
        {
            printf("\n\tRTC %dHz SUCCESS\n", power_two);
            for(tester = 0; tester < 10; tester ++)
            {
                rtc_read(NULL, 0);
                printf("%d  ", tester);
            }
        }
    }
    tester = 100;
    // Check non power of two; should fail.
    if(rtc_write(&tester, 1) == -1) //attempt to set frequency of 100Hz
        printf("\n\tRTC 100Hz FAIL");
    else
    {
        printf("\n\tRTC 100Hz SUCCESS\n");
        for(tester = 0; tester < 10; tester ++)
        {
            rtc_read(NULL, 0);
            printf("%d\t", tester);
        }
    }
}

/*
 * test_div0:
 *     DESCRIPTION: Tests divide by 0 exception
 *     INPUTS: None
 *     OUTPUTS: None
 *     RETURN VALUE: None
 *     SIDE EFFECTS: Raises divide by 0 exception
 */
void test_div0()
{
    int i = 5;
    int j = 0;
    i /= j;
}

/*
 * test_pagef:
 *     DESCRIPTION: Tests page fault exception
 *     INPUTS: None
 *     OUTPUTS: None
 *     RETURN VALUE: None
 *     SIDE EFFECTS: Raises page fault exception
 */
void test_pagef()
{
    int* i = NULL;
    *i = 5;
}

/*
 * 
 *   DESCRIPTION:
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
void test_syscall()
{
    clear();

//new testing without inline assembly
#if 1
    uint8_t* vmem_base_addr;
    int32_t retval;
    // uint32_t y;
    // uint32_t* addr = 0x8400000; //new VIRT_ADDR

    printf("pre vmem_base_addr %#x\n", vmem_base_addr);
    printf("pre *vmem_base_addr %#x\n", *vmem_base_addr);

    retval = test_vidmap(&vmem_base_addr);
    printf("post vmem_base_addr %#x\n", vmem_base_addr);
    printf("post *vmem_base_addr %#x\n", *vmem_base_addr);
    //y = (uint32_t) *(addr);
#endif

//original testing with inline assembly
#if 0
    uint8_t** pointer;
    int32_t retval;
    uint32_t y;
    //uint32_t* addr = 10001000; //in new video memory block
    //uint32_t* addr = 3145728; //3MB, in video block
    uint32_t* addr = 5242880; //5MB, in kernel block

    printf("pre pointer %d\n", pointer);
    printf("pre *pointer %d\n", *pointer);

    asm volatile(
    "movl   $7, %%eax\n\t" //sys call num
    "movl   %0, %%ebx\n\t" //1st arg
    // "movl   %0, %%ecx\n\t" //2nd
    // "movl   $128, %%edx\n\t" //3rd
    "int    $0x80\n\t"
    :
    : "r"(pointer)
    : "%eax","%ebx","%ecx","%edx"
    );
    asm("\tmovl %%eax, %0" : "=r"(retval));
    printf("retval = %d\n",retval);
    printf("post pointer %d\n", pointer);
    printf("post *pointer %d\n", *pointer);
    //printf("**pointer %d\n", *(*pointer));
    y = (uint32_t) *(addr);
#endif


#if 0
    uint8_t* buffer[1024];
    uint8_t* filename = (uint8_t*)"rtc";
    test_execute(NULL);
    printf("%d\n",test_open(filename));
    test_close(3);
    int i,j;
    //fake fish
    for(i=0; i < 2048; i++)
    {
        if(i%2 == 0)
            filename = (uint8_t*)"frame0.txt";
        else
            filename = (uint8_t*)"frame1.txt";
        test_open(filename);
        test_read(3,(void*)buffer, 1024);
        test_close(3);
        clear();
        reset_screen_pos();
        printf("%s", buffer);
        test_read(2,(void*)buffer,1);
        test_write(2,(void*)(&i),1);
        for(j=0; j < 1024; j++)
            buffer[j] = '\0';
    }
    //test read directory
    test_close(3);
    clear();
    reset_screen_pos();
    filename = (uint8_t*)".";
    printf("%d\n",test_open(filename));
    uint8_t dir_buf[MX_DIR_ENTRY_SZ];
    uint32_t dir_entry_sz;
    while(0 != (dir_entry_sz = test_read(3,(void*)dir_buf,MX_DIR_ENTRY_SZ)))
    {
        dir_buf[dir_entry_sz] = '\0';
        printf("%s\n",dir_buf);
        i++;
    }
#endif

#if 0
    i=2;
    uint8_t* buf = (uint8_t*)"hello";
asm volatile(
        "movl   $2, %%eax\n\t"
        "movl   $55, %%ebx\n\t"
        "movl   %0, %%ecx\n\t"
        "movl   $128, %%edx\n\t"
        "int    $0x80\n\t"
        :
        : "r"(buf)
        : "%eax","%ebx","%ecx","%edx"
        );
        asm("\tmovl %%eax, %0" : "=r"(retval));
        printf("retval = %d\n",retval);
#endif
}

void test_sys_exec()
{
    clear();
    int32_t retval;
    uint8_t* buf = (uint8_t*)"testprint";
    asm volatile(
        "movl   $2, %%eax\n\t"
        "movl   %0, %%ebx\n\t"
        "movl   %0, %%ecx\n\t"
        "movl   $128, %%edx\n\t"
        "int    $0x80\n\t"
        :
        :"r"(buf)
        : "%eax","%ebx","%ecx","%edx"
        );
        
    asm("\tmovl %%eax, %0" : "=r"(retval));
    printf("retval = %d\n",retval);
}

