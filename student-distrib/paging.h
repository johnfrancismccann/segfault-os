#ifndef PAGING_H
#define	PAGING_H

#define PAGE_DIR_SIZE 1024
#define PAGE_TABLE_SIZE 1024

#define PG_TBL_ALIGN 0x1000
#define PG_DIR_ALIGN 0x1000

void init_paging();


#endif

