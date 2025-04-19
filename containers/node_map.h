#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef K
#define K int
#endif // K

#ifndef V
#define V int
#endif // V

#ifndef _lc_nodemap_pfx
#define _lc_nodemap_pfx _lc_join(K, V)
#endif // _lc_nodemap_pfx

#define __node_t _lc_join(_lc_nodemap_pfx, node)
#define __map_t _lc_join(_lc_nodemap_pfx, node_map)

typedef struct __node_t {
  K key;
  V value;
  struct __node_t *next;
} __node_t;

typedef struct __map_t {
  size_t size;
  size_t capacity;
#ifdef _lc_profile_enabled
  size_t hash_or;
  size_t hash_and;
  size_t num_erases;
  size_t max_probe;
#endif // _lc_profile_enabled
  size_t (*hash)(K *);
  K (*clone_key)(K *);
  V (*clone_val)(V *);
  void (*drop_key)(K *);
  void (*drop_val)(V *);
  bool (*key_eq)(K *, K *);
  __node_t **buckets;
} __map_t;

static inline bool
_lc_join(__map_t, default_key_eq)(K *a, K *b) {
  return *a == *b;
}

static inline void
_lc_join(__map_t, init)(__map_t *map, size_t capacity, size_t (*hash)(K *),
                        bool (*k_eq)(K *, K *), K (*clone_k)(K *),
                        V (*clone_v)(V *), void (*drop_k)(K *),
                        void (*drop_v)(V *)) {
  map->size = 0;
  map->capacity = capacity == 0 ? 32 : capacity;
  map->hash = hash;
  map->clone_key = clone_k;
  map->clone_val = clone_v;
  map->drop_key = drop_k;
  map->drop_val = drop_v;
  map->key_eq = k_eq ? k_eq : _lc_join(__map_t, default_key_eq);
  map->buckets = (__node_t **)calloc(map->capacity, sizeof(__node_t *));
#ifdef _lc_profile_enabled
  map->hash_or = 0;
  map->hash_and = SIZE_T_MAX;
  map->num_erases = 0;
  map->max_probe = 0;
#endif // _lc_profile_enabled
}

static inline bool
_lc_join(__map_t, contains)(__map_t *map, K *key) {
  size_t h = map->hash(key);
  size_t index = h % map->capacity;
  __node_t *cur = map->buckets[index];
  while (cur) {
    if (map->key_eq(&cur->key, key))
      return true;
    cur = cur->next;
  }
  return false;
}

static inline V *
_lc_join(__map_t, get)(__map_t *map, K *key) {
  size_t h = map->hash(key);
  size_t index = h % map->capacity;
  __node_t *cur = map->buckets[index];
  while (cur) {
    if (map->key_eq(&cur->key, key))
      return &cur->value;
    cur = cur->next;
  }
  return NULL;
}

static inline bool
_lc_join(__map_t, remove)(__map_t *map, K *key) {
  size_t h = map->hash(key);
  size_t index = h % map->capacity;
  __node_t *cur = map->buckets[index];
  __node_t *prv = NULL;
  while (cur) {
    if (map->key_eq(&cur->key, key)) {
      if (!prv) {
        map->buckets[index] = cur->next;
      } else {
        prv->next = cur->next;
      }
      if (map->drop_key)
        map->drop_key(&cur->key);
      if (map->drop_val)
        map->drop_val(&cur->value);
      free(cur);
      return true;
    }
    prv = cur;
    cur = cur->next;
  }
  return false;
}

static inline void
_lc_join(__map_t, destroy)(__map_t *map) {
  if (!map)
    return;
  for (size_t i = 0; i < map->capacity; i++) {
    __node_t *cur = map->buckets[i];
    __node_t *to_free = NULL;
    while (cur) {
      to_free = cur;
      if (map->drop_key)
        map->drop_key(&cur->key);
      if (map->drop_val)
        map->drop_val(&cur->value);
      cur = cur->next;
      free(to_free);
    }
  }
  free(map->buckets);
  memset(map, 0, sizeof(*map));
}

#ifdef __cplusplus
}
#endif // __cplusplus
