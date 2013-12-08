#include "types.h"

#ifndef PAGING_H
#define PAGING_H

#define PAGE_DIR_SIZE           0x400 //1KB
#define PAGE_TABLE_SIZE         0x400
#define PAGE_SIZE_4K            0x1000 //4kB
#define KERNEL_LOAD_POINT       0x400000 //4MB
#define VID_VIRT_ADDR 			0x8400000 //132MB

#define SET_PAGE_GLOBAL         0x100
#define SET_PAGE_4MB            0x80
#define SET_PAGE_USER           0x4
#define SET_PAGE_RW             0x2
#define SET_PAGE_PRES           0x1

#define PG_TBL_ALIGN            0x1000 //to align pages on 4kB boundaries
#define PG_DIR_ALIGN            0x1000

#define FOUR_MB                 0x400000
#define EIGHT_MB                0x800000
#define EIGHT_KB                0x2000
#define FOUR_GB                 0x100000000
#define BYTE					0x4

extern void init_paging();
uint32_t get_proc_page_dir(uint32_t* proc_page_dir, 
                           uint32_t phys_addr,
                           uint32_t virt_addr);

void set_CR3(uint32_t page_dir_address);

//int32_t remap_4KB_user_page(pcb_t* proc, uint32_t phys_addr, uint32_t virt_addr);

/* page directory memory */
unsigned int page_dir[PAGE_DIR_SIZE] __attribute__((aligned(PG_DIR_ALIGN)));
/* page table for first 4MB of memory */
unsigned int page_table[PAGE_TABLE_SIZE] __attribute__((aligned(PG_TBL_ALIGN)));

#endif
