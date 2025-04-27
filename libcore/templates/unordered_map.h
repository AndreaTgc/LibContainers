#include "../libcore.h"
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
#define lcore_pfx _lcore_join(_lcore_join(K, V), umap)
#endif // lcore_pfx

#define Self lcore_pfx
#define _Node _lcore_join(Self, node)

typedef struct _Node {
  K key;
  V value;
  struct _Node *next;
} _Node;

typedef struct Self {
  usize size, capacity;
  _Node **buckets;
} Self;

// clang-format off
LCORE_API void _lcore_mfunc(init)(Self* self, usize capacity);
LCORE_API void _lcore_mfunc(destroy)(Self* self);
LCORE_API bool _lcore_mfunc(insert)(Self* self, K key, V value);
LCORE_API bool _lcore_mfunc(remove)(Self* self, K key, V value);
LCORE_API bool _lcore_mfunc(set)(Self* self, K key, V value);
LCORE_API V    _lcore_mfunc(find)(Self* self);
// clang-format on

LCORE_API void
_lcore_mfunc(init)(Self *self, usize capacity) {
  if (capacity == 0 || (capacity & capacity - 1) != 0)
    capacity = 64;
  self->capacity = capacity;
  self->size = 0;
  self->buckets = lcore_calloc(_Node *, sizeof(_Node *), self->capacity);
}

LCORE_API void
_lcore_mfunc(destroy)(Self *self) {
  if (!self)
    return;
  for (usize i = 0; i < self->capacity; i++) {
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

LCORE_API bool
_lcore_mfunc(insert)(Self *self, K key, V value) {
  u64 h = lcore_hash_fn(key);
  usize b = h & (self->capacity - 1);
  _Node *cur = self->buckets[b];
  _Node *prv = NULL;
  while (cur) {
    if (lcore_eq_fn(cur->key, key))
      return false;
    prv = cur;
    cur = cur->next;
  }
  _Node *new_node = lcore_malloc(_Node, sizeof(_Node));
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

LCORE_API bool
_lcore_mfunc(remove)(Self *self, K key) {
  u64 h = lcore_hash_fn(key);
  usize b = h & (self->capacity - 1);
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
