#ifndef _PROCESS_H
#define _PROCESS_H

#include "types.h"

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

#endif
