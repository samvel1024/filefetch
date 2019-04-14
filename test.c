#include "dto.h"
#include <memory.h>
#include <event.h>
#include <stdlib.h>
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

int main() {
  test_serialize_deserialize();
}
