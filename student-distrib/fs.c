#include "fs.h"
#include "lib.h"
#include "multiboot.h"

#define MAX_FNAME_LENGTH 32

static uint8_t* file_sys_loc;
/* filesystem parameters */
#define FS_BT_IND					0
#define FS_INOD_IND				1
#define FS_BLK_SZ					4096

/* boot block statistics parameters */
#define BT_NUM_DIR_IND		0
#define BT_NUM_INOD_IND		1
#define BT_NUM_DBLKS_IND	2
#define BT_DE_START_IND		1

/* boot block directory entry parameters */
#define DE_ENT_SZ			64
#define DE_FL_MAX_SZ	32
#define DE_NAME_OFF		0
#define DE_TYPE_OFF		32
#define DE_INOD_OFF		36

/* inode parameters */
#define IN_SZ_IND				0
#define IN_FRST_DB_IND		1

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))

#define TYPE_USER 0
#define TYPE_DIR 1
#define TYPE_REG 2

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
void set_fs_loc(const uint8_t* base_mods, uint32_t num_mods) {

	uint32_t i;
	module_t module;
	/* iterate through modules */
	for(i=0; i<num_mods; i++) {
		/* put iterated module in object */
		module = *((module_t*)(base_mods+i*sizeof(module_t)));
		/* check if module is the filesystem */
		if(!strncmp("/filesys_img", (int8_t*)(module.string), 12)) {
			file_sys_loc = (uint8_t*)(module.mod_start);
			return;
		}
	}
}

void read_directly(){

	uint32_t num_dir_ent;
	uint32_t i;
					
	/* get number of entries in directory */
	num_dir_ent = *((uint32_t*)(file_sys_loc+BT_NUM_DIR_IND));
//	printf("%d\n", num_dir_ent);
	for(i=1; i<num_dir_ent; i++) {
		printf("%s\n", ((uint8_t*)(file_sys_loc + i*DE_ENT_SZ)));
	}
}

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
int32_t read_dentry_by_name (const uint8_t * fname, dentry_t * dentry) {
	
	/* variable declarations */
	uint8_t loc_fname[32];
	uint32_t fname_size;
	uint8_t* dir_ent_loc;
	uint32_t num_dir_ent;
	uint32_t i;
			
	/* get the size of the filename, and perform checks */
	fname_size = strlen((int8_t*)fname);
	if(!fname_size)
		/* the name of a file can't be a null string. return failure */
		return -1;
	if(fname_size > DE_FL_MAX_SZ)
		/* the maximum size of a file name is 32 characters. return failue */
		return -1;
	
	/* create local copy of filename */
	strcpy((int8_t*)loc_fname, (int8_t*)fname);
	/* right pad local filename with 0s */
	for(i=fname_size; i<DE_FL_MAX_SZ; i++)
		loc_fname[i] = 0;
	
	/* get number of entries in directory */
	num_dir_ent = ((uint32_t*)(file_sys_loc+FS_BT_IND*FS_BLK_SZ))[BT_NUM_DIR_IND];
	/* iterate through directory entries */
	for(i=BT_DE_START_IND; i<num_dir_ent; i++) {
		/* get location of directory entry for reference */
		dir_ent_loc = file_sys_loc+i*DE_ENT_SZ; 
		/* check if fname matches with iterated directory entry filename */
		if(!strncmp((int8_t*)loc_fname, 
								((int8_t*)(dir_ent_loc+DE_NAME_OFF)), 
								DE_FL_MAX_SZ)) {
			/* if so, fill directory entry */
			strcpy((int8_t*)dentry->file_name, (int8_t*) fname);
			dentry->ftype = *((uint32_t*)(dir_ent_loc+DE_TYPE_OFF));
			dentry->index_node = *((uint32_t*)(dir_ent_loc+DE_INOD_OFF));
			//dentry->length = *((uint32_t*)(file_sys+BLK_SZ+(dentry->index_node)*BLK_SZ));
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

	uint32_t num_inodes;
	uint32_t i;
	uint8_t *dir_ent_loc;
	uint32_t file_type;
	
	/* get number of inodes */
	num_inodes = ((uint32_t*)(file_sys_loc+FS_BT_IND*FS_BLK_SZ))[BT_NUM_INOD_IND];
	/* check that passed index is within valid range of inode indices */
	if(index >= num_inodes)
		/* return failure */
		return -1;
	
	/* iterate over direcotry entries */
	for(i=BT_DE_START_IND; i<num_inodes; i++) {
		/* get location of directory entry */
		dir_ent_loc = file_sys_loc+i*DE_ENT_SZ;
		/* check if index matches iterated directory entry index */
		if(index == *((uint32_t*)(dir_ent_loc+DE_INOD_OFF))) {
			file_type = *((uint32_t*)(dir_ent_loc+DE_TYPE_OFF));
			/* check if directory entry is regular file */
			if(file_type == TYPE_REG) {
				/* fill dentry */
				strcpy((int8_t*)dentry->file_name, (int8_t*) (dir_ent_loc+DE_NAME_OFF));
				dentry->ftype = file_type;
				dentry->index_node = index;
				//dentry->length = *((uint32_t*)(file_sys + BLK_SZ + (dentry->index_node) * BLK_SZ));
				/* return success */
				return 0;
			}
		}
	}
	/* inode not found, return failure */
	return -1;
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

	uint32_t* inode_loc;
	uint8_t* source_loc;
	uint32_t* stat_loc;
 	uint32_t num_inode;
	uint32_t num_data_blks;
	uint32_t file_size;
	uint32_t bytes_left;
	uint32_t bytes_read;
	uint32_t inode_db_ind;
	uint32_t db_ind;
	uint16_t data_block_off;
	uint16_t cpy_sz;
	
	/* get location of statistics entry in boot block */
	stat_loc = (uint32_t*)(file_sys_loc+FS_BT_IND*FS_BLK_SZ);
	/* get number of inodes, data blocks */
	num_inode = stat_loc[BT_NUM_INOD_IND];
	num_data_blks = stat_loc[BT_NUM_DBLKS_IND];
	
	/* check that inode is within valid inode range */
	if(inode >= num_inode)
		return -1;
	
	/* get memory location of inode */
	inode_loc = (uint32_t*)(file_sys_loc+(BT_DE_START_IND+inode)*FS_BLK_SZ);
	/* get file size */
	file_size = inode_loc[IN_SZ_IND];
	//file_size = *((uint32_t*)(inode_loc+IN_FL_LEN_OFF)); 

	/* check that offset is less than file size */												 
	if(offset>=file_size)
		return 0;
		
	/* init bytes_left to smaller of length, number of bytes b/w offset,eof */	
	bytes_left = MIN(length,(uint32_t)(file_size-offset));
	bytes_read = 0;
	
	while(bytes_left) {
			/* calculate index, offset for calculating source location of copy */
			inode_db_ind = (uint32_t)((bytes_read+offset) / FS_BLK_SZ);
			data_block_off = (uint32_t)((bytes_read+offset) % FS_BLK_SZ);
			/* check bytes_left to determine copy size */
			if(!(bytes_left > FS_BLK_SZ))
				cpy_sz = bytes_left;
			else
				cpy_sz = FS_BLK_SZ;

			/* get data block index relative to first data block */	
			db_ind = inode_loc[IN_FRST_DB_IND + inode_db_ind];
			/* check that data block is within valid range of data blocks */
			if(db_ind >= num_data_blks)
				return -1;
			
			/* get base location for this copy of file */
			source_loc = file_sys_loc+(FS_INOD_IND+num_inode+db_ind)
									 *FS_BLK_SZ+data_block_off;
									 
			/* copy data to buffer */
			memcpy(buf+bytes_read, source_loc, cpy_sz);
			/* update bytes_left, bytes_read */
			bytes_left = (uint32_t)(bytes_left-cpy_sz);
			bytes_read += cpy_sz;
	}
    return bytes_read;
}


