#ifndef _PROCESS_H
#define _PROCESS_H

#include "types.h"

#define MAX_PROCESSES 8
#define SCED_ON 0

void launch_scheduler();

int32_t create_proc();

void destroy_proc();

pcb_t* get_cur_proc();

/* function to modify current pcb */
void set_arguments(const int8_t* arguments, uint8_t size);

void set_par_ebp(uint32_t par_ebp);

/* functions to access current pcb fields */
uint32_t get_page_dir();

uint32_t get_ebp();

uint32_t get_vid_mapped(uint32_t term_index);

/* remap 4k page of process running in term_index */
int32_t remap_4KB_user_page(uint32_t term_index, uint32_t phys_addr, uint32_t virt_addr);

uint32_t get_num_processes();

#if !(SCED_ON)
void switch_term_proc(uint32_t new_term);
#endif

#endif
