#include <stdint.h>
#include <arpa/inet.h>

#define NETSTRUCT_dec_uint32_t(n)  uint32_t n;
#define NETSTRUCT_dec_uint16_t(n)  uint16_t n;

#define NETSTRUCT_hton_uint32_t(n)  t->n = htonl(t->n);
#define NETSTRUCT_hton_uint16_t(n)  t->n = htons(t->n);

#define NETSTRUCT_ntoh_uint32_t(n)  t->n = ntohl(t->n);
#define NETSTRUCT_ntoh_uint16_t(n)  t->n = ntohs(t->n);

#define NETSTRUCT_dec(type, name)  NETSTRUCT_dec_##type(name)
#define NETSTRUCT_hton(type, name) NETSTRUCT_hton_##type(name)
#define NETSTRUCT_ntoh(type, name) NETSTRUCT_ntoh_##type(name)

#define NETSTRUCT1(mod, a)       NETSTRUCT_##mod a
#define NETSTRUCT3(mod, a, b, c) NETSTRUCT1(mod, a) NETSTRUCT1(mod, b) NETSTRUCT1(mod, c)

#define NETSTRUCT_GET(_1, _2, _3, _4, NAME, ...) NAME
#define NETSTRUCT(name, ...)  \
    typedef struct name { \
        NETSTRUCT_GET(__VA_ARGS__, NETSTRUCT4, NETSTRUCT3, NETSTRUCT2, NETSTRUCT1) \
            (dec, __VA_ARGS__) \
    } __attribute__((__packed__)) name; \
    \
    void name##_hton(struct name *t) { \
        NETSTRUCT_GET(__VA_ARGS__, NETSTRUCT4, NETSTRUCT3, NETSTRUCT2, NETSTRUCT1) \
            (hton, __VA_ARGS__) \
    } \
    \
    void name##_ntoh(struct name *t) { \
        NETSTRUCT_GET(__VA_ARGS__, NETSTRUCT4, NETSTRUCT3, NETSTRUCT2, NETSTRUCT1) \
            (ntoh, __VA_ARGS__) \
    }

NETSTRUCT(req_file,
          (uint32_t, start_pos),
          (uint32_t, byte_count),
          (uint16_t, name_len)
);

NETSTRUCT(res_file,
          (uint32_t, length)
);

NETSTRUCT(res_list,
          (uint32_t, length)
);

NETSTRUCT(res_error,
          (uint32_t, type)
);

typedef enum res_error_type {
  BAD_FILE_NAME = 1,
  BAD_FILE_PTR,
  BAD_FILE_SIZE
} res_error_type;

typedef enum dto_type {
  RES_LIST, RES_ERR, RES_FILE, REQ_LIST, REQ_FILE
} dto_type;