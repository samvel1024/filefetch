#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>

#include "err.h"
#include "dto.h"
#include "util.h"

#define QUEUE_LENGTH     5
#define PORT_NUM     6543

int init_server(int port) {
  struct sockaddr_in server_address;

  int sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP file_desc
  if (sock < 0)
    syserr("file_desc");

  server_address.sin_family = AF_INET; // IPv4
  server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
  server_address.sin_port = htons(port);

  int yes = 1;
  TRY(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)));
  TRY(bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)));
  // switch to listening (passive open)
  TRY(listen(sock, QUEUE_LENGTH));
  return sock;
}

int send_error(int sock, res_error_type er) {
  type_header h = {.type = RES_ERR};
  TRY(type_header_send(&h, sock));
  res_error err = {.type = er};
  TRY(res_error_send(&err, sock));
  return 0;
}

int resp_len(int from, int byte_count, int fsize) {
  int max = fsize - from;
  return byte_count < max ? byte_count : max;
}

int parse_args(char **dir, int *port, int argc, char *argv[]) {
  if (argc > 3 || argc < 2) {
    fprintf(stderr, "Usage netstore-server <dir> [<port>]\n");
    return -1;
  }
  DIR *d = opendir(argv[1]);
  if (!d) {
    fprintf(stderr, "Could not find the directory\n");
    return -1;
  }
  *dir = argv[1];
  *port = PORT_NUM;
  if (argc == 3) {
    if (1 != sscanf(argv[2], "%d", port)) {
      fprintf(stderr, "Could not read port\n");
      return -1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {

  char *base_dir;
  int port;
  TRY(parse_args(&base_dir, &port, argc, argv));

  int server_socket = init_server(port);
  printf("Listening on port %d\n", port);
  char *read_buff = malloc(READ_WRITE_BUFF_SIZE);
  for (;;) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    // get client connection from the file_d
    int client_sock;
    TRY(client_sock = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len));
    for (;;) {
      type_header head;
      int bytes_read;
      TRY(bytes_read = type_header_receive(client_sock, &head));
      if (bytes_read == 0){
        close(client_sock);
        break;
      }
      switch (head.type) {
        case REQ_LIST: {
          struct for_each_file_acc acc;
          memset(&acc, 0, sizeof(struct for_each_file_acc));
          acc.file_desc = client_sock;
          TRY(for_each_file(base_dir, &acc, for_each_file_measure));
          //Send header with type
          type_header h = {.type = 1};
          TRY(type_header_send(&h, client_sock));
          //Send header with length
          res_list res;
          res.length = acc.file_lenght + acc.file_count - 1;
          res_list_send(&res, client_sock);
          //Send the payload
          if (res.length > 0)
            TRY(for_each_file(base_dir, &acc, for_each_file_send));
          break;
        }
        case REQ_FILE: {
          req_file f;
          TRY(req_file_receive(client_sock, &f));
          read_whole_payload(client_sock, read_buff, f.name_len);
          printf("Requested file size=%d name_len=%d from_pos=%d name=%s\n",
                 f.byte_count,
                 f.name_len,
                 f.start_pos,
                 read_buff);
          //Check for errors
          char file_path[FILE_NAME_BUFF_SIZE];
          sprintf(file_path, "%s/%s", base_dir, read_buff);
          int open_file = open(file_path, O_RDONLY);
          {
            if (f.byte_count == 0) {
              send_error(client_sock, ERR_BAD_FILE_SIZE);
              break;
            }
            if (open_file < 0) {
              send_error(client_sock, ERR_BAD_FILE_NAME);
              break;
            }
            off_t fsize = lseek(open_file, 0, SEEK_END);
            if (f.start_pos >= fsize) {
              send_error(client_sock, ERR_BAD_FILE_PTR);
              break;
            }
          }
          //Send type header and length header
          {
            type_header h = {.type = RES_FILE};
            TRY(type_header_send(&h, client_sock));
            off_t fsize = lseek(open_file, 0, SEEK_END);
            res_file res = {.length = resp_len(f.start_pos, f.byte_count, fsize)};
            TRY(res_file_send(&res, client_sock));
          }
          //Send the file
          {
            lseek(open_file, f.start_pos, SEEK_SET);
            struct buffered_reader br;
            buffered_reader_init(&br, open_file, read_buff, READ_WRITE_BUFF_SIZE, f.byte_count);
            while (br.bytes_to_read) {
              TRY(read_to_buffer(&br));
              if (br.buffer_filled > 0)
                TRY(write(client_sock, br.buffer, br.buffer_filled));
            }
          }
          break;
        }
        default: return 4;
      }
    }

  }

  free(read_buff);

  return 0;
}
