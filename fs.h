#ifndef __FS_H_
#define __FS_H_

#include <unistd.h>
#include "disk.h"

extern struct metadata *meta;

int fs_open(char *name);
int fs_close(int fildes);
int fs_create(char *name);
int fs_delete(char *name);
int fs_read(int fildes, void *buffer, size_t nbyte);
int fs_write(int fildes, void *buffer, size_t nbyte);

#endif
