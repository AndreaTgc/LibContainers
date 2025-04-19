#ifndef _COMMON_H_
#define _COMMON_H_

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

/*******************************/
/*      PUBLIC MACROS          */
/*******************************/

#ifndef NDEBUG

#define ASSERT(cond, ...)                                                      \
  do {                                                                         \
    if (!cond) {                                                               \
      fprintf(stderr, "[ASSERT FAIL] " __VA_ARGS__);                           \
      fprintf(stderr, "[FAILED CONDITION] %s at [file] %s [line] %d\n", #cond, \
              __FILE__, __LINE__);                                             \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define LOG(...)                                                               \
  do {                                                                         \
    fprintf(stdout, "[LOG] " __VA_ARGS__);                                     \
  } while (0);

#else
#define ASSERT(cond, ...)
#define LOG(...)
#endif // NDEBUG

/*******************************/
/*     PRIVATE MACROS          */
/*******************************/

#define ___cat(a, b) a##b
#define ___concat(a, b) ___cat(a, b)
#define _lc_join(a, b) ___concat(a, ___concat(_, b))

#endif // COMMON_H_
