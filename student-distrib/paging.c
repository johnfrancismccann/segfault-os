#include "paging.h"
#include "lib.h"

static uint32_t *cur_page_dir = NULL;

/*
 * enable_paging
 *   DESCRIPTION: enables paging within the processor by utilizing
 *                 the processor control registers
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes the Page-Directory-Base field of CR3 to 
 *                 the address of the page directory. Sets the PSE
 *                               field of the CR4 to enable 4MB pages. Finally 
 *                               sets the PE field of CR0 to enable paging                               
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

                :                               // no output
                :   "r"(page_dir)   // input is pointer to page directory
                : "%eax"                // eax is clobbered
                );
}

void set_CR3(uint32_t page_dir_address)
{
    cur_page_dir = (uint32_t*)page_dir_address;
    asm volatile(
        /* load address of page directory into CR3 */
        "movl %0, %%cr3\n\t"
        :
        : "r"(page_dir_address)
        );    
}

/*
 * init_paging
 *  DESCRIPTION: initialize a page directory and page table so 
 *               that the first 4MB (not including the first 4KB) of 
 *               virtual memory is directly mapped to physical 
 *               memory and broken into 1024 4 KB pages. the 
 *               second 4 MB of virtual memory is also directly
 *               mapped to physical memory, but is broken into 
 *               into one 4 MB page.
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: the first 8 MB of virtual memory(not including the 
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
    /* initialize page_table to contain cb ontiguous 4MB block of memory */
    for(i=1; i<PAGE_TABLE_SIZE; i++)
        page_table[i] = ((PAGE_SIZE_4K*i) | (SET_PAGE_PRES | SET_PAGE_RW));

    /* initialize video directory entry with page_table and set to present, r/w (pages 4kB by default) */
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

/* map 4KB page at phys_addr to virt_addr. 
   assumption: page at virt_addr is already mapped to phys. page */
int32_t remap_4KB_user_page(uint32_t phys_addr, uint32_t virt_addr)
{
    uint32_t page_table;
    /* check that input parameters are not invalid */
    if(!phys_addr)
        return -1;
    if(!virt_addr)
        return -1;
    /* get page table for phys_addr  */
    page_table = cur_page_dir[virt_addr>>22] & (0xFFFFF000);
    /* set corresponding entry in page table to physical address provided */
    ((uint32_t*)page_table)[(virt_addr>>12) & (0x3FF)] 
    = phys_addr | SET_PAGE_PRES | SET_PAGE_RW | SET_PAGE_USER;

    return 0;
}

/*
 * get_proc_page_dir
 *  DESCRIPTION: map physical to virtual address in process page directory
 *  INPUTS: process page directory, real (physical) address, virtual address
 *          phys_addr, virt_addr are assumed to be 4 MB aligned
 *  OUTPUTS: none
 *  RETURN VALUE: -1 if error
 *  SIDE EFFECTS: new page in page directory for program
 */
uint32_t get_proc_page_dir(uint32_t* proc_page_dir, uint32_t phys_addr, uint32_t virt_addr)
{
#if 1
    uint32_t i;
    /* perform checks on input */
    if(proc_page_dir == NULL || phys_addr < 0 || virt_addr < 0)
        return -1;
    /* initialize process page table to original mapping */
    proc_page_dir[0] = page_dir[0];
    proc_page_dir[1] = page_dir[1];
    for(i=2; i<PAGE_DIR_SIZE; i++)
        proc_page_dir[i] = page_dir[i];

    /* init new page as 4MB page, user, present, r/w */
    phys_addr |= (SET_PAGE_4MB | SET_PAGE_USER | SET_PAGE_PRES | SET_PAGE_RW);
    proc_page_dir[virt_addr/FOUR_MB] = phys_addr;

#endif

#if 0
    proc_page_dir[0] = page_dir[0];
    proc_page_dir[1] = page_dir[1];
    int i;
    for(i=2; i<PAGE_DIR_SIZE; i++)
        proc_page_dir[i] = page_dir[i];
#endif

    return 0;
}
