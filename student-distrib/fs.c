#include "fs.h"
#include "lib.h"

#define FS_LOC				0x40D000
#define FS_BT_BLK_OFF	0
#define DIR_ENTRY_START 64 //FS_BT_BLK_OFF + DIR_ENTRY_START gives the start location of the first entry of dentry
#define DIR_ENTRY_SZ 	64

#define FL_NAME_OFF		0
#define FL_TYPE_OFF		32
#define FL_INODE_OFF	36
#define FL_READ_MASK    0xFF000000

#define INODE_OFF		0x1000 //because each block is 64kb

#define TYPE_USER       0
#define TYPE_DIR        1
#define TYPE_REG        2

#define BLOCK_BYTE_NUM   4
#define DATA_BLOCK_START 1
#define MAX_FNAME_LENGTH 32
#define FILE_EMPTY       0
#define BIT_PER_BYTE     8

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
	if(fname_size > MAX_FNAME_LENGTH)
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

	/* variable declarations */
	uint32_t fname_size;
	uint8_t* file_sys;
	uint32_t num_dir_ent;
	uint32_t i;

	/* get file system location */
	//do this first because an index was passed in
	file_sys = (uint8_t*)FS_LOC;
	
	/* get the size of the file, and perform checks */
	fname_size = strlen((int8_t*)(file_sys + FS_BT_BLK_OFF + DIR_ENTRY_START + FL_NAME_OFF));
	if(!fname_size)
		/* the name of a file can't be a null string. return failure */
		return -1;
	if(fname_size > MAX_FNAME_LENGTH)
		/* the maximum size of a file name is 32 characters. return failue */
		return -1;
	
	/* get number of entries in directory */
	num_dir_ent = *((uint32_t*)(file_sys+FS_BT_BLK_OFF));
	
	for(i=1; i<num_dir_ent; i++) {
		/* check if fname matches with iterated directory entry filename */
		if(!strncmp((int8_t*)(file_sys + FS_BT_BLK_OFF + DIR_ENTRY_START + FL_NAME_OFF), 
								((int8_t*)(file_sys+i*DIR_ENTRY_SZ+FL_NAME_OFF)), 
								fname_size)) {
			/* fill directory entry */
			dentry->file_name = (uint32_t*)(file_sys + FS_BT_BLK_OFF + DIR_ENTRY_START + FL_NAME_OFF);
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
 * read_data
 *   DESCRIPTION: read up to "length" bytes starting from position offset in the file 
 *                with inode number "inode"
 *   INPUTS: unit32_t inode,  ---inode # of the file read
 *           unit32_t offset, ---starting place in the file
 *			 unit8_t * buf, 
 *			 unit length
 *   OUTPUTS: contents in buf
 *   RETURN VALUE: number of bytes read and placed in the buffer;
 *                 0 indicate end of file is reached
 *   SIDE EFFECTS: modify buf							 
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length){

    /* variable declarations */
	uint8_t* file_sys;
	uint8_t* file_start;
    uint32_t file_length;
	
	uint32_t i;
	uint32_t j;
	
	/* get file system location */
	file_sys = (uint8_t*)FS_LOC;
	
	/*The start of the file, which points to "length in B" field*/
	file_start = file_sys + INODE_OFF + inode * INODE_OFF;
	
	/*to check how many bytes of data we can read*/
	file_length = *((uint32_t*)file_start);
	
	/*return 0 if reach end*/
	if ((file_length*BLOCK_BYTE_NUM) < offset)
	    return 0;
	
	/*That means we will reach the end before reading the (length)th byte*/
	//IMPORTANT: I assume offset 0 means the beginning. The return value may need modification
	if ((file_length*BLOCK_BYTE_NUM) < (offset+length)){
        for (i = 0; i < (file_length*BLOCK_BYTE_NUM - offset); i++){
		    for (j = 0; j < BLOCK_BYTE_NUM; j++){
  	            buf[i*BLOCK_BYTE_NUM + j] = (*((uint32_t*)(file_start + DATA_BLOCK_START + i))) * (FL_READ_MASK >> (BIT_PER_BYTE*j));
			}
	    }
	    return (file_length*BLOCK_BYTE_NUM - offset);
	}
	
	//then this is the normal reading, no out-of-range check;
    for (i = 0; i < length; i++){
	    for (j = 0; j < BLOCK_BYTE_NUM; j++){
  	        buf[i*BLOCK_BYTE_NUM + j] = (*((uint32_t*)(file_start + DATA_BLOCK_START + i))) * (FL_READ_MASK >> (BIT_PER_BYTE*j));
	    }
	}	
    return length;
}
