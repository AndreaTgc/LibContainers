#include "_lc_templating.h"
#include <stdbool.h>

#ifndef T
#define T int
#endif // T

#ifndef lcore_drop_fn
#define lcore_drop_fn(x)
#endif // lcore_drop_fn

#ifndef lcore_pfx
#define lcore_pfx _lc_join(T, vec)
#endif // lcore_pfx

#define Self lcore_pfx

typedef struct {
  size_t size, capacity;
  T *elements;
} Self;

// ============== PUBLIC API ==================== //
// These functions are meant to be used to interact with the vector structure
// you are NOT supposed to modify the struct memebers directly.

static inline void _lc_mfunc(init)(Self *self, size_t capacity);
static inline void _lc_mfunc(push_back)(Self *self, T value);
static inline void _lc_mfunc(resize)(Self *self, size_t new_capacity);
static inline void _lc_mfunc(insert_at)(Self *self, T value, size_t index);
static inline void _lc_mfunc(remove_at)(Self *self, size_t index);
static inline void _lc_mfunc(qsort)(Self *self, int (*cmp)(const T *, const T *));
static inline void _lc_mfunc(destroy)(Self *self);
static inline T _lc_mfunc(at)(Self *self, size_t index);
static inline T _lc_mfunc(pop_back)(Self *self);
static inline bool _lc_mfunc(check_health)(Self *self);

static inline void
_lc_mfunc(init)(Self *self, size_t capacity) {
  memset(self, 0, sizeof(*self));
  self->elements = lc_calloc(T, sizeof(T), capacity);
  self->capacity = capacity;
}

static inline void
_lc_mfunc(push_back)(Self *self, T value) {
  if (self->size == self->capacity)
    _lc_mfunc(resize)(self, self->capacity * 2);
  self->elements[self->size++] = value;
}

static inline void
_lc_mfunc(resize)(Self *self, size_t new_capacity) {
  self->capacity = new_capacity == 0 ? 16 : new_capacity;
  self->elements = (T *)realloc(self->elements, self->capacity * sizeof(T));
}

static inline void
_lc_mfunc(insert_at)(Self *self, T value, size_t index) {
  if (index >= self->size)
    return;
  if (self->size == self->capacity)
    _lc_mfunc(resize)(self, self->capacity * 2);
  void *dst = self->elements + (sizeof(T) * (index + 1));
  void *src = self->elements + (sizeof(T) * index);
  memmove(dst, src, self->size - index);
  self->size++;
}

static inline void
_lc_mfunc(remove_at)(Self *self, size_t index) {
  void *dst = self->elements + (sizeof(T) * index);
  void *src = self->elements + (sizeof(T) * (index + 1));
  memmove(dst, src, self->size - index - 1);
  self->size--;
}

static inline void
_lc_mfunc(qsort)(Self *self, int (*cmp)(const T *, const T *)) {
  qsort(self->elements, self->size, sizeof(T),
        (int (*)(const void *, const void *))cmp);
}

static inline void
_lc_mfunc(destroy)(Self *self) {
#ifdef lcore_dropfn
  // this code is required for heap allocated types.
  // How to use this feature:
  // Right before includind the header file, the user can
  // define a 'lcore_dropfn' macro that wraps a function that deallocates
  // the type T used in the vector
  //
  // #define lcore_dropfn(x) free(x)
  // #include "libcore/templates/vector.h"
  for (size_t i = 0; i < self->size; i++) {
    lcore_dropfn(self->elements[i]);
  }
#endif // lcore_dropfn
  free(self->elements);
  memset(self, 0, sizeof(*self));
}

static inline T
_lc_mfunc(at)(Self *self, size_t index) {
  assert(index < self->size);
  return self->elements[index];
}

static inline T
_lc_mfunc(pop_back)(Self *self) {
  assert(self->size > 0);
  return self->elements[--self->size];
}

static inline bool
_lc_mfunc(check_health)(Self *self) {
  return (self->size <= self->capacity && self->elements != NULL);
}

#undef T
#undef Self
#undef lcore_dropfn
#undef lcore_pfx
