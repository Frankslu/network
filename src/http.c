#include "socket.h"

#include <string.h>

static int init_http_server();
static int http_resp_handler(int conn_fd, req_header_t *h, char *serv_ip);
int wait_accept(int fd);
static void *http_req_handler_loop(void *args);

void *http_server(void *args) {
  int listen_fd = init_http_server();
  while (1) {
    int *conn_fd = malloc(sizeof(int));
    *conn_fd = wait_accept(listen_fd);
    pthread_t t;
    ensure(== 0, pthread_create, &t, NULL, http_req_handler_loop, (void *)conn_fd);
    pthread_detach(t);
  }
}

static int init_http_server() {
  int fd = ensure(>= 0, socket, AF_INET, SOCK_STREAM, 0);
  Log("Creat HTTP socket");

  struct sockaddr_in server_addr = {
      .sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_ANY), .sin_port = htons(HTTP_PORT)};

  ensure(>= 0, bind, fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  Log("Bind HTTP port: %d", HTTP_PORT);

  ensure(>= 0, listen, fd, BACKLOG);
  Log("Listen to HTTP port: %d", HTTP_PORT);

  return fd;
}

int wait_accept(int fd) {
  socklen_t client_addr_len = sizeof(struct sockaddr_in);
  struct sockaddr_in client_addr = {};
  Log("Waiting for HTTP accept");
  return ensure(>= 0, accept, fd, (struct sockaddr *)&client_addr, &client_addr_len);
}

static void *http_req_handler_loop(void *args) {
  int conn_fd = *(int *)args;
  char buf[HUGE_BUFFERSIZE];
  req_header_t h;

  socklen_t addr_size = sizeof(struct sockaddr_in);
  struct sockaddr_in addr;
  getsockname(conn_fd, (struct sockaddr *)&addr, &addr_size);
  char serv_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &addr.sin_addr, serv_ip, INET_ADDRSTRLEN);

  while (1) {
    if (ensure(>= 0, read, conn_fd, buf, HUGE_BUFFERSIZE) == 0) {
      Log("Client connection closed.\n");
      break;
    }
    LogColor(WHITE, "Request content:\n%s", buf);
    get_header(buf, &h);
    if (http_resp_handler(conn_fd, &h, serv_ip)) {
      break;
    }
  }
  free(args);
  return NULL;
}

static int http_resp_handler(int conn_fd, req_header_t *h, char *serv_ip) {
  if (strcmp(h->method, "GET") == 0 && strstr(h->version, "HTTP")) {
    char resp[2048];
    snprintf(resp, sizeof(resp),
             "HTTP/1.1 301 Moved Permanently\r\n"
             "Location: https://%s%s\r\n",
             serv_ip, h->url);
    LogColor(WHITE, "Response header:\n%s", resp);
    ensure(>= 0, send, conn_fd, resp, strlen(resp), 0);
    ensure(>= 0, close, conn_fd);
    return 1;
  } else {
    Assert(0, "Do not support");
  }
}
