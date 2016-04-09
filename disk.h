#ifndef _DISK_H_
#define _DISK_H_

#include <unistd.h>

#define DISK_BLOCKS     8192      /* number of blocks on the disk                */
#define BLOCK_SIZE      4096      /* block size on "disk"    4KB                  */
#define MAX_FILE_COUNT  64

/**
 * @brief The file control block struct
 */
struct fcb {
  char file_name[20];
  unsigned char is_opened;
  unsigned int size;
  unsigned int first_block;
  unsigned int last_block;
  unsigned int last_block_used; // n-bytes used from last block
} __attribute__((packed));

/**
 * @brief The volume control block struct
 */
struct vcb {
  int free_block_count;
  unsigned char free_fcb[MAX_FILE_COUNT];
} __attribute__((packed));

struct metadata {
  int disk_blocks;  // for validation
  struct vcb vcb;
  struct fcb fcb_list[MAX_FILE_COUNT];
} __attribute__((packed));

int make_disk(char *name);     /* create an empty, virtual disk file          */
int open_disk(char *name);     /* open a virtual disk (file)                  */
int close_disk();              /* close a previously opened disk (file)       */

int block_write(int block, char *buf);   /* write a block of size BLOCK_SIZE to disk    */
int block_read(int block, char *buf);    /* read a block of size BLOCK_SIZE from disk   */

int make_fs(char *disk_name);
int mount_fs(char *disk_name);
int umount_fs(char *disk_name);

/**
 * @brief rewrite metadata in disk
 * Assumes the disk is mounted
 * @return -1 in case of error, 0 in case of success
 */
int metadata_rewrite();

#endif
