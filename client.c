#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "err.h"
#include "dto.h"

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

int main(int argc, char *argv[]) {
  int sock = initialize(argv[1], argv[2]);

  uint16_t header = htons(1);
  write(sock, &header, sizeof(uint16_t));
  int file_id, begin, size;
  while (scanf("%d %d %d", &file_id, &begin, &size) == 3) { // read all numbers from the standard input
    header = htons(2);
    sock = initialize(argv[1], argv[2]);
    write(sock, &header, sizeof(uint16_t));
    req_file req;
    req.name_len = file_id;
    req.start_pos = begin;
    req.byte_count = size;
    req_file bik = req;
    req_file_hton(&bik);
    if (write(sock, &bik, sizeof(req_file)) != sizeof(req_file)) {
      syserr("partial / failed write");
    }
    printf("Sent\n");
    char buff[1000];
    memset(buff, 'a', 1000);
    if (write(sock, buff, req.byte_count) != req.byte_count) syserr("pizdec");

  }

  if (close(sock) < 0) // file_desc would be closed anyway when the program ends
    syserr("close");

  return 0;
}
