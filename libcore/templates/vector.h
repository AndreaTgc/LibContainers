#include "../libcore.h"
#include "_lc_templating.h"
#include <stdbool.h>

#ifndef T
#define T int
#endif // T

#ifndef lcore_pfx
#define lcore_pfx _lcore_join(T, vec)
#endif // lcore_pfx

#define Self lcore_pfx

typedef struct {
  usize size, capacity;
  T *elements;
} Self;

// clang-format off
LCORE_API void _lcore_mfunc(init)(Self *self, usize capacity);
LCORE_API void _lcore_mfunc(push_back)(Self *self, T value);
LCORE_API void _lcore_mfunc(resize)(Self *self, usize new_capacity);
LCORE_API void _lcore_mfunc(insert_at)(Self *self, T value, usize index);
LCORE_API void _lcore_mfunc(remove_at)(Self* self, usize index);
LCORE_API void _lcore_mfunc(qsort)(Self* self, int (*cmp)(const T*, const T*));
LCORE_API void _lcore_mfunc(destroy)(Self* self);
LCORE_API T    _lcore_mfunc(at)(Self *self, usize index);
LCORE_API T    _lcore_mfunc(pop_back)(Self *self);
LCORE_API bool _lcore_mfunc(check_health)(Self* self);
// clang-format on

LCORE_API void
_lcore_mfunc(init)(Self *self, usize capacity) {
  memset(self, 0, sizeof(*self));
  self->elements = lcore_calloc(T, sizeof(T), capacity);
  self->capacity = capacity;
}

LCORE_API void
_lcore_mfunc(push_back)(Self *self, T value) {
  if (self->size == self->capacity)
    _lcore_mfunc(resize)(self, self->capacity * 2);
  self->elements[self->size++] = value;
}

LCORE_API void
_lcore_mfunc(resize)(Self *self, usize new_capacity) {
  self->capacity = new_capacity == 0 ? 16 : new_capacity;
  self->elements = (T *)realloc(self->elements, self->capacity * sizeof(T));
}

LCORE_API void
_lcore_mfunc(insert_at)(Self *self, T value, usize index) {
  if (index >= self->size)
    return;
  if (self->size == self->capacity)
    _lcore_mfunc(resize)(self, self->capacity * 2);
  void *dst = self->elements + (sizeof(T) * (index + 1));
  void *src = self->elements + (sizeof(T) * index);
  memmove(dst, src, self->size - index);
  self->size++;
}

LCORE_API void
_lcore_mfunc(remove_at)(Self *self, usize index) {
  void *dst = self->elements + (sizeof(T) * index);
  void *src = self->elements + (sizeof(T) * (index + 1));
  memmove(dst, src, self->size - index - 1);
  self->size--;
}

LCORE_API void
_lcore_mfunc(qsort)(Self *self, int (*cmp)(const T *, const T *)) {
  qsort(self->elements, self->size, sizeof(T),
        (int (*)(const void *, const void *))cmp);
}

LCORE_API void
_lcore_mfunc(destroy)(Self *self) {
#ifdef lcore_dropfn
  // this code is required for heap allocated types.
  // How to use this feature:
  // Right before includind the header file, the user can
  // define a 'lcore_dropfn' macro that wraps a function that deallocates
  // the type T used in the vector
  //
  // #define lcore_dropfn(x) free(x)
  // #include "libcore/templates/vector.h"
  for (usize i = 0; i < self->size; i++) {
    lcore_dropfn(self->elements[i]);
  }
#endif // lcore_dropfn
  free(self->elements);
  memset(self, 0, sizeof(*self));
}

LCORE_API T
_lcore_mfunc(at)(Self *self, usize index) {
  assert(index < self->size);
  return self->elements[index];
}

LCORE_API T
_lcore_mfunc(pop_back)(Self *self) {
  assert(self->size > 0);
  return self->elements[--self->size];
}

LCORE_API bool
_lcore_mfunc(check_health)(Self *self) {
  return (self->size <= self->capacity && self->elements != NULL);
}

#undef T
#undef Self
#undef lcore_dropfn
#undef lcore_pfx
