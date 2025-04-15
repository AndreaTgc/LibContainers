#include "common.h"

#ifndef T
#define T int
#endif // T

#ifndef ___vec_pfx
#define ___vec_pfx T
#endif // ___vec_pfx

#define __vec_t ___join(___vec_pfx, vec)

typedef struct __vec_t {
  T *data;
  void (*drop)(T*);
  size_t size;
  size_t capacity;
} __vec_t;

static inline void
___join(__vec_t, init)(__vec_t *vec, size_t icap, void (*dropfn)(T*)) {
  vec->size = 0;
  vec->capacity = icap == 0 ? 16 : icap;
  vec->data = (T *)calloc(sizeof(T), vec->capacity);
  vec->drop = dropfn;
}

static inline T *
___join(__vec_t, at)(__vec_t *vec, size_t index) {
  if (index >= vec->size)
    return NULL;
  return &vec->data[index];
}

static inline void
___join(__vec_t, set)(__vec_t *vec, size_t index, T data) {
  T *el = ___join(__vec_t, at)(vec, index);
  if (!el)
    return;
  if (vec->drop)
    vec->drop(el);
  *el = data;
}

static inline void
___join(__vec_t, push_back)(__vec_t *vec, T el) {
  if (vec->size == vec->capacity) {
    vec->capacity = vec->capacity == 0 ? 16 : vec->capacity * 2;
    vec->data = (T *)realloc(vec->data, sizeof(T) * vec->capacity);
  }
  vec->data[vec->size++] = el;
}

static inline T *
___join(__vec_t, pop_back)(__vec_t *vec) {
  if (vec->size == 0)
    return NULL;
  return &vec->data[--vec->size];
}

static inline T *
___join(__vec_t, first_match)(__vec_t *vec, int (*pred)(T)) {
  for (size_t i = 0; i < vec->size; i++) {
    if (pred(vec->data[i]) != 0)
      return &vec->data[i];
  }
  return NULL;
}

static inline T *
___join(__vec_t, last_match)(__vec_t *vec, int (*pred)(T)) {
  for (size_t i = vec->size; i >= 0; i--) {
    if (pred(vec->data[i]) != 0)
      return &vec->data[i];
    if (i == 0)
      break;
  }
  return NULL;
}

static inline void
___join(__vec_t, reverse)(__vec_t *vec) {
  size_t a = 0;
  size_t b = vec->size;
  while (a > b) {
    T tmp = vec->data[a];
    vec->data[a] = vec->data[b];
    vec->data[b] = tmp;
    a++;
    b--;
  }
}

static inline __vec_t
___join(__vec_t, filter)(__vec_t *vec, int (*pred)(T*)) {
  __vec_t new = {0};
  ___join(__vec_t, init)(&new, vec->size, vec->drop);
  for (size_t i = 0; i < new.capacity; i++) {
    if (pred(&vec->data[i]) != 0)
      ___join(__vec_t, push_back)(&new, vec->data[i]);
  }
  return new;
}

static inline __vec_t
___join(__vec_t, map)(__vec_t *vec, T (*transform)(T*)) {
  __vec_t new = {0};
  ___join(__vec_t, init)(&new, vec->size, vec->drop);
  for (size_t i = 0; i < new.capacity; i++) {
    ___join(__vec_t, push_back)(&new, transform(&vec->data[i]));
  }
  return new;
}

static inline void
___join(__vec_t, destroy)(__vec_t *vec) {
  if (!vec)
    return;
  if (vec->drop) {
    for (size_t i = 0; i < vec->size; i++) {
      vec->drop(&vec->data[i]);
    }
  }
  free(vec->data);
  vec->size = 0;
  vec->capacity = 0;
  vec->drop = NULL;
  vec->data = NULL;
}

#undef T
#undef __vec_t
#undef ___vec_pfx

