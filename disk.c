#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


#include "disk.h"

static int active = 0;  /* is the virtual disk open (active) */
static int handle;      /* file handle to virtual disk       */
static char open_disk_name[128];

struct metadata *meta;

int make_disk(char *name)
{ 
  int f, cnt;
  char buf[BLOCK_SIZE];

  if (!name) {
    fprintf(stderr, "make_disk: invalid file name\n");
    return -1;
  }

  if ((f = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
    perror("make_disk: cannot open file");
    return -1;
  }

  memset(buf, 0, BLOCK_SIZE);
  for (cnt = 0; cnt < DISK_BLOCKS; ++cnt)
    write(f, buf, BLOCK_SIZE);

  close(f);

  return 0;
}

int open_disk(char *name)
{
  int f;

  if (!name) {
    fprintf(stderr, "open_disk: invalid file name\n");
    return -1;
  }  
  
  if (active) {
    fprintf(stderr, "open_disk: disk is already open\n");
    return -1;
  }
  
  if ((f = open(name, O_RDWR, 0644)) < 0) {
    perror("open_disk: cannot open file");
    return -1;
  }

  handle = f;
  active = 1;

  return 0;
}

int close_disk()
{
  if (!active) {
    fprintf(stderr, "close_disk: no open disk\n");
    return -1;
  }
  
  close(handle);

  active = handle = 0;

  return 0;
}

int block_write(int block, char *buf)
{
  if (!active) {
    fprintf(stderr, "block_write: disk not active\n");
    return -1;
  }

  if ((block < 0) || (block >= DISK_BLOCKS)) {
    fprintf(stderr, "block_write: block index out of bounds\n");
    return -1;
  }

  if (lseek(handle, block * BLOCK_SIZE, SEEK_SET) < 0) {
    perror("block_write: failed to lseek");
    return -1;
  }

  if (write(handle, buf, BLOCK_SIZE) < 0) {
    perror("block_write: failed to write");
    return -1;
  }

  return 0;
}

int block_read(int block, char *buf)
{
  if (!active) {
    fprintf(stderr, "block_read: disk not active\n");
    return -1;
  }

  if ((block < 0) || (block >= DISK_BLOCKS)) {
    fprintf(stderr, "block_read: block index out of bounds\n");
    return -1;
  }

  if (lseek(handle, block * BLOCK_SIZE, SEEK_SET) < 0) {
    perror("block_read: failed to lseek");
    return -1;
  }

  if (read(handle, buf, BLOCK_SIZE) < 0) {
    perror("block_read: failed to read");
    return -1;
  }

  return 0;
}

int make_fs(char *disk_name)
{
  // check file existance
  if (access(disk_name, F_OK) != -1)
    return -1;

  if (make_disk(disk_name) == 0) {
      // initialize a VCB
      struct vcb _vcb;
      memset(&_vcb, 0, sizeof(struct vcb));
      _vcb.free_block_count = DISK_BLOCKS;

      struct metadata _meta;
      memset(&_meta, 0, sizeof(struct metadata));
      _meta.vcb = _vcb;
      _meta.disk_blocks = DISK_BLOCKS;

      void *temp = malloc(BLOCK_SIZE);
      memset(temp, 0, BLOCK_SIZE);
      memcpy(temp, &_meta, sizeof(struct metadata));

      open_disk(disk_name);
      block_write(0, temp);
      close_disk(disk_name);

      return 0;
    }
  return -1;
}

int mount_fs(char *disk_name)
{
  if (active != 0)
    return -1;
  if (open_disk(disk_name) == -1)
    return -1;

  void *temp = malloc(BLOCK_SIZE);
  memset(temp, 0, BLOCK_SIZE);

  // read metadata from disk and put it in RAM
  block_read(0, temp);
  meta = (struct metadata *) temp;

  // validation for checking true filesystem
  if (meta->disk_blocks != DISK_BLOCKS)
    return -1;

  strcpy(open_disk_name, disk_name);

  return 0;
}

int umount_fs(char *disk_name)
{
  if (active != 1)
    return -1;
  if (strcmp(open_disk_name, disk_name) != 0)
    return -1;

  if (metadata_rewrite() == -1)
    return -1;

  if (close_disk() == -1)
    return -1;

  return 0;
}

int metadata_rewrite()
{
  void *temp = malloc(BLOCK_SIZE);
  memset(temp, 0, BLOCK_SIZE);
  memcpy(temp, meta, sizeof(struct metadata));

  return block_write(0, temp);
}
