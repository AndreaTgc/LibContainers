#include "common.h"

// @Author: Andrea Colombo (AndreaTgc)

#ifndef K
#define K int
#endif // K

#ifndef V
#define V int
#endif // V

#ifndef _lc_fib_const
#define _lc_fib_const 11400714819323198485llu
#endif // _lc_fib_const

#ifndef _lc_flatmap_pfx
#define _lc_flatmap_pfx _lc_join(K, V)
#define __map_t _lc_join(_lc_flatmap_pfx, flat_map)
#else
#define __map_t _lc_flatmap_pfx
#endif // _lc_flatmap_pfx

#define __entry_t _lc_join(_lc_flatmap_pfx, node)

typedef struct __entry_t {
  K key;
  V value;
  bool in_use;
  uint8_t off;
} __entry_t;

typedef struct __map_t {
  size_t size;          // number of elements in the table
  size_t capacity;      // total slots available
  __entry_t *slots;     // Slot array
  uint64_t (*hash)(K);  // user-defined hash for the key type
  bool (*key_eq)(K, K); // user_defined or 'default_key_eq'
  void (*drop_k)(K);    // used for complex/heap-allocated types
  void (*drop_v)(V);    // same as 'drop_k'
} __map_t;

static inline bool
_lc_join(__map_t, default_key_eq)(K a, K b) {
  return a == b;
}

static inline size_t
_lc_join(__map_t, fib_hash)(uint64_t hash, size_t cap) {
  return (size_t)(hash * _lc_fib_const) >> (__builtin_clzll(cap) + 1);
}

static inline void
_lc_join(__map_t, init)(__map_t *map, size_t cap, uint64_t (*hash)(K),
                        bool (*eq)(K, K), void (*dk)(K), void (*dv)(V)) {
  ASSERT((map != NULL), "Trying to call init on a NULL map\n");
  ASSERT((hash != NULL), "Trying to initialise a map without a hash func\n");
  if (cap != 0) {
    ASSERT(((cap & cap - 1) == 0),
           "The provided capacity must be a power of 2");
  }
  memset(map, 0, sizeof(*map));
  map->capacity = cap == 0 ? 32 : cap;
  map->hash = hash;
  map->key_eq = eq ? eq : _lc_join(__map_t, default_key_eq);
  map->drop_k = dk;
  map->drop_v = dv;
  map->slots = (__entry_t *)calloc(map->capacity, sizeof(__entry_t));
}

static inline V *
_lc_join(__map_t, find)(__map_t *map, K key) {
  size_t index = _lc_join(__map_t, fib_hash)(map->hash(key), map->capacity);
  uint8_t dist = 0;
  for (;;) {
    __entry_t *slot = &map->slots[index];
    if (!slot->in_use)
      return NULL;
    if (slot->off < dist)
      return NULL;
    if (map->key_eq(slot->key, key))
      return &slot->value;
    index = (index + 1) & (map->capacity - 1);
  }
  return NULL;
}

static inline bool
_lc_join(__map_t, contains)(__map_t *map, K key) {
  return _lc_join(__map_t, find)(map, key) != NULL;
}

static inline bool
_lc_join(__map_t, get)(__map_t *map, K key, V *out) {
  V *t = _lc_join(__map_t, find)(map, key);
  if (!t)
    return false;
  *out = *t;
  return true;
}

static inline bool
_lc_join(__map_t, insert)(__map_t *map, K key, V value) {
  size_t index = _lc_join(__map_t, fib_hash)(map->hash(key), map->capacity);
  uint8_t dist = 0;
  __entry_t new_entry = {key, value, true, dist};
  for (;;) {
    __entry_t *slot = &map->slots[index];
    if (!slot->in_use) {
      // Found an empty slot, we use it for insertion
      *slot = new_entry;
      map->size++;
      return true;
    }
    // Check if the key is already in the map
    // if yes, abort the insertion
    if (map->key_eq(slot->key, key))
      return false;

    if (slot->off < dist) {
      // swap the elements for robin hood hashing
      __entry_t tmp = *slot;
      *slot = new_entry;
      new_entry = tmp;
    }

    dist++;
    new_entry.off = dist;
    index = (index + 1) & (map->capacity - 1); // capacity is power of 2
  }
  // We should never get here
  return false;
}

static inline bool
_lc_join(__map_t, remove)(__map_t *map, K key) {
  size_t index = _lc_join(__map_t, fib_hash)(key, map->capacity);
  uint8_t dist = 0;
  for (;;) {
    __entry_t *slot = &map->slots[index];
    if (!slot->in_use)
      return false;
    if (slot->off < dist)
      return false;
    if (map->key_eq(slot->key, key)) {
      if (map->drop_k)
        map->drop_k(slot->key);
      if (map->drop_v)
        map->drop_v(slot->value);
      size_t next = (index + 1) & (map->capacity - 1);
      while (map->slots[next].in_use && map->slots[next].off > 0) {
        map->slots[index] = map->slots[next];
        map->slots[index].off--;
        index = next;
        next = (next + 1) & (map->capacity - 1);
        return true;
      }
    }
    dist++;
    index = (index + 1) & (map->capacity - 1);
  }
  // We should never get here
  return false;
}

static inline void
_lc_join(__map_t, destroy)(__map_t *map) {
  if (!map)
    return;
  if (map->slots) {
    for (size_t i = 0; i < map->capacity; i++) {
      if (map->slots[i].in_use) {
        if (map->drop_k)
          map->drop_k(map->slots[i].key);
        if (map->drop_v)
          map->drop_v(map->slots[i].value);
      }
    }
    free(map->slots);
  }
  memset(map, 0, sizeof(*map));
}
