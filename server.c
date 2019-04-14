#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>

#include "err.h"
#include "dto.h"
#include "util.h"

#define QUEUE_LENGTH     5
#define PORT_NUM     10001

int init_server() {
  struct sockaddr_in server_address;

  int sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP file_desc
  if (sock < 0)
    syserr("file_desc");

  server_address.sin_family = AF_INET; // IPv4
  server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
  server_address.sin_port = htons(PORT_NUM); // listening on port PORT_NUM

  struct linger lin;
  lin.l_onoff = 0;
  lin.l_linger = 0;
  setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *) &lin, sizeof(int));
  if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    syserr("bind");

  // switch to listening (passive open)
  if (listen(sock, QUEUE_LENGTH) < 0)
    syserr("listen");
  return sock;
}

int read_header(int sock, void *buff) {
  uint16_t header;
  if (read(sock, &header, 2) < 0) return -1;
  header = ntohs(header);
  switch (header) {
    case 1: {

      return REQ_LIST;
    }
    case 2: {
      req_file req;
      if (read(sock, &req, sizeof(req)) < 0) return -1;
      req_file_ntoh(&req);
      memcpy(buff, &req, sizeof(req));
      return REQ_FILE;
    }
    default:return -2;
  }
}

int read_whole_payload(int sock, void *buff, const int size) {
  int read_bytes = 0;
  do {
    int remains = size - read_bytes;
    int just_read = read(sock, buff + read_bytes, remains);
    if (just_read < 0) return -1;
    read_bytes += just_read;
  } while (read_bytes < size);
  assert(read_bytes == size && "read more bytes than needed");
  return size;
}

int main(int argc, char *argv[]) {
  int sock = init_server();
  printf("Listening on port %d\n", PORT_NUM);
  for (;;) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    // get client connection from the file_desc
    int msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
    if (msg_sock < 0)
      syserr("accept");

    char header_dto[100];
    int msg_type;
    if ((msg_type = read_header(msg_sock, header_dto)) < 0) return 3;
    switch (msg_type) {
      case REQ_LIST: {
        struct for_each_file_acc acc;
        memset(&acc, 0, sizeof(struct for_each_file_acc));
        acc.file_desc = msg_sock;
        for_each_file("./", &acc, for_each_file_measure);
        //Send header with length
        res_list header;
        header.length = acc.file_lenght + acc.file_count - 1;
        res_list_hton(&header);
        write(msg_sock, &header, sizeof(res_list));
        //Send the payload
        for_each_file("./", &acc, for_each_file_send);
        break;
      }
      case REQ_FILE: {
        req_file *f = (req_file *) header_dto;
        printf("Req file %d %d %d, reading %d bytes\n", f->byte_count, f->name_len, f->start_pos, f->byte_count);
        read_whole_payload(msg_sock, header_dto, f->byte_count);
        printf("Received message %s\n", header_dto);
        break;
      }
      default: return 4;
    }
    printf("ending connection\n");
    if (close(msg_sock) < 0)
      syserr("close");
  }

  return 0;
}
