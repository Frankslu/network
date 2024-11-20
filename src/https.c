#include "socket.h"

#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

extern int wait_accept(int fd);
static int init_https_server();
static SSL_CTX *init_SSL();
static void *https_req_handler_loop(void *ssl);
static void filetype(char *fname, char *ftype);
static int need_chunk(char *ftype, size_t flen);
void https_response(SSL *ssl, char *buf, char *url);

void *https_server(void *args) {
  int listenfd = init_https_server();
  SSL_CTX *ctx = init_SSL();
  while (1) {
    int connfd = wait_accept(listenfd);

    SSL *ssl = ensure(!= NULL, SSL_new, ctx);
    ensure(!= 0, SSL_set_fd, ssl, connfd);

    // If you believe in luck, you can try pthread_creat instead of fork
    // pthread_t t;
    // ensure(== 0, pthread_create, &t, NULL, https_req_handler_loop, (void *)ssl);
    // pthread_detach(t);
    pid_t pid = ensure(>=0, fork);
    if (pid == 0) {
      https_req_handler_loop(ssl);
      exit(0);
    } else {
      SSL_shutdown(ssl);
      SSL_free(ssl);
      close(connfd);
    }
  }
}

static int init_https_server() {
  int listenfd;

  listenfd = ensure(>= 0, socket, AF_INET, SOCK_STREAM, 0);
  Log("Creat HTTPS socket");

  struct sockaddr_in servaddr = {
      .sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_ANY), .sin_port = htons(HTTPS_PORT)};

  ensure(>= 0, bind, listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  Log("Bind HTTPS port: %d", HTTPS_PORT);

  ensure(>= 0, listen, listenfd, BACKLOG);
  Log("Listen to HTTPS port: %d", HTTPS_PORT);
  return listenfd;
}

static SSL_CTX *init_SSL() {
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
  ensure(> 0, SSL_CTX_use_certificate_file, ctx, "./keys/cnlab.cert", SSL_FILETYPE_PEM);
  ensure(> 0, SSL_CTX_use_PrivateKey_file, ctx, "./keys/cnlab.prikey", SSL_FILETYPE_PEM);
  ensure(> 0, SSL_CTX_check_private_key, ctx);

  Log("SSL init finish");
  return ctx;
}

static void *https_req_handler_loop(void *ssl) {
  req_header_t h;
  char buf[HUGE_BUFFERSIZE];

  while (1) {
    if (SSL_accept(ssl) == -1) {
      ERR_print_errors_fp(stderr);
    }

    if (ensure(>= 0, SSL_read, ssl, buf, HUGE_BUFFERSIZE) == 0) {
      break;
    }

    LogColor(WHITE, "Request content:\n%s", buf);
    get_header(buf, &h);

    if (strcmp(h.method, "GET") == 0 && strstr(h.version, "HTTP")) {
      https_response(ssl, buf, h.url);
    } else {
      Assert(0, "Do not support");
    }
  }

  Log("Client connection closed.\n");
  int requst = SSL_get_fd(ssl);
  SSL_shutdown(ssl);
  SSL_free(ssl);
  ensure(>= 0, close, requst);
  return NULL;
}

static void filetype(char *fname, char *ftype) {
  static const struct {
    const char *fname;
    const char *ftype;
  } list[] = {
      {".html", "text/html"},
      {".mp4", "video/mp4"},
  };
  for (int i = 0; i < ARRLEN(list); i++) {
    if (strstr(fname, list[i].fname)) {
      strcpy(ftype, list[i].ftype);
      return;
    }
  }
  Assert(0, "File type not supported");
}

static int need_chunk(char *ftype, size_t flen) {
  // return 0;
  if (strcmp(ftype, "video/mp4") == 0 && flen >= FILEBUF_LEN) {
    return 1;
  } else {
    return 0;
  }
}

void https_response(SSL *ssl, char *buf, char *url) {
  struct stat fstat;
  char header[4096], *fname, ftype[20];

  fname = url + 1;

  if (stat(fname, &fstat) < 0) {
#define RESP404 "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n"
    SSL_write(ssl, RESP404, STRLEN(RESP404));
#undef RESP404
    Log("Can not find file\n");
  } else {
    size_t start = 0;
    size_t end = fstat.st_size;
    size_t resp_pos = 0;

    char *range = strstr(buf, "Range: bytes=");
    if (range) {
      sscanf(range, "Range: bytes=%ld-%ld", &start, &end);
      end += 1;
      end = (end > fstat.st_size) ? fstat.st_size : end; 
      start = (start > fstat.st_size) ? fstat.st_size : start; 
      resp_pos += sprintf(header + resp_pos,
                          "HTTP/1.1 206 Partial Content\r\n"
                          "Content-Range: bytes %ld-%ld/%ld\r\n",
                          start, end - 1, fstat.st_size);
    } else {
      resp_pos += sprintf(header + resp_pos, "HTTP/1.1 200 OK\r\n");
    }
    long body_length = end - start;

    filetype(fname, ftype);
    int chunk = need_chunk(ftype, body_length);
    if (chunk) {
      resp_pos += sprintf(header + resp_pos, "Transfer-Encoding: chunked\r\n");
    } else {
      resp_pos += sprintf(header + resp_pos, "Content-Length: %ld\r\n", body_length);
    }

    resp_pos += sprintf(header + resp_pos, "Content-Type: %s\r\n\r\n", ftype);
    Log("Response headers:\n%s", header);
    ensure(>= 0, SSL_write, ssl, header, resp_pos);

    int fd = open(fname, O_RDONLY);
    char *fptr = mmap(0, fstat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    size_t offset = start;
    while (body_length > 0) {
      long numread = body_length < FILEBUF_LEN ? body_length : FILEBUF_LEN;

      if (chunk) {
        char num[1024];
        int len = snprintf(num, 1024, "%lx\r\n", numread);
        if (SSL_write(ssl, num, len) < 0) {
          ERR_print_errors_fp(stderr);
        }
      }

      if (SSL_write(ssl, fptr + offset, numread) < 0) {
        ERR_print_errors_fp(stderr);
      }

      if (chunk) {
        if (SSL_write(ssl, "\r\n", STRLEN("\r\n")) < 0) {
          ERR_print_errors_fp(stderr);
        }
      }

      body_length -= numread;
      offset += numread;
    }

    if (chunk) {
      if (SSL_write(ssl, "0\r\n\r\n", STRLEN("0\r\n\r\n")) < 0) {
        ERR_print_errors_fp(stderr);
      }
    }

    munmap(fptr, fstat.st_size);
    ensure(>= 0, close, fd);
    Log("Close");
  }
}
