#include "paging.h"
#include "lib.h"

/* page directory memory */
unsigned int page_dir[PAGE_DIR_SIZE] __attribute__((aligned(PG_DIR_ALIGN)));
/* page table for first 4MB of memory */
unsigned int page_table[PAGE_TABLE_SIZE] __attribute__((aligned(PG_TBL_ALIGN)));

/*
 * enable_paging
 *   DESCRIPTION: enables paging within the processor by utilizing
 *                 the processor control registers
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes the Page-Directory-Base field of CR3 to 
 *                 the address of the page directory. Sets the PSE
 *								 field of the CR4 to enable 4MB pages. Finally 
 *								 sets the PE field of CR0 to enable paging								 
 */
void enable_paging()
{
		
	asm volatile(
				/* load address of page directory into CR3 */
				"movl %0, %%cr3\n\t"
				
				/* enable PSE field in CR4 to enable 4MB page accesses  */
				"movl %%cr4, %%eax\n\t"
				"orl $0x00000010, %%eax\n\t"
				"movl %%eax, %%cr4\n\t" 

				/*set paging bit in CR0 to enable paging */
				"movl %%cr0, %%eax\n\t"
				"orl $0x80000000, %%eax\n\t"
				"movl %%eax, %%cr0\n\t"

				:								// no output
				:	"r"(page_dir)	// input is pointer to page directory
				: "%eax"				// eax is clobbered
				);
}

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
void init_paging()
{
	/* declare page directory entries, general purpose counter */
	unsigned int video_dir;
	unsigned int ker_dir;
	unsigned int i;
		
	/* initialize first page to not present, but r/w */	
	page_table[0] = SET_PAGE_RW;	
	/* initialize page_table to contain contiguous 4MB block of memory */
	for(i=1; i<PAGE_TABLE_SIZE; i++) 
		page_table[i] = ((PAGE_SIZE_4K*i) | (SET_PAGE_PRES | SET_PAGE_RW));

	/* initialize video directory entry with page_table and set to present, r/w */
	video_dir = (unsigned int)page_table;
	video_dir |= (SET_PAGE_PRES | SET_PAGE_RW);
	
	/* initialize kernel directory entry to point to page at 4MB */
	ker_dir = KERNEL_LOAD_POINT;
	/* set kernel directory as present, r/w */
	ker_dir |= (SET_PAGE_PRES | SET_PAGE_RW);
	/* set as a 4MB page */
	ker_dir |= SET_PAGE_4MB;
	
	/* assign directory entries for first 8 MB of memory */
	page_dir[0] = video_dir;
	page_dir[1] = ker_dir;
	
	/* set rest of memory as not present, but to read/write */
	for(i=2; i<PAGE_DIR_SIZE; i++)
		page_dir[i] = SET_PAGE_RW;		
		
	/* enable paging by changing control register values */	
	enable_paging();
}



