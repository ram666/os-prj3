#include "disk.h"
#include "fs.h"
#include <stdio.h>

int main() {
  printf("make:  %d\n", make_fs("test_disk.fs"));
  printf("mount: %d\n", mount_fs("test_disk.fs"));

  printf("fs_create: %d\n", fs_create("salam.txt"));
  int filedes = fs_open("salam.txt");
  printf("fs_open:   %d\n", filedes);
  printf("fs_write:  %d\n", fs_write(filedes, "Salam Chetori???", 17));
  char *buffer = (char *) malloc(200);
  printf("fs_read:   %d\n", fs_read(filedes, buffer, 200));
  printf("read: %s\n", buffer);
  printf("fs_close:  %d\n", fs_close(filedes));

  printf("umount:%d\n", umount_fs("test_disk.fs"));

  return 0;
}
