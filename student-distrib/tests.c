/*
 * Tests for basic functionality
 */

#include "types.h"
#include "lib.h"
#include "fs.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"

#define TEST_RDENTRY_NAME 	0 
#define TEST_RDENTRY_INDEX 	0   
#define TEST_RDATA 					1 	 

void test_rdentry_name();
void test_rdentry_index();
void test_rdata();

/* test_fs:
 *		DESCRIPTION: tests file system
 *			INPUTS: None
 *			OUTPUTS: None
 *			RETURN VALUE: None
 *			SIDE EFFECTS: None
 */
void test_fs()
{
		clear();
		
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

    copied_bytes = term_read((void*) ptr);
    puts(ptr);

    // while(1) {
    // copied_bytes = term_read((void*) ptr);
    // term_write(ptr, copied_bytes);
    // }

    // while(1) {
    //     copied_bytes = term_read((void*) ptr);
    //     puts(ptr);
    // }
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

    char ptr[] = "hellogsa;lka;lkjgdalhellogsa;lka;lkjgdalkjgdagdsa;lkjgdahellogsa;lka;lkjgdalkjgdagdsa;lkjgdakjgdagdsa;lkjgda";
    int copied_bytes;

    clear();

    // while(1) {
    //     ptr++;
    //     copied_bytes = term_write((const void*) (&ptr), 1);
    // }

    copied_bytes = term_write((const void*) ptr, 100);
    printf("bytes copied %d\n", copied_bytes);
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
		uint8_t *filename;
    int32_t ret_val;
		dentry_t dentry;
		
		/* set filename */
		filename = (uint8_t*) "grep";
		/* print header */
    printf("testing read_dentry_by_name for file: %s\n", filename);
		
		/* fill directory entry for specified file  */
    ret_val = read_dentry_by_name(filename, &dentry);
		/* check return value */
		if(!ret_val) {
		/* print directory entry information */
			printf("file_name: %s\n", dentry.file_name);
			printf("ftype: %d\n", dentry.ftype);
			printf("index_node: %d\n", dentry.index_node);
		}
		else if(ret_val == -1)
			printf("reading dentry failed\n");
		
		printf("\n");
		
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
    dentry_t _dentry;
		uint32_t index_dentry;
		int32_t ret_val;
		
		index_dentry = 0; 
		/* print header */
    printf("testing read_dentry_by_index for index: %u\n", index_dentry);

		/* fill directory entry for specified index */
    ret_val = read_dentry_by_index(index_dentry, &_dentry);
    /* check return value */
		if(!ret_val) {
			/* print direcotry entry information */
			printf("file_name: %s\n", _dentry.file_name);
			printf("ftype: %d\n", _dentry.ftype);
			printf("index_node: %d\n", _dentry.index_node);
		}
		else if(ret_val == -1) 
			printf("Reading dentry by index failed\n");
		
		printf("\n");
}

#define TEST_TEXT 0
#define TEST_NON_TEXT 1

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
		uint8_t* filename;
		uint32_t _length = 100;
		uint8_t _buf[_length];
		uint32_t _inode;
		uint32_t i;
		uint32_t bytes_read;
	
		/* set filename to read */
		filename = (uint8_t*) "hello";
		/* print header */
    printf("Testing read_data for file %s\n", filename);		
    
		/* fill directory entry of specified file */
    dentry_t dentry;
		read_dentry_by_name(filename, &dentry);
		
		/* initialize arguments for read_data */
    _inode = dentry.index_node;
    
		/* fill buffer with data from file */
    bytes_read = read_data(_inode, 0, _buf, _length);
		 
#if TEST_TEXT
    for(i = 0; i < _length; i++) {
        printf("%c", _buf[i]);
    }
#endif
    
#if TEST_NON_TEXT
		for(i = 0; i < _length; i++) {
        printf("%x ", _buf[i]);
				/* start printing at next line after 20 characters */
				if(!(i%20) && i > 0)
					printf("\n");
    }

		
#endif
		
		printf("\n");
		printf("Bytes read: %u ", bytes_read);
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
