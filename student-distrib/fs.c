#include "fs.h"
#include "lib.h"
#include "multiboot.h"

//#define MAX_FNAME_LENGTH 32
/* filesystem file states, parameters */
#define FS_FILE_OPEN        1
#define FS_FILE_CLOSED      0
static uint8_t  fs_file_state = FS_FILE_CLOSED;
static uint32_t fs_file_inode;
static uint32_t fs_file_offset;
/* filesystem directory states, parameters */
#define FS_DIR_OPEN         1
#define FS_DIR_CLOSED       0
static uint8_t  fs_dir_state = FS_DIR_CLOSED;
static uint32_t fs_dir_index;
static uint32_t fs_dir_num_entry;
/* location of filesystem in memory */
static uint8_t* file_sys_loc;

/* filesystem characteristics */
#define FS_BT_IND           0
#define FS_INOD_IND         1
#define FS_BLK_SZ           4096
/* boot block statistics characteristics */
#define BT_NUM_DIR_IND      0
#define BT_NUM_INOD_IND     1
#define BT_NUM_DBLKS_IND    2
#define BT_DE_START_IND     1
/* boot block directory entry characteristics */
#define DE_ENT_SZ       64
#define DE_FL_MAX_SZ    32
#define DE_NAME_OFF     0
#define DE_TYPE_OFF     32
#define DE_INOD_OFF     36
/* inode characteristics */
#define IN_SZ_IND           0
#define IN_FRST_DB_IND      1

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))


syscall_func_t filefops_table[4] = {(syscall_func_t)fs_open_file, 
                                    (syscall_func_t)fs_read_file,
                                    (syscall_func_t)fs_write_file,
                                    (syscall_func_t)fs_close_file};

syscall_func_t dirfops_table[4] = {(syscall_func_t)fs_open_dir, 
                                   (syscall_func_t)fs_read_dir,
                                   (syscall_func_t)fs_write_dir,
                                   (syscall_func_t)fs_close_dir};


/*
 * fs_open_file
 * DESCRIPTION: opens specified file from filesystem
 * INPUTS: filename--name of file to open
 * OUTPUTS: none
 * RETURN_VALUE: 0 on successful open, -1 on failed open
 * SIDE_EFFECTS: If file is opened successfully, set file
 *               state to open
 */ 
int32_t fs_open_file(const uint8_t* filename)
{
    dentry_t dentry;
    int32_t search_res;
    
    /* check input parameter */
    if(filename == NULL)
        return -1;
    
    /* check if file is already opened */
    if(fs_file_state == FS_FILE_OPEN)
        return -1;
    /* if not opened, set file as opened, offset to 0 */
    fs_file_state = FS_FILE_OPEN;
    fs_file_offset = 0;
    
    /* try finding inode number of passed file */
    search_res = read_dentry_by_name(filename, &dentry);
    
    if(!search_res) {
        /* check if filename is name of a directory */
        if(dentry.ftype == TYPE_DIR)
            /* if so, return failure */
            return -1;  
        /* otherwise, set inode number */
        fs_file_inode = dentry.index_node;
    }
    else if(search_res == -1)
        /* return failure if not found */
        return -1;

    return 0;
}

/*
 * fs_open_dir
 * DESCRIPTION: opens specified directory from filesystem
 * INPUTS: filename--name of directory to open
 * OUTPUTS: none
 * RETURN_VALUE: 0 on successful open, -1 on failed open
 * SIDE_EFFECTS: If directory is opened successfully, set
 *               directory state to open. 
 */
int32_t fs_open_dir(const uint8_t* filename)
{
    /* check that filename is not a null pointer */
    if(filename == NULL)
        return -1;
    /* check that directory is pwd */
    else if(filename[0] != '.' || filename[1] != '\0')
        return -1;
        
    /* check if directory is already opened */
    if(fs_dir_state == FS_DIR_OPEN)
        return -1;
    /* if not opened, set directory as open and index to first entry */
    fs_dir_state = FS_DIR_OPEN;
    fs_dir_index = 0;
    fs_dir_num_entry = ((uint32_t*)file_sys_loc+FS_BT_IND)[BT_NUM_DIR_IND];
    
    return 0;
}


/*
 * fs_read_file
 * DESCRIPTION: reads nbytes from file and places them in buf
 * INPUTS: buf--buffer to place read bytes
 *         nbytes--number of bytes to read
 * OUTPUTS: buf-- buffer to which bytes are placed
 * RETURN_VALUE: on successful read, return number of bytes read
 *               on failed read, return -1
 * SIDE_EFFECTS: buffer parameter is filled with the lesser of nbytes
 *               and the number of bytes left to read in file
 */
int32_t fs_read_file(void* buf, int32_t nbytes)
{
    int32_t bytes_read;
    
    /* check that buffer is valid */
    if(buf == NULL)
        return -1;  
    /* check that instructed number of bytes to read is nonnegative */
    if(nbytes < 0)
        return -1;
    /* check that file is opened */
    if(fs_file_state != FS_FILE_OPEN)
        return -1;
    
    /* read bytes from file */
    bytes_read = read_data(fs_file_inode, fs_file_offset, buf, (uint32_t)nbytes);
    /* update offset if reading didn't fail */
    if(bytes_read != -1)
        fs_file_offset += bytes_read; 
    
    return bytes_read;
}

/*
 * fs_read_dir
 * DESCRIPTION: reads next directory entry
 * INPUTS: buf--buffer to place next directory entry
 *               nbytes--number of bytes to read for each directory entry
 * OUTPUTS: buf--buffer to which directory entry is written
 * RETURN_VALUE: if last entry isn't reached, return length of directory entry
 *               if last entry is reached, return 0, return -1 on failure
 * SIDE_EFFECTS: buf is filled with the next directory entry
 */
int32_t fs_read_dir(void* buf, int32_t nbytes)
{
    uint8_t *filename_loc;
    uint32_t filename_len;
    uint8_t *filename[MAX_FNAME_LENGTH+1];
    
    /* check that buffer is valid */
    if(buf == NULL)
        return -1;
    /* check that instructed number of bytes to read is positive */
    if(nbytes <= 0)
        return -1;
    /* check that directory is opened */    
    if(fs_dir_state != FS_DIR_OPEN)
        return -1;
    /* if all directory entries have been read, return 0 */
    if(fs_dir_index >= fs_dir_num_entry)
        return 0;
    
    /* get 32 byte filename of current entry into filename buffer */
    filename_loc = (file_sys_loc+(BT_DE_START_IND+fs_dir_index)*DE_ENT_SZ);
    strncpy((int8_t*)filename, (int8_t*)filename_loc, MAX_FNAME_LENGTH);
    /* the filename could be 32 bytes long. terminate with EOS */
    filename[MAX_FNAME_LENGTH] = '\0'; 
    /* increment directory entry */
    fs_dir_index++;
    
    filename_len = strlen((int8_t*)filename);
    /* copy lesser of filename length,requested length to caller's buffer */
    if( nbytes < filename_len) {
        strncpy((int8_t*)buf, (int8_t*)filename, nbytes);
        return nbytes;
    }
    else {
        strncpy((int8_t*)buf, (int8_t*)filename, filename_len);
        return filename_len;
    }
    
    return -1;
}

/*
 * fs_write_file
 * DESCRIPTION: Filesystem is read-only. doesn't do anything
 * INPUTS: buf--buffer from which bytes are read to write
 *         nbytes--number of bytes to write
 * OUTPUTS: none
 * RETURN_VALUE: always -1 for failure
 * SIDE_EFFECTS: none
 */
int32_t fs_write_file(void* buf, int32_t  nbytes)
{
    /* the filesystem is read only. return failure */
    return -1;
}

/*
 * fs_write_dir
 * DESCRIPTION: Filesystem is read-only. doesn't do anything
 * INPUTS: buf--buffer from which bytes are read to write
 *         nbytes--number of bytes to write
 * OUTPUTS: none
 * RETURN_VALUE: always -1 for failure
 * SIDE_EFFECTS: none
 */
int32_t fs_write_dir(void* buf, int32_t  nbytes)
{
    /* the filesystem is read only. return failure */
    return -1;
}

/*
 * fs_close_file
 * DESCRIPTION: close current file if file is opened
 * INPUTS: none
 * OUTPUTS: none
 * RETURN_VALUE: 0 on successful close, -1 on failed close
 * SIDE_EFFECTS: state of the filesystem's file is changed to 
 *               closed if a file is already opened
 */
int32_t fs_close_file() 
{
    /* files must have been opened to be closed */
    if(fs_file_state != FS_FILE_OPEN)
        return -1;
    /* mark file as closed. must be opened for furthered processing */
    fs_file_state = FS_FILE_CLOSED;
    return 0;
}

/*
 * fs_close_dir
 * DESCRIPTION: close current directory if directory is opened
 * INPUTS: none
 * OUTPUTS: none
 * RETURN_VALUE: 0 on successful close, -1 on failed close
 * SIDE_EFFECTS: state of filesystem's directory is changed to closed
 *               if a directory is already opened
 */
int32_t fs_close_dir()
{
    /* directory must have been opened to be closed */
    if(fs_dir_state != FS_DIR_OPEN)
        return -1;
    /* mark directory as closed. must be opened for furthered processing */
    fs_dir_state = FS_DIR_CLOSED;
    return 0;
}


/*
 * set_fs_loc
 * DESCRIPTION: set file_sys_loc to the location of the file system 
 * INPUTS: base_mods-- pointer to modules structures
 *         num_mods-- the number of modules pointed to by base_mods
 * OUTPUS: none
 * RETURN VALUE: 0 on successful set. -1 on failure
 * SIDE_EFFECTS: fs.c scope variable file_sys_loc set to location of 
 *               file system
 */
int32_t set_fs_loc(const uint8_t* base_mods, uint32_t num_mods) {

    /* return failure if invalid modules pointer argument */
    if(base_mods == NULL)
        return -1;

    uint32_t i;
    module_t module;
    /* iterate through modules */
    for(i=0; i<num_mods; i++) {
        /* put iterated module in object */
        module = *((module_t*)(base_mods+i*sizeof(module_t)));
        /* check if module is the filesystem */
        if(!strncmp("/filesys_img", (int8_t*)(module.string), 12)) {
            file_sys_loc = (uint8_t*)(module.mod_start);
            return 0;
        }
    }
    /* return failure if filesystem is not found */
    return -1;
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
            
    /* check input paramaters */        
    if(fname == NULL || dentry == NULL)
        return -1;
            
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
    
    /* check dentry */
    if(dentry == NULL)
        return -1;
    
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
 *           unit8_t * buf, 
 *           unit length
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
    
    /* check input buffer */
    if(buf == NULL)
        return -1;
    /* check that that length isn't 0 */
    if(!length)
        return -1;
    
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
                cpy_sz = (FS_BLK_SZ-data_block_off);

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


