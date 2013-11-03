#ifndef _FS_H
#define _FS_H

#include "lib.h"
#include "types.h"
/*
 * File System:
 *
 */
 
 
 
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

