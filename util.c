#include "util.h"

void error_die() {
  int __err = errno;
  fprintf(stderr, "ERROR: %d %s\n", __err, strerror(__err));
  exit(EXIT_FAILURE);
}

int for_each_file_measure(struct for_each_file_acc *acc, struct dirent *d) {
  if (d->d_type == DT_REG) {
    acc->file_count += 1;
    acc->file_lenght += strlen(d->d_name);
  }
  return 0;
}

int for_each_file_send(struct for_each_file_acc *acc, struct dirent *d) {
  if (d->d_type == DT_REG) {
    int len = strlen(d->d_name);
    memcpy(acc->buffer, d->d_name, len);
    if (acc->curr_file < acc->file_count - 1) {
      acc->buffer[len] = '|';
      len += 1;
    }
    acc->buffer[len + 1] = '\0';
    if (write(acc->file_desc, acc->buffer, len) < 0) {
      return -1;
    }
    acc->curr_file++;
  }
  return 0;
}

void concat_paths(char *base, char *rest, char *buff) {
  sprintf(buff, "%s/%s", base, rest);
}

int for_each_file(char *in_dir, void *accumulator, int (*func)(struct for_each_file_acc *, struct dirent *)) {
  struct dirent *in_file;
  DIR *dir;
  char buf[FILE_NAME_BUFF_SIZE];
  if ((dir = opendir(in_dir)) == NULL) return -1;
  while ((in_file = readdir(dir))) {
    concat_paths(in_dir, in_file->d_name, buf);
    struct stat st;
    TRY(stat(buf, &st));
    in_file->d_type = S_ISREG(st.st_mode) ? DT_REG : DT_UNKNOWN;
    int status = func(accumulator, in_file);
    if (status < 0) return status;
  }
  closedir(dir);
  return 0;
}

int read_whole_payload(int sock, char *buff, const int size) {
  int read_bytes = 0;
  do {
    int remains = size - read_bytes;
    int just_read = read(sock, buff + read_bytes, remains);
    if (just_read == 0 && read_bytes == 0) {
      return 0;
    }
    if (just_read < 0) return -1;
    read_bytes += just_read;
  } while (read_bytes < size);
  assert(read_bytes == size && "read more bytes than needed");
  return size;
}

void pretty_print(char *str, int len) {
  str[len] = '\0';
  int line = 1;
  for (int i = 0; i < len; ++i) {
    printf("%d. ", line);
    while (str[i] != '|' && str[i] != '\0') {
      putchar(str[i]);
      ++i;
    }
    ++line;
    printf("\n");
  }
}

int length(const char *ptr, char end) {
  int ans = 0;
  while (ptr[ans] != end && ptr[ans] != '\0') ++ans;
  return ans;
}

int get_file_name(int index, char *str, char *ans) {
  int current = 0;
  for (int i = 0; str[i] != '\0'; ++i) {
    if (str[i] == '|') {
      current++;
    } else if (current == index) {
      int len = length(str + i, '|');
      memcpy(ans, str + i, len);
      ans[len] = '\0';
      return len;
    }
  }
  return -1;
}

void buffered_reader_init(struct buffered_reader *b, int fd, char *buf, uint32_t buf_size, uint32_t byte_count) {
  b->source_fd = fd;
  b->buffer = buf;
  b->buffer_max_size = buf_size;
  b->bytes_to_read = byte_count;
  b->buffer_filled = 0;
  buf[0] = '\0';
}

int read_to_buffer(struct buffered_reader *br) {
  int max_read = br->bytes_to_read < br->buffer_max_size ? br->bytes_to_read : br->buffer_max_size;
  int read_len;
  TRY(read_len = read(br->source_fd, br->buffer, max_read));
  br->buffer[read_len] = '\0';
  br->buffer_filled = read_len;
  br->bytes_to_read -= (read_len > 0 ? read_len : br->bytes_to_read);
  return read_len;
}

int copy_to_sparse_file(int sock, uint32_t begin, uint32_t size, char *file, char *buff) {
  struct buffered_reader br;
  buffered_reader_init(&br, sock, buff, READ_WRITE_BUFF_SIZE, size);
  struct stat st;
  if (stat("tmp", &st) == -1) {
    TRY(mkdir("tmp", 0755));
  } else if (!S_ISDIR(st.st_mode)) {
    fprintf(stdout, "tmp is already created and is not a directory\n");
    return -1;
  }
  char file_path[FILE_NAME_BUFF_SIZE];
  sprintf(file_path, "%s%s", "tmp/", file);
  int fd;
  TRY(fd = open(file_path, O_RDWR | O_CREAT, 0644));
  TRY(lseek(fd, begin, SEEK_SET));
  while (br.bytes_to_read) {
    TRY(read_to_buffer(&br));
    write(fd, br.buffer, br.buffer_filled);
  }
  return 0;
}


