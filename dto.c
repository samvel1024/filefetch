#include "dto.h"
#include <assert.h>
#include <stdbool.h>
#include <event.h>
#include <memory.h>

#define read(funcname, type, converter) \
type funcname(char *begin, int *len){\
  type net;\
  memcpy(&net, (begin+*len), sizeof(type));\
  *len += sizeof(type);\
  return converter(net);\
}

#define write(funcname, type, converter) \
void funcname(char *begin, type val, int *len){\
  type net = converter(val);\
  memcpy(begin + *len, &net, sizeof(type));\
  *len += sizeof(type);\
}

read(read2, uint16_t, ntohs)
read(read4, uint32_t, ntohl)
write(write2, uint16_t, htons)
write(write4, uint32_t, ntohl)

const uint16_t type_to_header[] = {1, 2, 3, 1, 2};

int marshall_dto(void *dto, char *buff, dto_type type) {
  int len = 0;
  write2(buff, type_to_header[type], &len);
  switch (type) {
    case REQ_FILE: {
      req_file *d = dto;
      write4(buff, d->start_pos, &len);
      write4(buff, d->byte_count, &len);
      write2(buff, d->name_len, &len);
      break;
    }
    case REQ_LIST: {
      break;
    }
    case RES_FILE: {
      res_file *d = dto;
      write4(buff, d->length, &len);
      break;
    }
    case RES_ERR: {
      res_error *d = dto;
      write4(buff, d->type, &len);
      break;
    }
    case RES_LIST: {
      res_list *d = dto;
      write4(buff, d->length, &len);
      break;
    }
    default: assert(false && "Illegal dto type");
  }
  return len;
}

int unmarshall_request_dto(char *buff, void *dto, dto_type *t) {
  int len = 0;
  uint16_t header = read2(buff, &len);
  switch (header) {
    case 1: {
      *t = REQ_LIST;
      break;
    }
    case 2: {
      *t = REQ_FILE;
      req_file *d = dto;
      d->start_pos = read4(buff, &len);
      d->byte_count = read4(buff, &len);
      d->name_len = read2(buff, &len);
      break;
    }
    default: assert(false && "Illegal header");
  }
  return len;
}

int unmarshall_response_dto(char *buff, void *dto, dto_type *t) {
  int len = 0;
  uint16_t header = read2(buff, &len);
  switch (header) {
    case 1: {
      res_list *d = dto;
      d->length = read4(buff, &len);
      break;

    }
    case 2: {
      res_error *d = dto;
      d->type = read4(buff, &len);
      break;

    }
    case 3: {
      res_file *d = dto;
      d->length = read4(buff, &len);
      break;

    }
    default: assert(false && "Illegal header");
  }
  return len;
}

void req_file_hton(req_file *d) {
  d->byte_count = ntohl(d->byte_count);
  d->start_pos = ntohl(d->start_pos);
  d->name_len = htons(d->name_len);
}

void req_file_ntoh(req_file *d) {
  d->byte_count = ntohl(d->byte_count);
  d->start_pos = ntohl(d->start_pos);
  d->name_len = ntohs(d->name_len);
}

void res_file_hton(res_file *d) {
  d->length = htonl(d->length);
}
void res_file_ntoh(res_file *d) {
  d->length = ntohl(d->length);
}
