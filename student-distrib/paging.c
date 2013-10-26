#include "paging.h"
#include "lib.h"

unsigned int page_dir[PAGE_DIR_SIZE] __attribute__((aligned(PG_DIR_ALIGN)));
unsigned int page_table[PAGE_TABLE_SIZE] __attribute__((aligned(PG_TBL_ALIGN)));

void enable_paging();

void init_paging()
{

	/* declare page directory entries */
	unsigned int video_dir;
	unsigned int ker_dir;
	/* counter */
	unsigned int i;
	
	/* initialize page_table to contain contiguous 4MB block of memory */
	for(i=0; i<1024; i++)
		page_table[i] = 4096*i;

	/* initialize video directory entry with page_table and set to present, r/w */
	video_dir = (unsigned int)page_table;
	video_dir |= 0x3;
	
	/* initialize kernel directory entry to point to page at 4MB */
	ker_dir = 0x400000;
	/* set kernel directory as present */
	ker_dir |= 0x1;
	/* set as a 4MB page */
	ker_dir |= 64;
	
	/* assign directory entries for first 8 MB of memory */
	page_dir[0] = video_dir;
	page_dir[1] = ker_dir;
	
	/* set rest of memory as not present */
	for(i=2; i<1024; i++)
		page_dir[i] = 0;
		
	enable_paging();
	
}

/*
 * enable_paging
 *   DESCRIPTION: enabling paging
 *   INPUTS: 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes cr3, cr0 and cr4.
 */
void enable_paging()
{
    //%eax and %ecx is being used
    __asm__("pushl %eax\n\t"
	        "pushl %ecx\n\t"
	        "movl page_dir, %eax\n\t"//load cr3 with address of page directory, change this to the apporiate address
	        "andl $0xFFC00000,%eax\n\t"//get the first 10 bits of memory address,clear everything else
			"movl %cr3, %ecx\n\t"
            "andl $0x3FFFFF,%ecx\n\t"//clear first 10 digits of cr3
            "orl %ecx, %eax\n\t"			
            "movl %eax, %cr3\n\t"
 
            //set paging bit in CR0
            "movl %cr0, %eax\n\t"
            "orl $0x80000000, %eax\n\t"
            "movl %eax, %cr0\n\t"
	
	        //enable PSE in CR4 because we need 4MB pages 
	        "movl %cr4, %eax\n\t"
            "orl $0x00000010, %eax\n\t"
            "movl %eax, %cr4\n\t"
			"popl %ecx\n\t"
			"popl %eax\n\t");

			
	printf("Paging Enabled\n");
}


#if 0
int main()
{

	init_paging();
	return 0;


}
#endif

