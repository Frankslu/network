#include "socket.h"

#define create_thread(thread, func, args)                                                          \
  do {                                                                                             \
    int res = pthread_create(&thread, NULL, func, args);                                           \
    Assert(res == 0, "Creat " str(func) " Failed\n");                                              \
  } while (0)

int main() {
  pthread_t p;
  create_thread(p, http_server, NULL);
  create_thread(p, https_server, NULL);
  pthread_join(p, NULL);
  return 0;
}
