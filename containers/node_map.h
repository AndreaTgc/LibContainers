#include "common.h"

// @Author: Andrea Colombo (AndreaTgc)
// Implementation of a hash table inspired by C++'s std::unordered_map
// Implementation details:
// [1] Handles collisions with open hashing (linked lists)
// [2] Offers pointer stability, even after resizing
// [3] Takes full ownership of the key and values (eg. if you use char* as
// the key, you do not deallocate the string after calling 'insert', the map
// will handle the string deallocation when calling 'remove' or 'destroy')

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef K
#define K int
#endif // K

#ifndef V
#define V int
#endif // V

#ifndef _lc_nodemap_lf
#define _lc_nodemap_lf 0.8
#endif // _lc_nodemap_lf

#ifndef _lc_nodemap_pfx
#define _lc_nodemap_pfx _lc_join(K, V)
#define __map_t _lc_join(_lc_nodemap_pfx, node_map)
#else
#define __map_t _lc_nodemap_pfx
#endif // _lc_nodemap_pfx

#define __node_t _lc_join(_lc_nodemap_pfx, node)

// Node using in linked list to handle hash
// collisions (open hashing)
typedef struct __node_t {
  K key;
  V value;
  struct __node_t *next;
} __node_t;

typedef struct __map_t {
  size_t size;
  size_t capacity;
#ifdef _lc_profile_enabled
  uint64_t hash_or;
  uint64_t hash_and;
  size_t num_erases;
  size_t max_probe;
#endif // _lc_profile_enabled
  uint64_t (*hash)(K);
  void (*drop_key)(K);
  void (*drop_val)(V);
  bool (*key_eq)(K, K);
  __node_t **buckets;
} __map_t;

static inline bool
_lc_join(__map_t, default_key_eq)(K a, K b) {
  return a == b;
}

static inline void
_lc_join(__map_t, init)(__map_t *map, size_t capacity, uint64_t (*hash)(K),
                        bool (*k_eq)(K, K), void (*drop_k)(K),
                        void (*drop_v)(V)) {
  map->size = 0;
  map->capacity = capacity == 0 ? 32 : capacity;
  map->hash = hash;
  map->key_eq = k_eq ? k_eq : _lc_join(__map_t, default_key_eq);
  map->buckets = (__node_t **)calloc(map->capacity, sizeof(__node_t *));
#ifdef _lc_profile_enabled
  map->hash_or = 0;
  map->hash_and = UINT64_MAX;
  map->num_erases = 0;
  map->max_probe = 0;
#endif // _lc_profile_enabled
}

static inline void
_lc_join(__map_t, rehash)(__map_t *map, size_t cap) {
  ASSERT((map != NULL), "Trying to call 'resize' on a NULL map\n");
  __node_t **new_buckets = (__node_t **)calloc(cap, sizeof(__node_t *));
  __node_t **old_buckets = map->buckets;
  for (size_t i = 0; i < map->capacity; i++) {
    __node_t *cur = map->buckets[i];
    __node_t *next = NULL;
    while (cur) {
      next = cur->next;
      uint64_t new_idx = map->hash(cur->key) % cap;
      __node_t *n = new_buckets[new_idx];
      if (!n) {
        new_buckets[new_idx] = cur;
        new_buckets[new_idx]->next = NULL;
      } else {
        __node_t *prv = NULL;
        while (n) {
          prv = n;
          n = n->next;
        }
        prv->next = cur;
        prv->next->next = NULL;
      }
      cur = next;
    }
  }
  map->buckets = new_buckets;
  free(old_buckets);
  map->capacity = cap;
}

static inline bool
_lc_join(__map_t, insert)(__map_t *map, K key, V val) {
  if ((float)map->size / (float)map->capacity > _lc_nodemap_lf)
    _lc_join(__map_t, rehash)(map, map->capacity * 2);
  uint64_t h = map->hash(key);
#ifdef _lc_profile_enabled
  map->hash_or |= h;
  map->hash_and &= h;
#endif // _lc_profile_enabled
  size_t index = h % map->capacity;
  __node_t *cur = map->buckets[index];
  __node_t *prv = NULL;
  while (cur) {
    if (map->key_eq(cur->key, key))
      return false;
    prv = cur;
    cur = cur->next;
  }
  __node_t *new_node = (__node_t *)malloc(sizeof(__node_t));
  new_node->next = NULL;
  new_node->key = key;
  new_node->value = val;
  if (!prv)
    map->buckets[index] = new_node;
  else
    prv->next = new_node;
  map->size++;
  return true;
}

static inline bool
_lc_join(__map_t, contains)(__map_t *map, K key) {
  uint64_t h = map->hash(key);
  size_t index = (size_t)(h % map->capacity);
  __node_t *cur = map->buckets[index];
  while (cur) {
    if (map->key_eq(cur->key, key))
      return true;
    cur = cur->next;
  }
  return false;
}

static inline V *
_lc_join(__map_t, find)(__map_t *map, K key) {
  uint64_t h = map->hash(key);
  size_t index = (size_t)(h % map->capacity);
  __node_t *cur = map->buckets[index];
  while (cur) {
    if (map->key_eq(cur->key, key))
      return &cur->value;
    cur = cur->next;
  }
  return NULL;
}

static inline bool
_lc_join(__map_t, get)(__map_t *map, K key, V *out) {
  V *tmp = _lc_join(__map_t, find)(map, key);
  if (tmp) {
    *out = *tmp;
    return true;
  } else
    return false;
}

static inline bool
_lc_join(__map_t, remove)(__map_t *map, K key) {
  uint64_t h = map->hash(key);
  size_t index = (size_t)(h % map->capacity);
  __node_t *cur = map->buckets[index];
  __node_t *prv = NULL;
  while (cur) {
    if (map->key_eq(cur->key, key)) {
      if (!prv) {
        map->buckets[index] = cur->next;
      } else {
        prv->next = cur->next;
      }
      if (map->drop_key)
        map->drop_key(cur->key);
      if (map->drop_val)
        map->drop_val(cur->value);
      free(cur);
      map->size--;
#ifdef _lc_profile_enabled
      map->num_erases++;
#endif // _lc_profile_enabled
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
        map->drop_key(cur->key);
      if (map->drop_val)
        map->drop_val(cur->value);
      cur = cur->next;
      free(to_free);
    }
  }
  free(map->buckets);
  memset(map, 0, sizeof(*map));
}

#undef __node_t
#undef __map_t
#undef K
#undef V
#undef _lc_nodemap_pfx

#ifdef __cplusplus
}
#endif // __cplusplus
