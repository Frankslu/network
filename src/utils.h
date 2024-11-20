#ifndef UTILS_H
#define UTILS_H

#include "macro.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// ----------- safe print ----------

#define safe_print(...)                                                                            \
  do {                                                                                             \
    printf(__VA_ARGS__);                                                                           \
    fflush(stdout);                                                                                \
  } while (0)

#define safe_err_printf(...) fprintf(stderr, __VA_ARGS__)

// -----------  log  -----------

#define ANSI_FG_BLACK "\33[1;30m"
#define ANSI_FG_RED "\33[1;31m"
#define ANSI_FG_GREEN "\33[1;32m"
#define ANSI_FG_YELLOW "\33[1;33m"
#define ANSI_FG_BLUE "\33[1;34m"
#define ANSI_FG_MAGENTA "\33[1;35m"
#define ANSI_FG_CYAN "\33[1;36m"
#define ANSI_FG_WHITE "\33[1;37m"
#define ANSI_BG_BLACK "\33[1;40m"
#define ANSI_BG_RED "\33[1;41m"
#define ANSI_BG_GREEN "\33[1;42m"
#define ANSI_BG_YELLOW "\33[1;43m"
#define ANSI_BG_BLUE "\33[1;44m"
#define ANSI_BG_MAGENTA "\33[1;35m"
#define ANSI_BG_CYAN "\33[1;46m"
#define ANSI_BG_WHITE "\33[1;47m"
#define ANSI_NONE "\33[0m"

#define ANSI_FMT(str, fmt) fmt str ANSI_NONE

// ----------- debug -----------

#define Print(...) safe_print(__VA_ARGS__)

#define PrintColor(color, fmt, ...)                                                                \
  safe_print(ANSI_FMT(fmt, CONCAT(ANSI_FG_, color)), ##__VA_ARGS__)

#define LogColor(color, fmt, ...)                                                                  \
  do {                                                                                             \
    PrintColor(color, "[%s:%d %s] " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__);        \
    fflush(stdout);                                                                                \
  } while (0)

#define Log(fmt, ...)                                                                              \
  do {                                                                                             \
    LogColor(GREEN, fmt, ##__VA_ARGS__);                                                           \
    fflush(stdout);                                                                                \
  } while (0)

#define Err(fmt, ...) safe_err_printf(ANSI_FMT(fmt, ANSI_FG_RED) "\n", ##__VA_ARGS__)

#define ErrAt(fmt, ...) Err("[%s:%d %s] " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define Assert(cond, ...)                                                                          \
  do {                                                                                             \
    if (!(cond)) {                                                                                 \
      fflush(stdout);                                                                              \
      fflush(stderr);                                                                              \
      ErrAt(__VA_ARGS__);                                                                          \
      assert(0);                                                                                   \
    }                                                                                              \
  } while (0)

#define panic(format, ...) Assert(0, 0, format, ##__VA_ARGS__)

#endif