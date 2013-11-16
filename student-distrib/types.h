/* types.h - Defines to use the familiar explicitly-sized types in this
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H

#define NULL 0

#ifndef ASM

/* Types defined here just like in <stdint.h> */
typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;

typedef int32_t (*syscall_func_t)(void);
typedef int32_t (*syscall_open_t)(const uint8_t* filename);
typedef int32_t (*syscall_read_t)(void* buf, int32_t nbytes);
typedef int32_t (*syscall_write_t)(const void* buf, int32_t nbytes);


#define MAX_FNAME_LENGTH 32
typedef struct
{
  uint8_t file_name[MAX_FNAME_LENGTH];
  uint32_t ftype;
  uint32_t index_node;
} dentry_t;

typedef struct
{
	syscall_func_t* file_ops_table;	
	uint32_t inode_ptr;
	uint32_t file_pos;
	uint32_t flags;  
} file_desc_t;


#define MAX_OPEN_FILES 8
typedef struct //pcb_t
{
	file_desc_t 	file_desc_arr[MAX_OPEN_FILES];
	uint32_t* 		page_dir;
  struct pcb_t* parent_pcb;
  uint32_t*	 		parent_kstack;
  struct pcb_t* child_pcb;
} pcb_t;

#endif /* ASM */

#endif /* _TYPES_H */
