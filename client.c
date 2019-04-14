#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "err.h"
#include "dto.h"
#include "util.h"


int initialize(char *host, char *port) {
  // 'converting' host/port in string to struct addrinfo
  struct addrinfo addr_hints;
  struct addrinfo *addr_result;
  memset(&addr_hints, 0, sizeof(struct addrinfo));
  addr_hints.ai_family = AF_INET; // IPv4
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = IPPROTO_TCP;
  int err = getaddrinfo(host, port, &addr_hints, &addr_result);
  if (err == EAI_SYSTEM) // system error
    syserr("getaddrinfo: %s", gai_strerror(err));
  else if (err != 0) // other error (host not found, etc.)
    fatal("getaddrinfo: %s", gai_strerror(err));

  // initialize file_desc according to getaddrinfo results
  int sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);
  if (sock < 0)
    syserr("file_desc");

  // connect file_desc to the server
  if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0)
    syserr("connect");

  freeaddrinfo(addr_result);
  return sock;
}

int fetch_file_list(char *host, char *port, char *buff) {
  int sock = initialize(host, port);
  type_header th = {.type = 1};
  ELSE_RETURN(type_header_send(&th, sock));
  type_header resp;
  ELSE_RETURN(type_header_receive(sock, &resp));
  assert(resp.type == 1 && "Unexpected response");
  res_list res;
  ELSE_RETURN(res_list_receive(sock, &res));
  if (res.length > 0){
    read_whole_payload(sock, buff, res.length);
    pretty_print(buff, res.length);
  }
  return 0;
}



int main(int argc, char *argv[]) {
  char *file_list = malloc(FILE_LIST_BUFF);
  char *file_name_buff = malloc(FILE_NAME_BUFF);
  ELSE_RETURN(fetch_file_list(argv[1], argv[2], file_list));

  int file_id, begin, end, sock;
  while (scanf("%d %d %d", &file_id, &begin, &end) == 3) { // read all numbers from the standard input
    sock = initialize(argv[1], argv[2]);

    //Send type header
    type_header hd = {.type = 2};
    type_header_send(&hd, sock);

    //Send the request
    req_file req;
    req.name_len = get_segment(file_id - 1, file_list, file_name_buff);
    req.start_pos = begin;
    req.byte_count = end - begin;
    ELSE_RETURN(req_file_send(&req, sock));
    ELSE_RETURN(write(sock, file_name_buff, req.name_len));
  }

  if (close(sock) < 0) // file_desc would be closed anyway when the program ends
    syserr("close");
  free(file_list);
  return 0;
}
