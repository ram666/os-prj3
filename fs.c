#include "fs.h"
#include <string.h>

int fs_open(char *name)
{
  int i = 0;
  for (; i < MAX_FILE_COUNT; i++) {
      if (strcmp(meta->fcb_list[i].file_name, name) == 0) {
          meta->fcb_list[i].is_opened = 1;
          return i;
        }
    }
  return -1;
}

int fs_close(int fildes)
{
  meta->fcb_list[fildes].is_opened = 0;
}

int fs_create(char *name)
{
  int i = 0;
  for (; i < MAX_FILE_COUNT; i++) {
      if (strcmp(name, meta->fcb_list[i].file_name) == 0)
        return -1;
    }

  i = 0;
  for (; i < MAX_FILE_COUNT; i++) {
      if (meta->vcb.free_fcb[i] == 0)
        break;
    }
  if (i == MAX_FILE_COUNT)
    return -1;

  meta->vcb.free_fcb[i] = 1;
  strcpy(meta->fcb_list[i].file_name, name);

  return 0;
}

int fs_delete(char *name)
{
  int i = 0;
  for (; i < MAX_FILE_COUNT; i++) {
      if (strcmp(name, meta->fcb_list[i].file_name) == 0)
        break;
    }
  if(meta->fcb_list[i].is_opened == 0)
    return -1;
  if(meta->fcb_list[i].size == 0)
    return -1;

  int current_block = meta->fcb_list[i].first_block;
  char *buffer = (char*)malloc(BLOCK_SIZE);
  int next_block = current_block;
  do{
      current_block = next_block ;
      block_read(current_block,buffer);
      next_block = get_block_icon(buffer);
      memset(buffer,0,BLOCK_SIZE);
      block_write(current_block,buffer);
      meta->vcb.free_block_count ++;

    }while(next_block >0);
  meta->vcb.free_fcb[i] = 0;
  memset(meta->fcb_list[i].file_name,0,20);
  meta->fcb_list[i].first_block = 0;
  meta->fcb_list[i].last_block = 0;
  meta->fcb_list[i].last_block_used = 0;
  meta->fcb_list[i].size = 0;
}

int fs_read(int fildes, void *buffer, size_t nbyte)
{
  if (meta->fcb_list[fildes].is_opened == 0)
    return -1;

  if (meta->fcb_list[fildes].size < nbyte)
    nbyte = meta->fcb_list[fildes].size;

  int block = meta->fcb_list[fildes].first_block;

  if (block == 0)
    return -1;

  char *temp = (char*)malloc(BLOCK_SIZE);

  int i = 0;
  while (nbyte > BLOCK_SIZE - 4) {
      block_read(block, temp);
      memcpy(buffer + (i * (BLOCK_SIZE - 4)), temp, BLOCK_SIZE - 4);

      block = get_block_icon(temp);
      nbyte -= BLOCK_SIZE - 4;
      i++;
    }
  block_read(block, temp);
  memcpy(buffer + (i * (BLOCK_SIZE - 4)), temp, nbyte);

  return 0;
}

int get_block_icon(char *buffer)
{
  return * ((int *) (buffer + BLOCK_SIZE - 4));
}

int get_first_empty_block_from(int i)
{
  char *buffer = (char *) malloc(BLOCK_SIZE);

  for (; i < DISK_BLOCKS; i++) {
      block_read(i, buffer);
      if (get_block_icon(buffer) == 0) {
          free(buffer);
          return i;
        }
    }
  free(buffer);
  return -1;
}

int get_first_empty_block()
{
  return get_first_empty_block_from(1);
}

/**
 * @brief write data of buffer into a block, from the beginning of block
 * @param last  the last block used for writing (0 for searching from start)
 * @param nbyte
 * @return last block used in file
 */
int write_starting_from_block(int last, char *buffer, int *n_byte)
{
  int next, nbyte = *n_byte;
  char *buff = (char *) malloc(BLOCK_SIZE);

  int i = 0;
  while (nbyte > BLOCK_SIZE - 4) {
      next = get_first_empty_block_from(last + 1);
      memcpy(buff, buffer + (i * (BLOCK_SIZE - 4)), BLOCK_SIZE - 4);
      memcpy(buff + ((i + 1) * BLOCK_SIZE - 4), &next, 4);
      block_write(last, buff);
      last = next;
      nbyte -= (BLOCK_SIZE - 4);
      i++;
    }

  next = -1;
  memset(buff, 0, BLOCK_SIZE);
  memcpy(buff, buffer + (i * (BLOCK_SIZE - 4)), nbyte);
  memcpy(buff + ((i + 1) * BLOCK_SIZE - 4), &next, 4);
  block_write(last, buff);

  return last;
}

// TODO: change 4 to sizeof(int)

int fs_write(int fildes, void *buffer, size_t nbyte)
{
  if (meta->fcb_list[fildes].is_opened == 0)
    return -1;

  if (meta->vcb.free_block_count < (nbyte / BLOCK_SIZE) + 1)
    return -1;

  meta->fcb_list[fildes].size += nbyte;

  if (meta->fcb_list[fildes].first_block == 0) {
      int last = get_first_empty_block();
      meta->fcb_list[fildes].first_block = last;

      last = write_starting_from_block(last - 1, buffer, &nbyte);

      meta->fcb_list[fildes].last_block = last;
      meta->fcb_list[fildes].last_block_used = nbyte;

      metadata_rewrite();
    }
  else {
      char *buff = (char *) malloc(BLOCK_SIZE);

      if (meta->fcb_list[fildes].last_block_used + 4 + nbyte <= BLOCK_SIZE) {
          block_read(meta->fcb_list[fildes].last_block, buff);
          memcpy(buff + (meta->fcb_list[fildes].last_block_used), buffer, nbyte);
          block_write(meta->fcb_list[fildes].last_block, buff);

          meta->fcb_list[fildes].last_block_used += nbyte;

          metadata_rewrite();
        }
      else {
          int last = meta->fcb_list[fildes].last_block;

          block_read(last, buff);
          memcpy(buff + (meta->fcb_list[fildes].last_block_used),
                 buffer,
                 (BLOCK_SIZE - 4 - meta->fcb_list[fildes].last_block_used));

          int next = get_first_empty_block();
          memcpy(buff + (BLOCK_SIZE - 4), &next, 4);
          block_write(last, buff);

          last = next;
          nbyte -= (BLOCK_SIZE - 4 - meta->fcb_list[fildes].last_block_used);

          last = write_starting_from_block(last, buffer, &nbyte);

          meta->fcb_list[fildes].last_block = last;
          meta->fcb_list[fildes].last_block_used = nbyte;

          metadata_rewrite();
        }
    }
}
