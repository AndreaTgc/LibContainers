#if !defined(LCORE_H)
#define LCORE_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
typedef uint8_t      u8;
typedef int8_t       i8;
typedef uint16_t     u16;
typedef int16_t      i16;
typedef uint32_t     u32;
typedef int32_t      i32;
typedef uint64_t     u64;
typedef int64_t      i64;
typedef size_t       usize;
typedef unsigned int uint;
typedef float        f32;
typedef double       f64;
typedef char*        c_str;
// clang-format on

#define LCORE_API static inline

// Basic wrappers around malloc and calloc for more readable code
#define lcore_malloc(T, S) ((T*)malloc(S))
#define lcore_calloc(T, Tsize, Count) ((T*)calloc(Count, Tsize))

#endif // LCORE_H
