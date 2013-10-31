#include "fs.h"
#include "lib.h"

#define MAX_FNAME_LENGTH 32

#define TYPE_USER 0
#define TYPE_DIR 1
#define TYPE_REG 2

//dentry_t defined in types.h

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
int32_t read_dentry_by_name (const uint8_t * fname, dentry_t * dentry){
    //assume dentry has already been allocated
    //file name should be less or equal than 32, if *fname is longer
    //we'll just copy as long as we can	
	
	//copy name first
    strncpy(dentry->file_name, fname, MAX_FNAME_LENGTH);		
	//copy type
	dentry->ftype = TYPE_REG;//just a test
	//copy inode#
	//dentry->index_node = //inode;
	
    return 0;
}


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
int32_t read_dentry_by_index (uint32_t index, dentry_t * dentry){
    return 0;
}


/*
 * read_data
 *   DESCRIPTION: read up to "length" bytes starting from position offset in the file 
 *                with inode number "inode"
 *   INPUTS: unit32_t inode, 
 *           unit32_t offset, 
 *			 unit8_t * buf, 
 *			 unit length
 *   OUTPUTS: contents in buf
 *   RETURN VALUE: number of bytes read and placed in the buffer;
 *                 0 indicate end of file is reached
 *   SIDE EFFECTS: modify buf							 
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length){
    return 0;
}
