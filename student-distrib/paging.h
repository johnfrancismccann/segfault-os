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
 *
 *
 */
extern void init_paging();

#endif

