#include "_lc_templating.h"

#ifndef K
#define K int
#endif // K

#ifndef V
#define V int
#endif // V

#ifndef lcore_hash_fn
#define lcore_hash_fn(x) x
#endif // lcore_hash_fn

#ifndef lcore_eq_fn
#define lcore_eq_fn(a, b) ((a) == (b))
#endif // lcore_eq_fn

#ifndef lcore_drop_k
#define lcore_drop_k(x)
#endif // lcore_drop_k

#ifndef lcore_drop_v
#define lcore_drop_v(x)
#endif // lcore_drop_v

#ifndef lcore_pfx
#define lcore_pfx _lc_join(_lc_join(K, V), umap)
#endif // lcore_pfx

#define Self lcore_pfx
#define _Node _lc_join(Self, node)

typedef struct _Node {
  K key;
  V value;
  struct _Node *next;
} _Node;

typedef struct Self {
  size_t size, capacity;
  _Node **buckets;
} Self;

// clang-format off
static inline void _lc_mfunc(init)(Self* self, size_t capacity);
static inline void _lc_mfunc(destroy)(Self* self);
static inline void _lc_mfunc(rehash)(Self* self, size_t new_capacity);
static inline void _lc_mfunc(set)(Self* self, K key, V value);
static inline bool _lc_mfunc(insert)(Self* self, K key, V value);
static inline bool _lc_mfunc(remove)(Self* self, K key, V value);
static inline V*   _lc_mfunc(find)(Self* self, K key);
// clang-format on

static inline void _lc_mfunc(init)(Self *self, size_t capacity) {
  if (capacity == 0 || (capacity & capacity - 1) != 0)
    capacity = 64;
  self->capacity = capacity;
  self->size = 0;
  self->buckets = lc_calloc(_Node *, sizeof(_Node *), self->capacity);
}

static inline void _lc_mfunc(destroy)(Self *self) {
  if (!self)
    return;
  for (size_t i = 0; i < self->capacity; i++) {
    _Node *cur = self->buckets[i];
    _Node *to_free = NULL;
    while (cur) {
      to_free = cur;
      cur = cur->next;
      lcore_drop_k(to_free->key);
      lcore_drop_v(to_free->value);
      free(to_free);
    }
  }
  free(self->buckets);
  memset(self, 0, sizeof(*self));
}

static inline void _lc_mfunc(rehash)(Self *self, size_t new_capacity) {
  if (new_capacity == 0)
    return;
  _Node **new_buckets = lc_calloc(_Node *, sizeof(_Node *), new_capacity);
  if (!new_buckets)
    return;
  _Node **old_buckets = self->buckets;
  size_t old_capacity = self->capacity;
  self->capacity = new_capacity;
  for (size_t i = 0; i < old_capacity; i++) {
    _Node *cur = old_buckets[i];
    while (cur) {
      _Node *next = cur->next;
      uint64_t h = lcore_hash_fn(cur->key);
      size_t b = h & (old_capacity - 1);
      cur->next = new_buckets[b];
      new_buckets[b] = cur;
      cur = next;
    }
  }
  free(old_buckets);
}

static inline void _lc_mfunc(set)(Self *self, K key, V value) {
  V *val = _lc_mfunc(find)(self, key);
  if (!val)
    _lc_mfunc(insert)(self, key, value);
  else
    *val = value;
}

static inline bool _lc_mfunc(insert)(Self *self, K key, V value) {
  uint64_t h = lcore_hash_fn(key);
  size_t b = h & (self->capacity - 1);
  _Node *cur = self->buckets[b];
  _Node *prv = NULL;
  while (cur) {
    if (lcore_eq_fn(cur->key, key))
      return false;
    prv = cur;
    cur = cur->next;
  }
  _Node *new_node = lc_malloc(_Node, sizeof(_Node));
  if (!new_node)
    return false;
  new_node->next = NULL;
  new_node->key = key;
  new_node->value = value;
  if (prv)
    prv->next = new_node;
  else
    self->buckets[b] = new_node;
  return true;
}

static inline bool _lc_mfunc(remove)(Self *self, K key) {
  uint64_t h = lcore_hash_fn(key);
  size_t b = h & (self->capacity - 1);
  _Node *cur = self->buckets[b];
  _Node *prv = NULL;
  while (cur) {
    if (lcore_eq_fn(cur->key, key)) {
      if (prv)
        prv->next = cur->next;
      else
        self->buckets[b] = cur->next;
      lcore_drop_k(cur->key);
      lcore_drop_v(cur->value);
      free(cur);
      return true;
    }
    prv = cur;
    cur = cur->next;
  }
  return false;
}

static inline V *_lc_mfunc(find)(Self *self, K key) {
  uint64_t h = lcore_hash_fn(key);
  size_t b = h & (self->capacity - 1);
  _Node *cur = self->buckets[b];
  while (cur) {
    if (lcore_eq_fn(cur->key, key))
      return &cur->value;
    cur = cur->next;
  }
  return NULL;
}
