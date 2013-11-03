#include "fs.h"
#include "lib.h"

#define MAX_FNAME_LENGTH 32

/* filesystem parameters */
#define FS_LOC              0x40D000
#define DIR_ENTRY_SZ        64
#define BT_BLK_OFF          0
#define BLK_SZ              4096
#define INODES_BLK_OFF      1
/* boot block parameters */
#define NUM_DIR_OFF     0
#define NUM_INODE_OFF   4
#define NUM_DBLKS_OFF   8
/* inode parameters */
#define IN_FL_LEN_OFF       0
#define IN_FIRST_DB_OFF     4
#define IN_DB_IND_SZ        4
/* directory entry parameters */
#define FL_NAME_OFF     0
#define FL_NAME_MX_SZ   32
#define FL_TYPE_OFF     32
#define FL_INODE_OFF    36

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))

#define TYPE_USER 0
#define TYPE_DIR 1
#define TYPE_REG 2

//dentry_t defined in types.h

void read_directly(){
    
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
int32_t read_dentry_by_name (const uint8_t * fname, dentry_t * dentry){
    /* variable declarations */
    uint32_t fname_size;
    uint8_t* file_sys;
    uint32_t num_dir_ent;
    uint32_t i;

    if (*((int8_t*)fname) == ".")
        read_directly();

    /* get the size of the file, and perform checks */
    fname_size = strlen((int8_t*)fname);
    if(!fname_size)
        /* the name of a file can't be a null string. return failure */
        return -1;
    if(fname_size > FL_NAME_MX_SZ)
        /* the maximum size of a file name is 32 characters. return failue */
        return -1;

    /* get file system location */
    file_sys = (uint8_t*)FS_LOC;
    /* get number of entries in directory */
    num_dir_ent = *((uint32_t*)(file_sys+BT_BLK_OFF*BLK_SZ+NUM_DIR_OFF));

    for(i=1; i<num_dir_ent; i++) {
        /* check if fname matches with iterated directory entry filename */
        if(!strncmp((int8_t*)fname, 
                            ((int8_t*)(file_sys+i*DIR_ENTRY_SZ+FL_NAME_OFF)), 
                            fname_size)) {
            /* fill directory entry */
            dentry->file_name = (uint8_t*)fname;
            dentry->ftype = *((uint32_t*)(file_sys + i*DIR_ENTRY_SZ + FL_TYPE_OFF));
            dentry->index_node = *((uint32_t*)(file_sys + i*DIR_ENTRY_SZ + FL_INODE_OFF));
            dentry->length = *((uint32_t*)(file_sys + BLK_SZ + (dentry->index_node) * BLK_SZ));
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

    uint8_t* file_sys;
    uint32_t num_inodes;
    uint32_t i;

    /* get memory location of filesystem */
    file_sys = (uint8_t*)FS_LOC;
    /* get number of inodes */
    num_inodes = *((uint32_t*)(file_sys+NUM_INODE_OFF));

    /* check that passed index is within valid range of inode indices */
    if(index >= num_inodes)
        /* return failure */
        return -1;

    /* iterate over direcotry entries */
    for(i=1; i<num_inodes; i++) {
        /* if index matches directory entry index */
        if(index == *((uint32_t*)(file_sys+i*DIR_ENTRY_SZ+FL_INODE_OFF))) {
            /* fill dentry if match is found */
            dentry->file_name = (uint32_t*)(file_sys+i*DIR_ENTRY_SZ+FL_NAME_OFF);
            dentry->ftype = *((uint32_t*)(file_sys+i*DIR_ENTRY_SZ+FL_TYPE_OFF));
            dentry->index_node = index;
            dentry->length = *((uint32_t*)(file_sys + BLK_SZ + (dentry->index_node) * BLK_SZ));
            /* return success */
            return 0;
        }
    }
    /* inode not found, return failure. this should never be reached */
    return -1;
}


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
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length){

    uint8_t *file_sys;
    uint8_t *inode_loc;
    uint8_t *source_loc;
    uint32_t num_inode;
    uint32_t file_size;
    uint32_t bytes_left;
    uint32_t bytes_read;
    uint32_t data_block_ind;
    uint16_t data_block_off;
    uint16_t cpy_sz;

    /* get memory location of filesystem */
    file_sys = (uint8_t*)FS_LOC;

    num_inode = *((uint32_t*)(file_sys+BT_BLK_OFF*BLK_SZ+NUM_INODE_OFF));
    /* check that inode is within valid inode range */
    if(inode >= num_inode)
        return -1;

    /* get memory location of inode */
    inode_loc = file_sys+(INODES_BLK_OFF+inode)*BLK_SZ;
    /* get file size */
    file_size = *((uint32_t*)(inode_loc+IN_FL_LEN_OFF)); 

    /* check that offset is less than file size */
    if(offset>=file_size)
        return 0;

    /* init bytes_left to smaller of length, number of bytes b/w offset,eof */	
    bytes_left = MIN(length,(uint32_t)(file_size-offset));
    bytes_read = 0;

    while(bytes_left) {
            /* calculate index, offset for calculating source location of copy */
            data_block_ind = (uint32_t)((bytes_read+offset) / BLK_SZ);
            data_block_off = (uint32_t)((bytes_read+offset) % BLK_SZ);
            /* check bytes_left to determine copy size */
            if(!(bytes_left > BLK_SZ))
                cpy_sz = bytes_left;
            else
                cpy_sz = BLK_SZ;

            /* get data block index relative to first data block */	
            data_block_ind = *((uint32_t*)(inode_loc+IN_FIRST_DB_OFF
                                                                    +data_block_ind*IN_DB_IND_SZ));
            /* get source location for copy */
            source_loc = file_sys+(INODES_BLK_OFF+num_inode+data_block_ind)
                                   *BLK_SZ+data_block_off;
            /* copy data to buffer */
            memcpy(buf+bytes_read, source_loc, cpy_sz);
            /* update bytes_left, bytes_read */
            bytes_left = (uint32_t)(bytes_left-cpy_sz);
            bytes_read += cpy_sz;
    }
    return bytes_read;
}
