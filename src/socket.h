#ifndef SOCKET_H
#define SOCKET_H

#include "utils.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define ensure(cond, func, ...)                                                                    \
  ({                                                                                               \
    __auto_type res = func(__VA_ARGS__);                                                           \
    if (!(res cond)) {                                                                             \
      ErrAt(str(func) " failed\n");                                                                \
      perror("");                                                                                  \
      exit(1);                                                                                     \
    }                                                                                              \
    res;                                                                                           \
  })

void *http_server(void *);
void *https_server(void *);

#define HUGE_BUFFERSIZE 1UL << 20
#define FILEBUF_LEN 1UL << 15

#define HTTP_PORT 80
#define HTTPS_PORT 443

#define BACKLOG 32

typedef struct {
  char method[8];
  char url[1024];
  char version[32];
} req_header_t;

static inline void get_header(char *buf, req_header_t *h) {
  sscanf(buf, "%s %s %s", h->method, h->url, h->version);
}

#endif