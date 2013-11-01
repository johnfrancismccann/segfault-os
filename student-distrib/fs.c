#include "fs.h"
#include "lib.h"

#define MAX_FNAME_LENGTH 32

#define FS_LOC				0x40D000
#define FS_BT_BLK_OFF	0
#define DIR_ENTRY_SZ 	64

#define FL_NAME_OFF		0
#define FL_TYPE_OFF		32
#define FL_INODE_OFF	36


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

#if 0
    //assume dentry has already been allocated
    //file name should be less or equal than 32, if *fname is longer
    //we'll just copy as long as we can	
	
	//copy name first
    strncpy(dentry->file_name, fname, MAX_FNAME_LENGTH);		
	//copy type
	dentry->ftype = TYPE_REG;//just a test
	//copy inode#
	//dentry->index_node = //inode;
#endif

	/* variable declarations */
	uint32_t fname_size;
	uint8_t* file_sys;
	uint32_t num_dir_ent;
	uint32_t i;
	
	/* get the size of the file, and perform checks */
	fname_size = strlen((int8_t*)fname);
	if(!fname_size)
		/* the name of a file can't be a null string. return failure */
		return -1;
	if(fname_size > 32)
		/* the maximum size of a file name is 32 characters. return failue */
		return -1;
	
	/* get file system location */
	file_sys = (uint8_t*)FS_LOC;
	/* get number of entries in directory */
	num_dir_ent = *((uint32_t*)(file_sys+FS_BT_BLK_OFF));
	
	for(i=1; i<num_dir_ent; i++) {
		/* check if fname matches with iterated directory entry filename */
		if(!strncmp((int8_t*)fname, 
								((int8_t*)(file_sys+i*DIR_ENTRY_SZ+FL_NAME_OFF)), 
								fname_size)) {
			/* fill directory entry */
			dentry->file_name = (uint32_t*)fname;
			dentry->ftype = *((uint32_t*)(file_sys + i*DIR_ENTRY_SZ + FL_TYPE_OFF));
			dentry->index_node = *((uint32_t*)(file_sys + i*DIR_ENTRY_SZ + FL_INODE_OFF));
			/* return success */
			return 0;
		}
	}
	/* file name does not match any files. return failure */	
  return -1;
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
