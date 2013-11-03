#ifndef _FS_H
#define _FS_H

#include "lib.h"
#include "types.h"
/*
 * File System:
 *
 */
 
 
/*
 * set_fs_loc
 * DESCRIPTION: set file_sys_loc to the location of the file system 
 * INPUTS: base_mods-- pointer to modules structures
 *         num_mods-- the number of modules pointed to by base_mods
 * OUTPUS: none
 * RETURN VALUE: none
 * SIDE_EFFECTS: fs.c scope variable file_sys_loc set to location of 
 *               file system
 */
void set_fs_loc(const uint8_t* base_mods, uint32_t num_mods);
 
 
/*
 * read_dentry_by_name
 *   DESCRIPTION: fills in the dentry_t block passed with file name, file type,
 *                and inode number for the file
 *   INPUTS: const unit8_t * fname, 
 *           dentry_t * dentry
 *   OUTPUTS: contents in dentry
 *   RETURN VALUE: 0
 *                 -1 on failure
 *   SIDE EFFECTS: modify dentry
 */
int32_t read_dentry_by_name (const uint8_t * fname, dentry_t * dentry);


/*
 * read_dentry_by_index
 *   DESCRIPTION: fills in the dentry_t block passed with file name, file type,
 *                and inode number for the file
 *   INPUTS: unit32_t index, 
 *           dentry_t * dentry
 *   OUTPUTS: contents in dentry
 *   RETURN VALUE: 0
 *                 -1 on failure
 *   SIDE EFFECTS: modify dentry 
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t * dentry);


/*
 * read_data
 *   DESCRIPTION: read up to "length" bytes starting from position offset in the file 
 *                with inode number "inode"
 *   INPUTS: unit32_t inode, 
 *           unit32_t offset, 
 *           unit8_t * buf, 
 *           unit length
 *   OUTPUTS: contents in buf
 *   RETURN VALUE: number of bytes read and placed in the buffer;
 *                 0 indicate end of file is reached
 *   SIDE EFFECTS: modify buf
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length);

#endif

