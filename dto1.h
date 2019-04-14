#ifndef NETSTORE_DTO_H
#define NETSTORE_DTO_H
#include <assert.h>
#include <stdint.h>

typedef enum res_error_type {
  BAD_FILE_NAME = 1,
  BAD_FILE_PTR,
  BAD_FILE_SIZE
} res_error_type;

typedef enum dto_type {
  RES_LIST, RES_ERR, RES_FILE, REQ_LIST, REQ_FILE
} dto_type;


//*********** FILE ******************

typedef struct __attribute__((__packed__)) req_file {
  uint32_t start_pos;
  uint32_t byte_count;
  uint16_t name_len;
} req_file;

typedef struct __attribute__((__packed__)) res_file {
  uint32_t length;
} res_file;

//*********** LIST ******************

typedef struct __attribute__((__packed__)) res_list {
  uint32_t length;
} res_list;

//*********** ERROR ******************

typedef struct __attribute__((__packed__)) res_error {
  uint32_t type;
} res_error;

int marshall_dto(void *dto, char *buff, dto_type type);
int unmarshall_request_dto(char *buff, void *dto, dto_type *type);
int unmarshall_response_dto(char *buff, void *dto, dto_type *type);



#endif //NETSTORE_DTO_H
