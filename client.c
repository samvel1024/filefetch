#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "err.h"

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

  // initialize socket according to getaddrinfo results
  int sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);
  if (sock < 0)
    syserr("socket");

  // connect socket to the server
  if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0)
    syserr("connect");

  freeaddrinfo(addr_result);
  return sock;
}

int main(int argc, char *argv[]) {
  int sock = initialize(argv[1], argv[2]);

  int number;

  while (scanf("%d", &number) == 1) { // read all numbers from the standard input

    printf("sending number %d\n", number);

    char *data_to_send = "arbabik";
    if (write(sock, &data_to_send, 7) != 7)
      syserr("partial / failed write");
  }

  if (close(sock) < 0) // socket would be closed anyway when the program ends
    syserr("close");

  return 0;
}
