#include "common.h"

// @Author: Andrea Colombo (AndreaTgc)
// Generic type-safe hash set
// Implementation details:
// [1] Uses open hashing to handle key collisions
// [2] Defaults to a max load factor of 0.8, to override it
// just define _lc_nodeset_lf before including this file
// [3] The equality operator defaults to a == b, you can ovveride this
// by passing your own equality functions to your 'init' call
// [4] Ensures pointer stability (even after rehashing)

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if !defined(T)
#define T int
#endif // T

#if !defined(_lc_nodeset_pfx)
#define _lc_nodeset_pfx T
#define __set_t _lc_join(_lc_nodeset_pfx, node_set)
#else
#define __set_t _lc_nodeset_pfx
#endif // _lc_nodeset_pfx

#if !defined(_lc_nodeset_lf)
#define _lc_nodeset_lf 0.8f
#endif // _lc_nodeset_lf

#define __node_t _lc_join(_lc_nodeset_pfx, node)

// Generic node used for open hashing
typedef struct __node_t {
  struct __node_t *next;
  T data;
} __node_t;

typedef struct __set_t {
  size_t size;
  size_t capacity;
#if defined(_lc_profile_enabled)
  // This profiling data can be used for debugging and spotting
  // potential issues inside the hash function provided
  // Ideal case:
  // [1] hash_or should tend to 0xFFFFFFFF
  // [2] hash_and should tend to 0
  uint64_t hash_or;
  uint64_t hash_and;
  size_t num_erases;
  size_t max_probe;
#endif // _lc_profile_enabled
  __node_t **buckets;
  uint64_t (*hash)(T);
  void (*drop)(T);
  bool (*eq)(T, T);
} __set_t;

/**
 * @brief default equality used if the user doesn't provide a
 * custom one
 */
static inline bool
_lc_join(__set_t, fallback_eq)(T a, T b) {
  return a == b;
}

/**
 * @brief initializes the set
 *
 * @param self pointer to the set
 * @param icap initial capacity for the set
 * @param hash user defined hash function for type T
 * @param drop function used for deallocating the elements (optional)
 * @param eq   equality function for type T
 */
static inline void
_lc_join(__set_t, init)(__set_t *self, size_t icap, uint64_t (*hash)(T),
                        void (*drop)(T), bool (*eq)(T, T)) {
  ASSERT((self != NULL), "Trying to call 'init' on a NULL set\n");
  ASSERT((hash != NULL),
         "A hash function is required for set initialization\n");
  self->size = 0;
  self->capacity = icap == 0 ? 32 : icap;
  self->hash = hash;
  self->drop = drop;
  self->eq = eq == NULL ? _lc_join(__set_t, fallback_eq) : eq;
  self->buckets = (__node_t **)calloc(self->capacity, sizeof(__node_t *));
#if defined(_lc_profile_enabled)
  self->hash_or = 0;
  self->hash_and = UINT64_MAX;
  self->num_erases = 0;
  self->max_probe = 0;
#endif // _lc_profile_enabled
}

/**
 * @brief rehashes the set with a new capacity, mainly used for growing
 * the set size when it goes over the max load factor
 *
 * @param self pointer to the set
 * @param cap  new set capacity
 */
static inline void
_lc_join(__set_t, rehash)(__set_t *self, size_t cap) {
  ASSERT((self != NULL), "Trying to call 'resize' on a NULL set\n");
  __node_t **new_buckets = (__node_t **)calloc(cap, sizeof(__node_t *));
  __node_t **old_buckets = self->buckets;
  for (size_t i = 0; i < self->capacity; i++) {
    __node_t *n = old_buckets[i];
    __node_t *next = NULL;
    while (n != NULL) {
      next = n->next;
      // Insert the element into the new buckets
      uint64_t idx = (size_t)self->hash(n->data) % cap;
      __node_t *n2 = new_buckets[idx];
      if (n2 == NULL) {
        new_buckets[idx] = n;
        new_buckets[idx]->next = NULL;
        // new_buckets[idx]->data = n->data;
      } else {
        __node_t *prv = NULL;
        while (n2 != NULL) {
          prv = n2;
          n2 = n2->next;
        }
        prv->next = n;
        prv->next->next = NULL;
        prv->next->data = n->data;
      }
      n = next;
    }
  }
  self->capacity = cap;
  self->buckets = new_buckets;
  free(old_buckets);
}

/**
 * @brief inserts a new key inside the set
 * returns true if the key was added to the set
 * returns false if the key was already in the set
 *
 * @param self pointer to the set
 * @param key  key to add to the set
 */
static inline bool
_lc_join(__set_t, insert)(__set_t *self, T key) {
  ASSERT((self != NULL), "Trying to call 'insert' on a NULL set\n");
  ASSERT((self->eq != NULL && self->hash != NULL),
         "Trying to call 'insert' on a set that's not correctly initialised\n");
  if (((float)self->size / (float)self->capacity) > _lc_nodeset_lf)
    _lc_join(__set_t, rehash)(self, self->capacity * 2);
  uint64_t h = self->hash(key);
#if defined(_lc_profile_enabled)
  s->hash_or |= h;
  s->hash_and &= h;
#endif // _lc_profile_enabled
  size_t index = ((size_t)h % self->capacity);
  if (self->buckets[index] == NULL) {
    self->buckets[index] = (__node_t *)malloc(sizeof(__node_t));
    ASSERT((self->buckets[index] != NULL),
           "Failed to allocate first node inside bucket\n");
    self->buckets[index]->data = key;
    self->buckets[index]->next = NULL;
    self->size++;
    return true;
  }
  __node_t *cur = self->buckets[index];
  __node_t *prev = NULL;
  while (cur != NULL) {
    if (self->eq(cur->data, key))
      return false;
    prev = cur;
    cur = cur->next;
  }
  prev->next = (__node_t *)malloc(sizeof(__node_t));
  ASSERT((prev->next != NULL), "Failed to allocate new node for hash set\n");
  prev->next->next = NULL;
  prev->next->data = key;
  self->size++;
  return true;
}

/**
 * @brief checks if a given key is inside the set.
 * returns true if the key is in the set, returns false if not
 *
 * @param self pointer to the set
 * @param key  key that has to be checked
 */
static inline bool
_lc_join(__set_t, contains)(__set_t *self, T key) {
  uint64_t h = self->hash(key);
#if defined(_lc_profile_enabled)
  s->hash_or |= h;
  s->hash_and &= h;
#endif // _lc_profile_enabled
  size_t index = (size_t)(h % self->capacity);
  __node_t *cur = self->buckets[index];
  while (cur) {
    if (self->eq(key, cur->data))
      return true;
    cur = cur->next;
  }
  return false;
}

/**
 * @brief returns the pointer to the key inside the set (NULL if the key is not
 * inside the set)
 *
 * @param self pointer to the set
 * @param key  key to find
 */
static inline T *
_lc_join(__set_t, find)(__set_t *self, T key) {
  uint64_t h = self->hash(key);
#if defined(_lc_profile_enabled)
  self->hash_or |= h;
  self->hash_and &= h;
  size_t probe = 0;
#endif // _lc_profile_enabled
  size_t index = (size_t)(h % self->capacity);
  __node_t *cur = self->buckets[index];
  while (cur) {
    if (self->eq(key, cur->data)) {
#if defined(_lc_profile_enabled)
      if (probe > self->max_probe)
        self->max_probe = probe;
#endif // _lc_profile_enabled
      return &cur->data;
    }
#if defined(_lc_profile_enabled)
    probe++;
#endif // _lc_profile_enabled
    cur = cur->next;
  }
#if defined(_lc_profile_enabled)
  if (probe > s->max_probe)
    self->max_probe = probe;
#endif // _lc_profile_enabled
  return NULL;
}

/**
 * @brief removes a key from the set.
 * returns true if the key is removed
 * returns false if the key is not in the set
 *
 * @param self pointer to the set
 * @param key  key to remove from the set
 */
static inline bool
_lc_join(__set_t, remove)(__set_t *self, T key) {
  uint64_t h = self->hash(key);
  size_t index = (size_t)(h % self->capacity);
#if defined(_lc_profile_enabled)
  self->hash_or |= h;
  self->hash_and &= h;
  size_t probe = 0;
#endif // _lc_profile_enabled
  __node_t *cur = self->buckets[index];
  __node_t *prv = NULL;
  while (cur) {
    if (self->eq(key, cur->data)) {
      if (prv == NULL) {
        self->buckets[index] = cur->next;
        if (self->drop)
          self->drop(cur->data);
        free(cur);
      } else {
        prv->next = cur->next;
        if (self->drop)
          self->drop(cur->data);
        free(cur);
      }
      self->size--;
#if defined(_lc_profile_enabled)
      if (probe > self->max_probe)
        self->max_probe = probe;
      self->num_erases++;
#endif // _lc_profile_enabled
      return true;
    }
#if defined(_lc_profile_enabled)
    probe++;
#endif // _lc_profile_enabled
    prv = cur;
    cur = cur->next;
  }
#if defined(_lc_profile_enabled)
  if (probe > self->max_probe)
    self->max_probe = probe;
#endif // _lc_profile_enabled
  return false;
}

/**
 * @brief deallocates the memory associated with a set, resetting it
 * to a "base" state
 *
 * @param self pointer to the set
 */
static inline void
_lc_join(__set_t, destroy)(__set_t *self) {
  if (!self)
    return;
  for (size_t i = 0; i < self->capacity; i++) {
    if (self->buckets[i] == NULL)
      continue;
    __node_t *cur = self->buckets[i];
    __node_t *to_free = NULL;
    while (cur) {
      to_free = cur;
      cur = cur->next;
      if (self->drop)
        self->drop(to_free->data);
      free(to_free);
    }
  }
  free(self->buckets);
  self->buckets = NULL;
}

#if defined(_lc_profile_enabled)
// Prints a small set of data used to gather information
// about the set performance. The performance of a hash set comes
// down to a simple question: "How good is your hash function?"
// the hash_or and hash_and values tells us if our hash function
// has stuck bits, the ideal case is where hash_or tends to 0xFFFF
// and hash_and to 0
static inline void
_lc_join(__set_t, print_profiling_data)(__set_t *self, const char *name) {
  fprintf(stdout, "[%s profiling data] set ptr = %p\n", name, (void *)self);
  fprintf(stdout, "[Running hash or] %04llx\n", self->hash_or);
  fprintf(stdout, "[Running hash and] %04llx\n", self->hash_and);
  size_t total = 0, max = 0, c = 0, empty = 0;
  for (size_t i = 0; i < self->capacity; i++) {
    c = 0;
    __node_t *n = self->buckets[i];
    while (n != NULL) {
      total++;
      c++;
      n = n->next;
    }
    if (c == 0)
      empty++;
    if (c > max)
      max = c;
  }
  ASSERT((total == self->size),
         "Something when wrong when calculating profiling data\n");
  fprintf(stdout, "[Maximum bucket length] %zu\n", max);
  fprintf(stdout, "[Average bucket length] %.3f\n",
          (float)total / (float)self->capacity);
  fprintf(stdout, "[Number of empty buckets] %zu\n", empty);
  fprintf(stdout, "[Average length of non empty buckets] %.3f\n",
          (float)total / (float)(self->capacity - empty));
  fprintf(stdout, "[Num erases] %zu\n", self->num_erases);
  fprintf(stdout, "[Maximum probe length] %zu\n", self->max_probe);
}
#endif // _lc_profile_enabled

#undef T
#undef __node_t
#undef __set_t
#undef _lc_nodeset_pfx

#if defined(__cplusplus)
}
#endif // __cplusplus
