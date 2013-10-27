#ifndef PAGING_H
#define	PAGING_H

#define PAGE_DIR_SIZE 			0x400
#define PAGE_TABLE_SIZE 		0x400
#define PAGE_SIZE_4K 				0x1000
#define KERNEL_LOAD_POINT 	0x400000

#define SET_PAGE_4MB				0x80
#define SET_PAGE_RW					0x2
#define SET_PAGE_PRES				0x1

#define PG_TBL_ALIGN 0x1000
#define PG_DIR_ALIGN 0x1000

/*
 * init_paging
 *	DESCRIPTION: initialize a page directory and page table so 
 *               that the first 4MB (not including the first 4KB) of 
 *               virtual memory is directly mapped to physical 
 *							 memory and broken into 1024 4 KB pages. the 
 *							 second 4 MB of virtual memory is also directly
 *               mapped to physical memory, but is broken into 
 *               into one 4 MB page.
 *	INPUTS: none
 *	OUTPUTS: none
 * 	RETURN VALUE: none
 *	SIDE EFFECTS: the first 8 MB of virtual memory(not including the 
 *                first 4 KB) are present within physical memory and 
 *                mapped directly to physical memory. the rest of memory 
 *                is not present within physical memory.
 */
extern void init_paging();

#endif

