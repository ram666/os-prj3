#include "fs.h"

int fs_open(char *name)
{

}

int fs_close(int fildes)
{

}

int fs_create(char *name)
{
  int i = 0;
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

}

int fs_read(int fildes, void *buffer, size_t nbyte)
{

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
          return i;
        }
    }
  return -1;
}

int get_first_empty_block()
{
  return get_first_empty_block_from(1);
}

// TODO: change 4 to sizeof(int)

int fs_write(int fildes, void *buffer, size_t nbyte)
{
  int start = 0;

  if (meta->fcb_list[fildes].first_block == 0) {
      int last = get_first_empty_block();
      meta->fcb_list[fildes].first_block = last;

      char *cpy = (char *) malloc(BLOCK_SIZE);

      int i = 0;
      while (nbyte >= BLOCK_SIZE - 4) {
          int next = get_first_empty_block_from(last + 1);
          memcpy(cpy, buffer + (i * (BLOCK_SIZE - 4)), BLOCK_SIZE - 4);
          memcpy(cpy + ((i + 1) * BLOCK_SIZE - 4), &next, 4);

          nbyte -= (BLOCK_SIZE - 4);
          i++;
        }

    }
  else {

    }
}
