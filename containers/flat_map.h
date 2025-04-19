#include "common.h"

// @Author: Andrea Colombo (AndreaTgc)
// Flat hash table implementation, heavily inspired by Abseil's flat hash map
// Implementation details:
// [1] Uses control bytes to probe during inserts/finds, each control bytes has
// the following layout: 1 bit to check if the entry is occupied and 7 bits to
// store a part of the key hash (H2). Instead of probing directly into the slots
// array, we use the control bytes array first, this allows us to have better
// cache efficiency and thus better performance.
// Let's suppose we're calling 'find' inside out table, here's what happens
// 1: We compute the key hash and divide it into H1 and H2, H1 is going to be
// the one we use to compute the mod and the starting index of our probing. 2:
// We start probing inside the control bytes array until we find a slot that
// matches with out computed H2
// 3: Once we found a match, we check the same index into the slots array and
// call the equality function on our keys. If the keys match we can stop
// probing, if they don't we simply go back into the control bytes and we keep
// probing until we find a match or an empty slot
// This works well because generally L1 cache lines are 64 bytes long, meaning
// we can probe 64 times into the control bytes basically for free. If we were
// probing into the slots we wouln't be able to fit nearly as many slots into
// the same L1 cache line

#ifndef K
#define K int
#endif // K

#ifndef V
#define V int
#endif // V

#ifndef _lc_flatmap_pfx
#define _lc_flatmap_pfx _lc_join(K, V)
#endif // _lc_flatmap_pfx

#define __entry_t _lc_join(_lc_flatmap_pfx, node)
#define __map_t _lc_join(_lc_flatmap_pfx, map)

typedef struct __entry_t {
  K key;
  V value;
} __entry_t;

typedef struct __map_t {
  size_t size;          // number of elements in the table
  size_t capacity;      // total slots available
  uint8_t *ctrl;        // control bytes for probing with H2
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

static inline void
_lc_join(__map_t, init)(__map_t *map, size_t cap, uint64_t (*hash)(K),
                        bool (*eq)(K, K), void (*dk)(K), void (*dv)(V)) {
  ASSERT((map != NULL), "Trying to call init on a NULL map\n");
  ASSERT((hash != NULL), "Trying to initialise a map without a hash func\n");
  memset(map, 0, sizeof(*map));
  map->capacity = cap == 0 ? 32 : cap;
  map->hash = hash;
  map->key_eq = eq ? eq : _lc_join(__map_t, default_key_eq);
  map->drop_k = dk;
  map->drop_v = dv;
  size_t len = cap + (cap * sizeof(__entry_t));
  char *p = (char *)calloc(1, len);
  ASSERT((p != NULL), "Something went wrong during map init\n");
  map->ctrl = (uint8_t *)p;
  map->slots = (__entry_t *)(p + cap);
}

static inline void
_lc_join(__map_t, destroy)(__map_t *map) {
  if (!map)
    return;
  for (size_t i = 0; i < map->capacity; i++) {
  }
  free(map->ctrl); // Also frees slots
  memset(map, 0, sizeof(*map));
}
