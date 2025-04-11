#ifndef CTL_MAP_H_
#define CTL_MAP_H_

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CTL_DEF_MAP_DEFAULT(KT, VT) CTL_DEF_MAP(KT, VT, KT##_##VT##_)

#define CTL_DEF_MAP(KT, VT, PREFIX)                                            \
  typedef struct PREFIX##map {                                                 \
    size_t size;                                                               \
    size_t capacity;                                                           \
    KT *keys;                                                                  \
    VT *values;                                                                \
    bool (*eq)(KT, KT);                                                        \
    uint64_t (*hash)(KT);                                                      \
  } PREFIX##map;                                                               \
                                                                               \
  void PREFIX##map_init(PREFIX##map *map, size_t initial_capacity,             \
                        bool (*eq)(KT, KT), uint64_t (*hash)(KT)) {            \
    map->capacity = initial_capacity == 0 ? 32 : initial_capacity;             \
    map->size = 0;                                                             \
    map->eq = eq;                                                              \
    map->hash = hash;                                                          \
    map->keys = calloc(sizeof(KT), map->capacity);                             \
    map->values = calloc(sizeof(VT), map->capacity);                           \
    assert(map->keys != NULL);                                                 \
    assert(map->values != NULL);                                               \
  }                                                                            \
                                                                               \
  void PREFIX##map_destroy(PREFIX##map *map) {                                 \
    if (!map)                                                                  \
      return;                                                                  \
    if (map->keys)                                                             \
      free(map->keys);                                                         \
    if (map->values)                                                           \
      free(map->values);                                                       \
    map->capacity = 0;                                                         \
    map->size = 0;                                                             \
  }

#endif // CTL_MAP_H_
