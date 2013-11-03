/*
 *  The terminal files allow the terminal to read and write to/from user/kernel space.
 */

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"

int32_t term_open();

int32_t term_close();

int32_t term_read(void* buf);

void term_write(void* buf, int32_t n);

#endif
