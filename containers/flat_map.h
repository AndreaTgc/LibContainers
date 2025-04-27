#include "common.h"

// @Author: Andrea Colombo (AndreaTgc)
// Generic type-safe flat hash-map
// Implementation details:
// [1] Uses closed hashing to handle collisions, the entries are stored in a
// single array, resulting in more cache-friendly structure compared to the
// node_map.
// [2] Instead of using the modulo operator we use fibonacci hashing to map the
// key hash onto a slot in the array, this doesn't result in any chance in how
// the map behaves, it's just a neat trick to gain some performance (and style)
// points. The only thing to keep in mind is that the map capacity needs to be a
// power of 2.
// [3] During insertion and deletion we use a technique called
// Robin-hood hashing in order to have the best distribution possible, resulting
// in less variance in lookup times and better performance at high load factors

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#ifndef K
#define K int
#endif // K

#if !defined(V)
#define V int
#endif // V

#if !defined(_lc_fib_const)
#define _lc_fib_const 11400714819323198485llu
#endif // _lc_fib_const

#if !defined(_lc_flatmap_lf)
#define _lc_flatmap_lf 0.85
#endif // _lc_flatmap_lf

#if !defined(_lc_flatmap_pfx)
#define _lc_flatmap_pfx _lc_join(K, V)
#define __map_t _lc_join(_lc_flatmap_pfx, flat_map)
#else
#define __map_t _lc_flatmap_pfx
#endif // _lc_flatmap_pfx

#define __entry_t _lc_join(_lc_flatmap_pfx, node)
#define Self __map_t

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

/**
 * @brief default key equality function, used if the user
 * doesn't provide a custom one.
 */
static inline bool
_lc_membfunc(default_key_eq)(K a, K b) {
  return a == b;
}

/**
 * @brief fibonacci hashing function that maps a key hash onto a slot
 * inside the map (still experimenting)
 */
static inline size_t
_lc_membfunc(fib_hash)(uint64_t hash, size_t cap) {
  return (size_t)(hash * _lc_fib_const) >> (__builtin_clzll(cap) + 1);
}

/**
 * @brief initializes a map
 *
 * @param self pointer to the map
 * @param cap  initial number of slots
 * @param hash user-provided hash function for K
 * @param eq   equality function for keys
 * @param dk   function to deallocate keys (optional)
 * @param dv   function to deallocate values (optional)
 */
static inline void
_lc_membfunc(init)(__map_t *self, size_t cap, uint64_t (*hash)(K),
                        bool (*eq)(K, K), void (*dk)(K), void (*dv)(V)) {
  ASSERT((self != NULL), "Trying to call init on a NULL map\n");
  ASSERT((hash != NULL), "Trying to initialise a map without a hash func\n");
  if (cap != 0) {
    ASSERT(((cap & cap - 1) == 0),
           "The provided capacity must be a power of 2");
  }
  memset(self, 0, sizeof(*self));
  self->capacity = cap == 0 ? 32 : cap;
  self->hash = hash;
  self->key_eq = eq ? eq : _lc_membfunc(default_key_eq);
  self->drop_k = dk;
  self->drop_v = dv;
  self->slots = (__entry_t *)calloc(self->capacity, sizeof(__entry_t));
}

/**
 * @brief returns the pointer to the value associated to a given key
 * returns NULL if the key is not found inside the map
 *
 * @param self pointer to the map
 * @param key  key to find
 */
static inline V *
_lc_membfunc(find)(__map_t *self, K key) {
  size_t index = _lc_membfunc(fib_hash)(self->hash(key), self->capacity);
  uint8_t dist = 0;
  for (;;) {
    __entry_t *slot = &self->slots[index];
    if (!slot->in_use)
      return NULL;
    if (slot->off < dist)
      return NULL;
    if (self->key_eq(slot->key, key))
      return &slot->value;
    index = (index + 1) & (self->capacity - 1);
  }
  return NULL;
}

/**
 * @brief checks if a key is found inside the map
 * returns true if the key is found, returns false if not found
 *
 * @param self pointer to the map
 * @param key  key to find
 */
static inline bool
_lc_membfunc(contains)(__map_t *self, K key) {
  return _lc_membfunc(find)(self, key) != NULL;
}

static inline bool
_lc_membfunc(get)(__map_t *self, K key, V *out) {
  V *t = _lc_membfunc(find)(self, key);
  if (!t)
    return false;
  *out = *t;
  return true;
}

// Declaration for rehashing
static inline bool _lc_membfunc(insert)(__map_t *self, K key, V value);

/**
 * @brief rehashes the map, requires new cap to be higher than the current
 * capacity
 *
 * @param self    pointer to the map
 * @param new_cap new capacity for rehashing
 */
static inline void
_lc_membfunc(rehash)(__map_t *self, size_t new_cap) {
  ASSERT((new_cap > self->capacity),
         "Trying to resize the table with a smaller capacity\n");
  __entry_t *new_slots = (__entry_t *)calloc(new_cap, sizeof(__entry_t));
  __entry_t *old_slots = self->slots;
  size_t old_cap = self->capacity;
  self->capacity = new_cap;
  self->slots = new_slots;
  for (size_t i = 0; i < old_cap; i++) {
    if (old_slots[i].in_use) {
      _lc_membfunc(insert)(self, old_slots[i].key, old_slots[i].value);
    }
  }
  free(old_slots);
}

/**
 * @brief inserts a (K, V) pair into the map
 * returns true if the pair was inserted
 * returns false if the key was already inside the map
 *
 * @param self  pointer to the map
 * @param key   key to insert
 * @param value value to insert
 */
static inline bool
_lc_membfunc(insert)(__map_t *self, K key, V value) {
  if ((float)self->size / self->capacity >= _lc_flatmap_lf) {
    _lc_membfunc(rehash)(self, self->capacity << 1);
  }
  size_t index = _lc_membfunc(fib_hash)(self->hash(key), self->capacity);
  uint8_t dist = 0;
  __entry_t new_entry = {key, value, true, dist};
  for (;;) {
    __entry_t *slot = &self->slots[index];
    if (!slot->in_use) {
      // Found an empty slot, we use it for insertion
      *slot = new_entry;
      self->size++;
      return true;
    }
    // Check if the key is already in the map
    // if yes, abort the insertion
    if (self->key_eq(slot->key, key))
      return false;

    if (slot->off < dist) {
      // swap the elements for robin hood hashing
      __entry_t tmp = *slot;
      *slot = new_entry;
      new_entry = tmp;
    }

    dist++;
    new_entry.off = dist;
    index = (index + 1) & (self->capacity - 1); // capacity is power of 2
  }
  // We should never get here
  return false;
}

/**
 * @brief removes a key and its associated value from the map
 * returns true if the pair is removed from the map
 * returns false if the key is not found in the map
 *
 * @param self pointer to the map
 * @param key  key to remove
 */
static inline bool
_lc_membfunc(remove)(__map_t *self, K key) {
  size_t index = _lc_membfunc(fib_hash)(key, self->capacity);
  uint8_t dist = 0;
  for (;;) {
    __entry_t *slot = &self->slots[index];
    if (!slot->in_use)
      return false;
    if (slot->off < dist)
      return false;
    if (self->key_eq(slot->key, key)) {
      if (self->drop_k)
        self->drop_k(slot->key);
      if (self->drop_v)
        self->drop_v(slot->value);
      size_t next = (index + 1) & (self->capacity - 1);
      while (self->slots[next].in_use && self->slots[next].off > 0) {
        self->slots[index] = self->slots[next];
        self->slots[index].off--;
        index = next;
        next = (next + 1) & (self->capacity - 1);
        return true;
      }
    }
    dist++;
    index = (index + 1) & (self->capacity - 1);
  }
  // We should never get here
  return false;
}

/**
 * @brief deallocates the memory associated to a map, resetting it to a
 * "base" state
 *
 * @param self pointer to the map
 */
static inline void
_lc_membfunc(destroy)(__map_t *self) {
  if (!self)
    return;
  if (self->slots) {
    for (size_t i = 0; i < self->capacity; i++) {
      if (self->slots[i].in_use) {
        if (self->drop_k)
          self->drop_k(self->slots[i].key);
        if (self->drop_v)
          self->drop_v(self->slots[i].value);
      }
    }
    free(self->slots);
  }
  memset(self, 0, sizeof(*self));
}

#undef K
#undef V
#undef __map_t
#undef __entry_t
#undef _lc_flatmap_pfx
#undef _lc_flatmap_lf

#if defined(__cplusplus)
}
#endif // __cplusplus
