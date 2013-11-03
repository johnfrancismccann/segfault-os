/*
 * Tests for basic functionality
 */

#include "types.h"
#include "lib.h"
#include "fs.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"

#define TEST_RDENTRY_NAME 	0    //Set to 1 to test read_dentry_by_name
#define TEST_RDENTRY_INDEX 	0    //Set to 1 to test read_dentry_by_index
#define TEST_RDATA 			0 	 //Set to 1 to test read_data
#define TEST_RDENTRY_INDEX 	0    //Set to 1 to test read_dentry_by_index
#define TEST_RDATA          0 	 //Set to 1 to test read_data

void test_rdentry_name();
void test_rdentry_index();
void test_rdata();

void test_fs()
{

#if TEST_RDENTRY_NAME //set to 1 if wanna test read_dentry_by_name
    test_rdentry_name();
#endif
	
#if TEST_RDENTRY_INDEX //set to 1 if wanna test read_dentry_by_index	
    test_rdentry_index();
#endif
	
#if TEST_RDATA //set to 1 if wanna test read_data
    test_rdata();
#endif

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

    term_open();

    //read first command
    copied_bytes = term_read((void*) ptr);
    //printf("bytes copied %d", copied_bytes);
    puts(ptr);

    //read second command
    copied_bytes = term_read((void*) ptr);
    puts(ptr);
}

/*
 * test_rdentry_name:
 *     DESCRIPTION: Tests file system read dentry by name
 *     INPUTS: None
 *     OUTPUTS: None
 *     RETURN VALUE: None
 *     SIDE EFFECTS: None
 */
void test_rdentry_name()
{
    clear();
    printf("testing read_dentry_by_name\n");

    dentry_t dentry;
    read_dentry_by_name((const uint8_t*)"ls", &dentry);
    
    printf("file_name: %s\n", dentry.file_name);
    printf("ftype: %d\n", dentry.ftype);
    printf("index_node: %d\n", dentry.index_node);
    //printf("length: %d\n", dentry.length);
}

/*
 * test_rdentry_index:
 *     DESCRIPTION: Tests file system read dentry by index
 *     INPUTS: None
 *     OUTPUTS: None
 *     RETURN VALUE: None
 *     SIDE EFFECTS: None
 */
void test_rdentry_index()
{
		clear();
    printf("testing read_dentry_by_index\n");

    dentry_t _dentry;
    uint32_t index_den = 0x19;    
    read_dentry_by_index(index_den, &_dentry);
    
    printf("file_name: %s\n", _dentry.file_name);
    printf("ftype: %d\n", _dentry.ftype);
    printf("index_node: %d\n", _dentry.index_node);
    //printf("length: %d\n", _dentry.length);
}

/*
 * test_rdata:
 *     DESCRIPTION: Tests file system read data
 *     INPUTS: None
 *     OUTPUTS: None
 *     RETURN VALUE: None
 *     SIDE EFFECTS: None
 */
void test_rdata()
{
		clear();
    printf("testing read_data\n");
    
    dentry_t dentry;
    uint32_t _inode = dentry.index_node; //0x19;
    uint32_t _offset = 0;
    uint8_t  _buf[7000];
    uint32_t _length = dentry.length;
    
    uint32_t it;
    int32_t ret_val;
    
    ret_val = read_data(_inode, _offset, _buf, _length);
    
    printf("ret value is %d\n", ret_val);

    for(it = 0; it < _length; it++)
    {
        printf("%c", _buf[it]);
    }
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
    uint8_t tester = 0;
    uint32_t power_two = 1;
    clear();
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
    i /= 0;
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
