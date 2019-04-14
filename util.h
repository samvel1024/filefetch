#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#define FILE_LIST_BUFF 16842751
#define FILE_NAME_BUFF 10000


#define ELSE_RETURN(call) if ((call) < 0) return -1

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
    acc->buffer[len+1] = '\0';
    if (write(acc->file_desc, acc->buffer, len) < 0) {
      return -1;
    }
    acc->curr_file++;
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

int read_whole_payload(int sock, char *buff, const int size) {
  int read_bytes = 0;
  do {
    int remains = size - read_bytes;
    int just_read = read(sock, buff + read_bytes, remains);
    if (just_read < 0) return -1;
    read_bytes += just_read;
  } while (read_bytes < size);
  assert(read_bytes == size && "read more bytes than needed");
  buff[size] = '\0';
  return size;
}

void pretty_print(char *str, int len) {
  str[len] = '\0';
  int line = 1;
  for (int i = 0; i < len; ++i) {
    printf("%d. ", line);
    while (str[i] != '|' && str[i] != '\0') {
      putchar(str[i]);
      ++i;
    }
    ++line;
    printf("\n");
  }
}


int length(char *ptr, char end){
  int ans = 0;
  while(ptr[ans] != end && ptr[ans] != '\0') ++ans;
  return ans;
}

int get_segment(int index, char *str, char *ans){
  int current = 0;
  for(int i=0; str[i] != '\0'; ++i){
    if (str[i] == '|'){
      current ++;
    }else if (current == index){
      int len = length(str + i, '|');
      memcpy(ans, str + i, len);
      ans [len + 1] = '\0';
      return len;
    }
  }
  return -1;
}

#endif

