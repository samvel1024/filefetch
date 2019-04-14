#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

struct for_each_file_acc {
  int file_count;
  int file_lenght;
  int curr_file;
  int file_desc;
  char buffer[1024];
};

int for_each_file_measure(struct for_each_file_acc *acc, struct dirent *d) {
  if (d->d_type == DT_REG) {
    acc->file_count += 1;
    acc->file_lenght += strlen(d->d_name);
  }
  return 0;
}

int for_each_file_send(struct for_each_file_acc *acc, struct dirent *d) {
  if (d->d_type == DT_REG) {
    int len = strlen(d->d_name);
    memcpy(acc->buffer, d->d_name, len);
    if (acc->curr_file < acc->file_count - 1) {
      acc->buffer[len] = '|';
      len += 1;
    }
    if (write(acc->file_desc, acc->buffer, len) < 0) {
      return -1;
    }
    acc->curr_file ++;
  }
  return 0;
}

int for_each_file(char *in_dir, void *accumulator, int (*func)(struct for_each_file_acc *, struct dirent *)) {
  struct dirent *in_file;
  DIR *dir;
  if ((dir = opendir(in_dir)) == NULL) return -1;
  while ((in_file = readdir(dir))) {
    int status = func(accumulator, in_file);
    if (status < 0) return status;
  }
  closedir(dir);
  return 0;
}

#endif

