#include "_lc_templating.h"

#ifndef T
#define T int
#endif // T

#ifndef lcore_eq_fn
#define lcore_eq_fn(a, b) ((a) == (b))
#endif // lcore_eq_fn

#ifndef lcore_drop_fn
#define lcore_drop_fn(x)
#endif // lcore_drop_fn

#ifndef lcore_hash_fn
#define lcore_hash_fn(x) x
#endif // lcore_hash_fn

#ifndef lcore_max_loadf
#define lcore_max_loadf 0.85f
#endif // lcore_max_loadf

#ifndef lcore_pfx
#define lcore_pfx _lc_join(T, uset)
#endif // lcore_pfx

#define Self lcore_pfx
#define _Node _lc_join(Self, node)

typedef struct _Node {
  T data;
  struct _Node *next;
} _Node;

typedef struct {
  size_t size, capacity;
  _Node **buckets;
} Self;

// ============== PUBLIC API ==================== //
// These functions are meant to be used to interact with the vector structure
// you are NOT supposed to modify the struct memebers directly.
static inline void _lc_mfunc(init)(Self *self, size_t capacity);
static inline bool _lc_mfunc(insert)(Self *self, T key);
static inline bool _lc_mfunc(contains)(Self *self, T key);
static inline bool _lc_mfunc(remove)(Self *self, T key);
static inline void _lc_mfunc(rehash)(Self *self, size_t new_capacity);
static inline void _lc_mfunc(destroy)(Self *self);

static inline void _lc_mfunc(init)(Self *self, size_t capacity) {
  memset(self, 0, sizeof(*self));
  if (capacity == 0 || (capacity & capacity - 1))
    capacity = 64;
  self->capacity = capacity;
  self->buckets = lc_calloc(_Node *, sizeof(_Node *), capacity);
}

static inline bool _lc_mfunc(insert)(Self *self, T key) {
  if ((float)self->size / self->capacity >= lcore_max_loadf)
    _lc_mfunc(rehash)(self, self->capacity << 1);
  uint64_t h = lcore_hash_fn(key);
  size_t b = h & (self->capacity - 1);
  _Node *cur = self->buckets[b];
  _Node *prv = NULL;
  while (cur) {
    if (lcore_eq_fn(cur->data, key))
      return false;
    prv = cur;
    cur = cur->next;
  }
  _Node *new_node = lc_malloc(_Node, sizeof(_Node));
  if (!new_node)
    return false;
  new_node->next = NULL;
  new_node->data = key;
  if (!prv)
    self->buckets[b] = new_node;
  else
    prv->next = new_node;
  return true;
}

static inline bool _lc_mfunc(contains)(Self *self, T key) {
  uint64_t h = lcore_hash_fn(key);
  size_t b = h & (self->capacity - 1);
  _Node *cur = self->buckets[b];
  while (cur) {
    if (lcore_eq_fn(cur->data, key))
      return true;
    cur = cur->next;
  }
  return false;
}

static inline bool _lc_mfunc(remove)(Self *self, T key) {
  uint64_t h = lcore_hash_fn(key);
  size_t b = h & (self->capacity - 1);
  _Node *cur = self->buckets[b];
  _Node *prv = NULL;
  while (cur) {
    if (lcore_eq_fn(cur->data, key)) {
      if (prv) {
        prv->next = cur->next;
      } else {
        self->buckets[b] = cur->next;
      }
      lcore_drop_fn(cur->data);
      free(cur);
      return true;
    }
    prv = cur;
    cur = cur->next;
  }
  return false;
}

static inline void _lc_mfunc(rehash)(Self *self, size_t new_capacity) {
  if (new_capacity == 0)
    return;
  _Node **new_buckets = lc_calloc(_Node *, sizeof(_Node *), new_capacity);
  if (!new_buckets)
    return;
  _Node **old_buckets = self->buckets;
  size_t old_capacity = self->capacity;
  self->size = 0;
  self->capacity = new_capacity;
  self->buckets = new_buckets;
  for (size_t i = 0; i < old_capacity; i++) {
    _Node *cur = old_buckets[i];
    _Node *nxt = NULL;
    while (cur) {
      nxt = cur->next;
      uint64_t h = lcore_hash_fn(cur->data);
      size_t b = h & (self->capacity - 1);
      cur->next = self->buckets[b];
      self->buckets[b] = cur;
      cur = nxt;
    }
  }
  free(old_buckets);
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
      lcore_drop_fn(to_free->data);
      free(to_free);
    }
  }
  free(self->buckets);
  memset(self, 0, sizeof(*self));
}

#undef T
#undef lcore_max_loadf
#undef lcore_drop_fn
#undef Self
#undef _Node
