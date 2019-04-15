#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "dto.h"
#include "util.h"

const char *ERROR_MSG[4] = {"", "File name does no longer exist", "Invalid begin address", "Invalid size"};

int initialize(char *host, char *port) {
  struct addrinfo addr_hints;
  struct addrinfo *addr_result;
  memset(&addr_hints, 0, sizeof(struct addrinfo));
  addr_hints.ai_family = AF_INET; // IPv4
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = IPPROTO_TCP;
  int err = getaddrinfo(host, port, &addr_hints, &addr_result);
  if (err == EAI_SYSTEM || err != 0) {
    fprintf(stderr, "getaddrinfo: %s", gai_strerror(err));
    exit(EXIT_FAILURE);
  }

  int sock;
  TRY(sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol));

  TRY(connect(sock, addr_result->ai_addr, addr_result->ai_addrlen));

  freeaddrinfo(addr_result);
  return sock;
}

int fetch_file_list(int sock, char *buff) {
  type_header th = {.type = 1};
  TRY(type_header_send(&th, sock));
  type_header resp;
  TRY(type_header_receive(sock, &resp));
  assert(resp.type == 1 && "Unexpected response");
  res_list res;
  TRY(res_list_receive(sock, &res));
  if (res.length > 0) {
    read_whole_payload(sock, buff, res.length);
    buff[res.length] = '\0';
    pretty_print(buff, res.length);
  } else {
    printf("No files to serve");
  }
  return res.length;
}

int fetch_file(int sock, char *file_list) {
  char *file_name = malloc(FILE_NAME_BUFF_SIZE);
  char *read_write_buff = malloc(READ_WRITE_BUFF_SIZE);

  long file_id, begin, end;
  if (scanf("%ld %ld %ld", &file_id, &begin, &end) != 3) {
    printf("Illegal input");
  }
  if (get_file_name(file_id - 1, file_list, file_name) < 0) {
    printf("Illegal file number\n");
    return 1;
  } else if (begin < 0) {
    printf("Illegal starting address\n");
    return 1;
  } else if (end < begin) {
    printf("End address should be greater than begin address\n");
    return 1;
  }

  printf("Trying to fetch file %s\n", file_name);

  //Send type header
  type_header hd = {.type = 2};
  type_header_send(&hd, sock);

  //Send the request
  req_file req;
  req.name_len = get_file_name(file_id - 1, file_list, file_name);
  req.start_pos = begin;
  req.byte_count = end - begin;
  TRY(req_file_send(&req, sock));
  TRY(write(sock, file_name, req.name_len));
  //Expect the response
  type_header rhd;
  TRY(type_header_receive(sock, &rhd));
  switch (rhd.type) {
    case RES_ERR: {
      res_error err;
      TRY(res_error_receive(sock, &err));
      assert(err.type < 4 && err.type > 0 && "Invalid error code");
      printf("%s\n", ERROR_MSG[err.type]);
      break;
    }
    case RES_FILE: {
      res_file fl;
      TRY(res_file_receive(sock, &fl));
      if (fl.length > 0) {
        printf("req_file : %d %d %d\n", req.name_len, req.start_pos, req.byte_count);
        copy_to_sparse_file(sock, req.start_pos, fl.length, file_name, read_write_buff);
        printf("OK: Saved file in tmp folder\n");
      } else {
        printf("OK: Got empty response\n");
      }
      break;
    }
    default: assert(false && "Unexpected response");
  }
  free(file_name);
  free(read_write_buff);
  return 0;
}


int valid_args(int argc, char *argv[], char **port) {
  if (argc > 3 || argc < 2) {
    fprintf(stderr, "Usage netstore-client <dir> [<port>]\n");
    return -1;
  }
  *port = "6543";
  if (argc == 3) {
    int p;
    if (1 != sscanf(argv[2], "%d", &p)) {
      fprintf(stderr, "Could not read port\n");
      return -1;
    }
    *port = argv[2];
  }
  return 0;
}


int main(int argc, char *argv[]) {
  char *file_list = malloc(FILE_LIST_BUFF);
  char *port;
  TRY(valid_args(argc, argv, &port));
  int sock = initialize(argv[1], port);
  int file_len;
  TRY(file_len = fetch_file_list(sock, file_list));
  if (file_len > 0) {
    fetch_file(sock, file_list);
  }
  TRY(close(sock));
  free(file_list);
  return 0;
}
