#ifndef _PROCESS_H
#define _PROCESS_H

uint32_t new_process();
pcb_t* get_process(uint32_t pid);
uint32_t get_curprocess();
uint32_t del_process();

#endif
