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
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>

#define FILE_LIST_BUFF 17000000
#define READ_WRITE_BUFF_SIZE 10000
#define FILE_NAME_BUFF_SIZE 1100
#define PORT_NUM     6543

void error_die();

#define TRY(call) \
if ((call) < 0) { \
  error_die(); \
}

// ***********************************************************************************
// Family of functions for iterating over files in directory, calculating the concatenated length and sending through socket
struct for_each_file_acc {
  int file_count;
  int file_lenght;
  int curr_file;
  int file_desc;
  char buffer[1024];
};

int for_each_file_measure(struct for_each_file_acc *acc, struct dirent *d);
int for_each_file_send(struct for_each_file_acc *acc, struct dirent *d);
int for_each_file(char *in_dir, void *accumulator, int (*func)(struct for_each_file_acc *, struct dirent *));

// ***********************************************************************************
// Family of functions for reading from raw file descriptor using buffer
struct buffered_reader {
  int source_fd;
  char *buffer;
  int buffer_max_size;
  int buffer_filled;
  uint32_t bytes_to_read;
};
void buffered_reader_init(struct buffered_reader *b, int fd, char *buf, uint32_t buf_size, uint32_t byte_count);

int read_to_buffer(struct buffered_reader *br);


// ***********************************************************************************
// Some ad-hoc functions which I needed to test separately

void pretty_print(char *str, int len);
int get_file_name(int index, char *str, char *ans);
int copy_to_sparse_file(int sock, uint32_t begin, uint32_t size, char *file, char *buff);
int read_whole_payload(int sock, char *buff, const int size);

#endif

