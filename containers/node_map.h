#include "common.h"

// @Author: Andrea Colombo (AndreaTgc)
// Implementation of a hash table inspired by C++'s std::unordered_map
// Implementation details:
// [1] Handles collisions with open hashing (linked lists)
// [2] Offers pointer stability, even after resizing
// [3] Takes full ownership of the key and values (eg. if you use char* as
// the key, you do not deallocate the string after calling 'insert', the map
// will handle the string deallocation when calling 'remove' or 'destroy')

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if !defined(K)
#define K int
#endif // K

#if !defined(V)
#define V int
#endif // V

#if !defined(_lc_nodemap_lf)
#define _lc_nodemap_lf 0.8
#endif // _lc_nodemap_lf

#if !defined(_lc_nodemap_pfx)
#define _lc_nodemap_pfx _lc_join(K, V)
#define __map_t _lc_join(_lc_nodemap_pfx, node_map)
#else
#define __map_t _lc_nodemap_pfx
#endif // _lc_nodemap_pfx

#define __node_t _lc_join(_lc_nodemap_pfx, node)

typedef struct __node_t {
  K key;
  V value;
  struct __node_t *next;
} __node_t;

typedef struct __map_t {
  size_t size;
  size_t capacity;
#if defined(_lc_profile_enabled)
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

/**
 * @brief default key equality function, used if the user
 * doesn't provide a custom one.
 */
static inline bool
_lc_join(__map_t, default_key_eq)(K a, K b) {
  return a == b;
}

/**
 * @brief initializes a map
 *
 * @param self   pointer to the map
 * @param icap   initial number of buckets
 * @param hash   user provided hash function for T
 * @param k_eq   equality function for T
 * @param drop_k function used for deallocating keys (optional)
 * @param drop_v function used for deallocating values (optional)
 */
static inline void
_lc_join(__map_t, init)(__map_t *self, size_t icap, uint64_t (*hash)(K),
                        bool (*k_eq)(K, K), void (*drop_k)(K),
                        void (*drop_v)(V)) {
  self->size = 0;
  self->capacity = icap == 0 ? 32 : icap;
  self->hash = hash;
  self->key_eq = k_eq ? k_eq : _lc_join(__map_t, default_key_eq);
  self->buckets = (__node_t **)calloc(self->capacity, sizeof(__node_t *));
#if defined(_lc_profile_enabled)
  self->hash_or = 0;
  self->hash_and = UINT64_MAX;
  self->num_erases = 0;
  self->max_probe = 0;
#endif // _lc_profile_enabled
}

/**
 * @brief rehashes the map using a new number of buckets
 *
 * @param self pointer to the map
 * @param cap  new number of buckets to allocate
 */
static inline void
_lc_join(__map_t, rehash)(__map_t *self, size_t cap) {
  ASSERT((self != NULL), "Trying to call 'resize' on a NULL map\n");
  __node_t **new_buckets = (__node_t **)calloc(cap, sizeof(__node_t *));
  __node_t **old_buckets = self->buckets;
  for (size_t i = 0; i < self->capacity; i++) {
    __node_t *cur = self->buckets[i];
    __node_t *next = NULL;
    while (cur) {
      next = cur->next;
      uint64_t new_idx = self->hash(cur->key) % cap;
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
  self->buckets = new_buckets;
  free(old_buckets);
  self->capacity = cap;
}

/**
 * @brief inserts a new (K, V) pair inside the map
 * returns true if the pair is aadded to the map
 * return false if the key was already present in the map
 *
 * @param self pointer to the map
 * @param key  key to add to the map
 * @param val  value associated to the key
 */
static inline bool
_lc_join(__map_t, insert)(__map_t *self, K key, V val) {
  if ((float)self->size / (float)self->capacity > _lc_nodemap_lf)
    _lc_join(__map_t, rehash)(self, self->capacity * 2);
  uint64_t h = self->hash(key);
#if defined(_lc_profile_enabled)
  self->hash_or |= h;
  self->hash_and &= h;
#endif // _lc_profile_enabled
  size_t index = h % self->capacity;
  __node_t *cur = self->buckets[index];
  __node_t *prv = NULL;
  while (cur) {
    if (self->key_eq(cur->key, key))
      return false;
    prv = cur;
    cur = cur->next;
  }
  __node_t *new_node = (__node_t *)malloc(sizeof(__node_t));
  new_node->next = NULL;
  new_node->key = key;
  new_node->value = val;
  if (!prv)
    self->buckets[index] = new_node;
  else
    prv->next = new_node;
  self->size++;
  return true;
}

/**
 * @brief checks if a key is inside the map
 * returns true if the key is in the map
 * returns false otherwise
 *
 * @param self pointer to the map
 * @param key  key to check
 */
static inline bool
_lc_join(__map_t, contains)(__map_t *self, K key) {
  uint64_t h = self->hash(key);
  size_t index = (size_t)(h % self->capacity);
  __node_t *cur = self->buckets[index];
  while (cur) {
    if (self->key_eq(cur->key, key))
      return true;
    cur = cur->next;
  }
  return false;
}

/**
 * @brief returns the pointer to the value associated with a given key.
 * returns NULL if the key is not found inside the map
 *
 * @param self pointer to the map
 * @param key  key to find
 */
static inline V *
_lc_join(__map_t, find)(__map_t *self, K key) {
  uint64_t h = self->hash(key);
  size_t index = (size_t)(h % self->capacity);
  __node_t *cur = self->buckets[index];
  while (cur) {
    if (self->key_eq(cur->key, key))
      return &cur->value;
    cur = cur->next;
  }
  return NULL;
}

/**
 * @brief tries to find a key inside the map.
 * If the key is found, it returns true and copies the value associated to the
 * key in the 'out' parameter
 *
 * @param self pointer to the map
 * @param key  key we want to find
 * @param out  pointer for value copying
 */
static inline bool
_lc_join(__map_t, get)(__map_t *self, K key, V *out) {
  V *tmp = _lc_join(__map_t, find)(self, key);
  if (tmp) {
    *out = *tmp;
    return true;
  } else
    return false;
}

/**
 * @brief removes a key and the value associated to it from the map
 * returns true if the (K, V) pair is removed
 * returns false if the key is not found in the map
 *
 * @param self pointer to the map
 * @param key  key to remove
 */
static inline bool
_lc_join(__map_t, remove)(__map_t *self, K key) {
  uint64_t h = self->hash(key);
  size_t index = (size_t)(h % self->capacity);
  __node_t *cur = self->buckets[index];
  __node_t *prv = NULL;
  while (cur) {
    if (self->key_eq(cur->key, key)) {
      if (!prv) {
        self->buckets[index] = cur->next;
      } else {
        prv->next = cur->next;
      }
      if (self->drop_key)
        self->drop_key(cur->key);
      if (self->drop_val)
        self->drop_val(cur->value);
      free(cur);
      self->size--;
#if defined(_lc_profile_enabled)
      self->num_erases++;
#endif // _lc_profile_enabled
      return true;
    }
    prv = cur;
    cur = cur->next;
  }
  return false;
}

/**
 * @brief deallocates the memory associated to a map, resetting it
 * to a "base" state
 *
 * @param self pointer to the map
 */
static inline void
_lc_join(__map_t, destroy)(__map_t *self) {
  if (!self)
    return;
  for (size_t i = 0; i < self->capacity; i++) {
    __node_t *cur = self->buckets[i];
    __node_t *to_free = NULL;
    while (cur) {
      to_free = cur;
      if (self->drop_key)
        self->drop_key(cur->key);
      if (self->drop_val)
        self->drop_val(cur->value);
      cur = cur->next;
      free(to_free);
    }
  }
  free(self->buckets);
  memset(self, 0, sizeof(*self));
}

#undef __node_t
#undef __map_t
#undef K
#undef V
#undef _lc_nodemap_pfx

#ifdef __cplusplus
}
#endif // __cplusplus
