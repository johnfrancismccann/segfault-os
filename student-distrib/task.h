#ifndef _TASK_H
#define _TASK_H

#include "paging.h"
#include "fs.h"
#include "lib.h"
#include "x86_desc.h"
#include "syscalls.h"

//task structures
typedef struct
{
    tss_t task_tss;
    seg_desc_t tss_dec;	
} struct_task;
//task structures end

#endif

