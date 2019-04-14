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

  int yes = 1;
  ELSE_RETURN(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)));
  ELSE_RETURN(bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)));
  // switch to listening (passive open)
  ELSE_RETURN(listen(sock, QUEUE_LENGTH));
  return sock;
}

int main(int argc, char *argv[]) {
  int server_socket = init_server();
  printf("Listening on port %d\n", PORT_NUM);
  char *read_buff = malloc(FILE_NAME_BUFF);
  for (;;) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    // get client connection from the file_desc
    int client_sock = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
    if (client_sock < 0)
      syserr("accept");

    type_header head;
    ELSE_RETURN(type_header_receive(client_sock, &head));
    switch (head.type) {
      case REQ_LIST: {
        struct for_each_file_acc acc;
        memset(&acc, 0, sizeof(struct for_each_file_acc));
        acc.file_desc = client_sock;
        for_each_file("./", &acc, for_each_file_measure);
        //Send header with type
        type_header h = {.type = 1};
        ELSE_RETURN(type_header_send(&h, client_sock));
        //Send header with length
        res_list res;
        res.length = acc.file_lenght + acc.file_count - 1;
        res_list_send(&res, client_sock);
        //Send the payload
        for_each_file("./", &acc, for_each_file_send);
        break;
      }
      case REQ_FILE: {
        req_file f;
        ELSE_RETURN(req_file_receive(client_sock, &f));
        printf("Req file %d %d %d, reading %d bytes\n", f.byte_count, f.name_len, f.start_pos, f.byte_count);
        read_whole_payload(client_sock, read_buff, f.name_len);
        printf("Received message %s\n", read_buff);
        break;
      }
      default: return 4;
    }
    printf("ending connection\n");
    if (close(client_sock) < 0)
      syserr("close");
  }

  free(read_buff);

  return 0;
}
