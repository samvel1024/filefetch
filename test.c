#include "dto.h"
#include <memory.h>
#include <event.h>
#include <stdlib.h>
#include "util.h"
#include <assert.h>

#define CHECK_EQUAL(a, b) assert(a == b)

void test_serialize_deserialize() {
  char buff[100];
  memset(buff, 0, 100);
  req_file origin;
  origin.name_len = 10;
  origin.byte_count = 20;
  origin.start_pos = 30;
  req_file_hton(&origin);
  memcpy(buff, &origin, sizeof(req_file));

  req_file test;
  memcpy(&test, buff, sizeof(test));
  req_file_ntoh(&test);
  CHECK_EQUAL(10, test.name_len);
  CHECK_EQUAL(20, test.byte_count);
  CHECK_EQUAL(30, test.start_pos);
}

void test_scan() {
  struct for_each_file_acc acc;
  memset(&acc, 0, sizeof(acc));
  acc.file_desc = STDOUT_FILENO;
  for_each_file("./", &acc, for_each_file_measure);
  for_each_file("./", &acc, for_each_file_send);
}

void test_segment() {
  char buff[100];
  char *str = "ar|ba|bik|jpuct";
  assert(get_file_name(2, str, buff) == 3);
  assert(strcmp(buff, "bik") == 0);

  assert(get_file_name(3, str, buff) == 5);
  assert(strcmp(buff, "jpuct") == 0);

  assert(get_file_name(4, str, buff) == -1);
}

int test_buffered_read(){
  int fd;
  IF_NEGATIVE_RETURN(fd = open("CMakeCache.txt", O_RDWR | O_CREAT, S_IRUSR));
  lseek(fd, 10000000, SEEK_SET);
  char buf[100];
  struct buffered_reader
      br = {.buffer_max_size = 2, .buffer = buf, .bytes_to_read = 20, .source_fd = fd, .buffer_filled = 0};

  while (br.bytes_to_read) {
    IF_NEGATIVE_RETURN(read_to_buffer(&br));
    write(STDOUT_FILENO, br.buffer, br.buffer_filled);
  }
  return 0;

}

int main() {
//  test_scan();
//  test_buffered_read();
//  test_segment();
//  test_serialize_deserialize();
  int fd;
  IF_NEGATIVE_RETURN(fd = open("bikct", O_RDWR));
  lseek(fd, 10000000, SEEK_SET);
  off_t fsize = lseek(fd, 0, SEEK_END);
}
