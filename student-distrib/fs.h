#ifndef _FS_H
#define _FS_H

#include "lib.h"
#include "types.h"

/* file types */
#define TYPE_USER 0
#define TYPE_DIR 1
#define TYPE_REG 2

int32_t fs_open_file(const uint8_t* filename);

int32_t fs_read_file(void* buf, int32_t nbytes);

int32_t fs_write_file(void* buf, int32_t  nbytes);

int32_t fs_close_file();

int32_t fs_open_dir(const uint8_t* filename);

int32_t fs_read_dir(void* buf, int32_t nbytes);

int32_t fs_write_dir(void* buf, int32_t  nbytes);

int32_t fs_close_dir();  
 
int32_t set_fs_loc(const uint8_t* base_mods, uint32_t num_mods);
 
int32_t read_dentry_by_name (const uint8_t * fname, dentry_t * dentry);

int32_t read_dentry_by_index (uint32_t index, dentry_t * dentry);

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length);

int32_t load_file(const int8_t* filename, void* buf, int32_t nbytes);

extern syscall_func_t filefops_table[4];

extern syscall_func_t dirfops_table[4];

#endif

