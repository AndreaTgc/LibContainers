#include "common.h"

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if !defined(T)
#define T int
#endif // T

#if !defined(_lc_vec_pfx)
#define _lc_vec_pfx T
#define __vec_t _lc_join(_lc_vec_pfx, vec)
#else
#define __vec_t _lc_vec_pfx
#endif // _lc_vec_pfx

typedef struct __vec_t {
  T *data;
  void (*drop)(T *);
  size_t size;
  size_t capacity;
} __vec_t;

static inline void
_lc_join(__vec_t, init)(__vec_t *self, size_t icap, void (*dropfn)(T *)) {
  ASSERT((self != NULL), "Trying to init a NULL vector\n");
  self->size = 0;
  self->capacity = icap == 0 ? 16 : icap;
  self->data = (T *)calloc(sizeof(T), self->capacity);
  self->drop = dropfn;
}

static inline T *
_lc_join(__vec_t, at)(__vec_t *self, size_t index) {
  ASSERT((self != NULL), "Trying to call 'at' on a NULL vector\n");
  if (index >= self->size)
    return NULL;
  return &self->data[index];
}

static inline void
_lc_join(__vec_t, set)(__vec_t *self, size_t index, T data) {
  ASSERT((self != NULL), "Trying to call 'set' on a NULL vector\n");
  T *el = _lc_join(__vec_t, at)(self, index);
  if (!el)
    return;
  if (self->drop)
    self->drop(el);
  *el = data;
}

static inline void
_lc_join(__vec_t, remove_at)(__vec_t *self, size_t index) {
  if (index >= self->size)
    return;
  memmove(&self->data[index], &self->data[index + 1], self->size - index);
  self->size--;
}

static inline void
_lc_join(__vec_t, insert_at)(__vec_t *self, size_t index) {
  if (index >= self->size)
    return;
  if (self->size == self->capacity) {
    self->capacity = self->capacity == 0 ? 16 : self->capacity * 2;
    self->data = (T *)realloc(self->data, sizeof(T) * self->capacity);
  }
  memmove(&self->data[index + 1], &self->data[index], self->size - index);
  self->size++;
}

static inline void
_lc_join(__vec_t, push_back)(__vec_t *self, T el) {
  ASSERT((self != NULL), "Trying to call 'push_back' on a NULL vector\n");
  if (self->size == self->capacity) {
    self->capacity = self->capacity == 0 ? 16 : self->capacity * 2;
    self->data = (T *)realloc(self->data, sizeof(T) * self->capacity);
  }
  self->data[self->size++] = el;
}

static inline T *
_lc_join(__vec_t, pop_back)(__vec_t *self) {
  ASSERT((self != NULL), "Trying to call 'pop_back' on a NULL vector\n");
  if (self->size == 0)
    return NULL;
  return &self->data[--self->size];
}

static inline T *
_lc_join(__vec_t, first_match)(__vec_t *self, int (*pred)(T)) {
  ASSERT((self != NULL), "Trying to call 'first_match' on a NULL vector\n");
  for (size_t i = 0; i < self->size; i++) {
    if (pred(self->data[i]) != 0)
      return &self->data[i];
  }
  return NULL;
}

static inline T *
_lc_join(__vec_t, last_match)(__vec_t *self, int (*pred)(T)) {
  ASSERT((self != NULL), "Trying to call 'last_match' on a NULL vector\n");
  for (size_t i = self->size; i >= 0; i--) {
    if (pred(self->data[i]) != 0)
      return &self->data[i];
    if (i == 0)
      break;
  }
  return NULL;
}

static inline void
_lc_join(__vec_t, reverse)(__vec_t *self) {
  ASSERT((self != NULL), "Trying to call 'reverse' on a NULL vector\n");
  size_t a = 0;
  size_t b = self->size;
  while (a > b) {
    T tmp = self->data[a];
    self->data[a] = self->data[b];
    self->data[b] = tmp;
    a++;
    b--;
  }
}

static inline void
_lc_join(__vec_t, qsort)(__vec_t *self, int (*cmp)(const T *, const T *)) {
  qsort(self->data, self->size, sizeof(T),
        (int (*)(const void *, const void *))cmp);
}

static inline __vec_t
_lc_join(__vec_t, filter)(__vec_t *self, bool (*pred)(T *)) {
  ASSERT((self != NULL), "Trying to call 'filter' on a NULL vector\n");
  __vec_t n = {0};
  _lc_join(__vec_t, init)(&n, self->size, self->drop);
  for (size_t i = 0; i < n.capacity; i++) {
    if (pred(&self->data[i]))
      _lc_join(__vec_t, push_back)(&n, self->data[i]);
  }
  return n;
}

static inline __vec_t
_lc_join(__vec_t, map)(__vec_t *self, T (*transform)(T *)) {
  ASSERT((self != NULL), "Trying to call 'map' on a NULL vector\n");
  __vec_t n = {0};
  _lc_join(__vec_t, init)(&n, self->size, self->drop);
  for (size_t i = 0; i < n.capacity; i++) {
    _lc_join(__vec_t, push_back)(&n, transform(&self->data[i]));
  }
  return n;
}

static inline void
_lc_join(__vec_t, destroy)(__vec_t *self) {
  if (!self)
    return;
  if (self->drop) {
    for (size_t i = 0; i < self->size; i++) {
      self->drop(&self->data[i]);
    }
  }
  free(self->data);
  self->size = 0;
  self->capacity = 0;
  self->drop = NULL;
  self->data = NULL;
}

#undef T
#undef __vec_t
#undef _lc_vec_pfx

#if defined(__cplusplus)
}
#endif // __cplusplus
