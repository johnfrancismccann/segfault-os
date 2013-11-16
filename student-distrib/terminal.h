/*
 *  The terminal files allow the terminal to read and write to/from user/kernel space.
 */

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"

#define NO 0
#define YES 1

void set_termfops();

int32_t term_open();

int32_t term_close();

int32_t term_read(void* read_ptr, int32_t nbytes);

int32_t term_write(const void* wrt_ptr, int32_t nbytes);

#endif
