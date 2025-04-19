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

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef T
#define T int
#endif // T

#ifndef _lc_nodeset_pfx
#define _lc_nodeset_pfx T
#endif // _lc_nodeset_pfx

#ifndef _lc_nodeset_lf
#define _lc_nodeset_lf 0.8f
#endif // _lc_nodeset_lf

#define __node_t _lc_join(_lc_nodeset_pfx, node)
#define __set_t _lc_join(_lc_nodeset_pfx, node_set)

// Generic node used for open hashing
typedef struct __node_t {
  struct __node_t *next;
  T data;
} __node_t;

typedef struct __set_t {
  size_t size;
  size_t capacity;
#ifdef _lc_profile_enabled
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

// Basic equality function used as default if the user doesn't provide
// a custom one
static inline bool
_lc_join(__set_t, fallback_eq)(T a, T b) {
  return a == b;
}

static inline void
_lc_join(__set_t, init)(__set_t *s, size_t initial_capacity,
                        uint64_t (*hash)(T), void (*drop)(T),
                        bool (*eq)(T, T)) {
  ASSERT((s != NULL), "Trying to call 'init' on a NULL set");
  ASSERT((hash != NULL), "A hash function is required for set initialization");
  s->size = 0;
  s->capacity = initial_capacity == 0 ? 32 : initial_capacity;
  s->hash = hash;
  s->drop = drop;
  s->eq = eq == NULL ? _lc_join(__set_t, fallback_eq) : eq;
  s->buckets = (__node_t **)calloc(s->capacity, sizeof(__node_t *));
#ifdef _lc_profile_enabled
  s->hash_or = 0;
  s->hash_and = 0;
  s->num_erases = 0;
  s->max_probe = 0;
#endif // _lc_profile_enabled
}

static inline void
_lc_join(__set_t, rehash)(__set_t *s, size_t cap) {
  ASSERT((s != NULL), "Trying to call 'resize' on a NULL set\n");
  __node_t **new_buckets = (__node_t **)calloc(cap, sizeof(__node_t *));
  __node_t **old_buckets = s->buckets;
  for (size_t i = 0; i < s->capacity; i++) {
    __node_t *n = old_buckets[i];
    __node_t *next = NULL;
    while (n != NULL) {
      next = n->next;
      // Insert the element into the new buckets
      uint64_t idx = (size_t)s->hash(n->data) % cap;
      __node_t *n2 = new_buckets[idx];
      if (n2 == NULL) {
        new_buckets[idx] = n;
        new_buckets[idx]->next = NULL;
        new_buckets[idx]->data = n->data;
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
  s->capacity = cap;
  s->buckets = new_buckets;
  free(old_buckets);
}

// Inserts a new key inside the set
// resolves hash collisions by using open hashing (linked list)
// the new element gets appended at the end of the list, preserving
// the insertion order
static inline bool
_lc_join(__set_t, insert)(__set_t *s, T key) {
  ASSERT((s != NULL), "Trying to call 'insert' on a NULL set\n");
  ASSERT((s->eq != NULL && s->hash != NULL),
         "Trying to call 'insert' on a set that's not correctly initialised\n");
  if (((float)s->size / (float)s->capacity) > _lc_nodeset_lf)
    _lc_join(__set_t, rehash)(s, s->capacity * 2);
  uint64_t h = s->hash(key);
#ifdef _lc_profile_enabled
  s->hash_or |= h;
  s->hash_and &= h;
#endif // _lc_profile_enabled
  size_t index = ((size_t)h % s->capacity);
  if (s->buckets[index] == NULL) {
    s->buckets[index] = (__node_t *)malloc(sizeof(__node_t));
    ASSERT((s->buckets[index] != NULL),
           "Failed to allocate first node inside bucket\n");
    s->buckets[index]->data = key;
    s->buckets[index]->next = NULL;
    s->size++;
    return true;
  }
  __node_t *cur = s->buckets[index];
  __node_t *prev = NULL;
  while (cur != NULL) {
    if (s->eq(cur->data, key))
      return false;
    prev = cur;
    cur = cur->next;
  }
  prev->next = (__node_t *)malloc(sizeof(__node_t));
  ASSERT((prev->next != NULL), "Failed to allocate new node for hash set\n");
  prev->next->next = NULL;
  prev->next->data = key;
  s->size++;
  return true;
}

// Checks whether a key is present inside the set
// returns true if the key is found
// returns false if the key is NOT found
static inline bool
_lc_join(__set_t, contains)(__set_t *s, T key) {
  uint64_t h = s->hash(key);
#ifdef _lc_profile_enabled
  s->hash_or |= h;
  s->hash_and &= h;
#endif // _lc_profile_enabled
  size_t index = (size_t)(h % s->capacity);
  __node_t *cur = s->buckets[index];
  while (cur) {
    if (s->eq(key, cur->data))
      return true;
    cur = cur->next;
  }
  return false;
}

// Returns the pointer to the key if it's found
// inside the set, allowing the user to modify it
static inline T *
_lc_join(__set_t, get)(__set_t *s, T key) {
  uint64_t h = s->hash(key);
#ifdef _lc_profile_enabled
  s->hash_or |= h;
  s->hash_and &= h;
  size_t probe = 0;
#endif // _lc_profile_enabled
  size_t index = (size_t)(h % s->capacity);
  __node_t *cur = s->buckets[index];
  while (cur) {
    if (s->eq(key, cur->data)) {
#ifdef _lc_profile_enabled
      if (probe > s->max_probe)
        s->max_probe = probe;
#endif // _lc_profile_enabled
      return &cur->data;
    }
#ifdef _lc_profile_enabled
    probe++;
#endif // _lc_profile_enabled
    cur = cur->next;
  }
#ifdef _lc_profile_enabled
  if (probe > s->max_probe)
    s->max_probe = probe;
#endif // _lc_profile_enabled
  return NULL;
}

// Removes a key from the hash-set
// Returns true if the element was in the set and it has been deleted
// Returns false if the element was not in the set
static inline bool
_lc_join(__set_t, remove)(__set_t *s, T key) {
  uint64_t h = s->hash(key);
  size_t index = (size_t)(h % s->capacity);
#ifdef _lc_profile_enabled
  s->hash_or |= h;
  s->hash_and &= h;
  size_t probe = 0;
#endif // _lc_profile_enabled
  __node_t *cur = s->buckets[index];
  __node_t *prv = NULL;
  while (cur) {
    if (s->eq(key, cur->data)) {
      if (prv == NULL) {
        s->buckets[index] = cur->next;
        if (s->drop)
          s->drop(cur->data);
        free(cur);
      } else {
        prv->next = cur->next;
        if (s->drop)
          s->drop(cur->data);
        free(cur);
      }
      s->size--;
#ifdef _lc_profile_enabled
      if (probe > s->max_probe)
        s->max_probe = probe;
      s->num_erases++;
#endif // _lc_profile_enabled
      return true;
    }
#ifdef _lc_profile_enabled
    probe++;
#endif // _lc_profile_enabled
    prv = cur;
    cur = cur->next;
  }
#ifdef _lc_profile_enabled
  if (probe > s->max_probe)
    s->max_probe = probe;
#endif // _lc_profile_enabled
  return false;
}

// Deallocates all the hash-set memory
// The user is responsible for freeing the set itself
// if it was heap-allocated
static inline void
_lc_join(__set_t, destroy)(__set_t *s) {
  if (!s)
    return;
  for (size_t i = 0; i < s->capacity; i++) {
    if (s->buckets[i] == NULL)
      continue;
    __node_t *cur = s->buckets[i];
    __node_t *to_free = NULL;
    while (cur) {
      to_free = cur;
      cur = cur->next;
      if (s->drop)
        s->drop(to_free->data);
      free(to_free);
    }
  }
  free(s->buckets);
  s->buckets = NULL;
}

#ifdef _lc_profile_enabled
// Prints a small set of data used to gather information
// about the set performance. The performance of a hash set comes
// down to a simple question: "How good is your hash function?"
// the hash_or and hash_and values tells us if our hash function
// has stuck bits, the ideal case is where hash_or tends to 0xFFFF
// and hash_and to 0
static inline void
_lc_join(__set_t, print_profiling_data)(__set_t *s, const char *name) {
  fprintf(stdout, "[%s profiling data] set ptr = %p\n", name, (void *)s);
  fprintf(stdout, "[Running hash or] %04llx\n", s->hash_or);
  fprintf(stdout, "[Running hash and] %04llx\n", s->hash_and);
  size_t total = 0, max = 0, c = 0, empty = 0;
  for (size_t i = 0; i < s->capacity; i++) {
    c = 0;
    __node_t *n = s->buckets[i];
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
  ASSERT((total == s->size),
         "Something when wrong when calculating profiling data\n");
  fprintf(stdout, "[Maximum bucket length] %zu\n", max);
  fprintf(stdout, "[Average bucket length] %.3f\n",
          (float)total / (float)s->capacity);
  fprintf(stdout, "[Number of empty buckets] %zu\n", empty);
  fprintf(stdout, "[Average length of non empty buckets] %.3f\n",
          (float)total / (float)(s->capacity - empty));
  fprintf(stdout, "[Num erases] %zu\n", s->num_erases);
  fprintf(stdout, "[Maximum probe length] %zu\n", s->max_probe);
}
#endif // _lc_profile_enabled

#undef T
#undef __node_t
#undef __set_t
#undef _lc_nodeset_pfx

#ifdef __cplusplus
}
#endif // __cplusplus
