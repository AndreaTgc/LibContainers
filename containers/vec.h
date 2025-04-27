#include "common.h"
#include <stdarg.h>
#include <string.h>

// @Author: Andrea Colombo (AndreaTgc)
// Generic type-safe vector (inspried by STL's vector)
// Implementation details
// [1] Stores vector elements on the heap, allowing the vector
// to grow as needed
// [2] Amortized constant time insertion and deletion
// [3] Constant time access

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

#define Self __vec_t

typedef struct Self {
  T *data;
  void (*drop)(T *);
  size_t size;
  size_t capacity;
} Self;

/**
 * @brief initializes the vector
 *
 * @param  self pointer to the vector
 * @param  icap initial capacity for the vector
 * @dropfn function for elements deallocation (optional)
 */
static inline void
_lc_membfunc(init)(Self *self, size_t icap, void (*dropfn)(T *)) {
  ASSERT((self != NULL), "Trying to init a NULL vector\n");
  self->size = 0;
  self->capacity = icap == 0 ? 16 : icap;
  self->data = (T *)calloc(sizeof(T), self->capacity);
  self->drop = dropfn;
}

/**
 * @brief returns the pointer to the value at a given index
 *
 * @param self  pointer to the vector
 * @param index index of the element to access
 */
static inline T *
_lc_membfunc(at)(Self *self, size_t index) {
  ASSERT((self != NULL), "Trying to call 'at' on a NULL vector\n");
  if (index >= self->size)
    return NULL;
  return &self->data[index];
}

/**
 * @brief sets the value of the element at a given index
 *
 * @param self  pointer to the vector
 * @param index index of the element to set
 * @param val   value to assign to the element
 */
static inline void
_lc_membfunc(set)(Self *self, size_t index, T val) {
  ASSERT((self != NULL), "Trying to call 'set' on a NULL vector\n");
  T *elem = _lc_membfunc(at)(self, index);
  if (!elem)
    return;
  if (self->drop)
    self->drop(elem);
  *elem = val;
}

/**
 * @brief removes the element at a given index, shifting back
 * the rest of the vector.
 *
 * @param self  pointer to the vector
 * @param index index of the element to remove
 */
static inline void
_lc_membfunc(remove_at)(Self *self, size_t index) {
  if (index >= self->size)
    return;
  memmove(&self->data[index], &self->data[index + 1], self->size - index);
  self->size--;
}

/**
 * @brief inserts a new element at a given index
 *
 * @param self  pointer to the vector
 * @param index index for the new element insertion
 * @param data  value to insert at the given index
 */
static inline void
_lc_membfunc(insert_at)(Self *self, size_t index, T val) {
  if (index >= self->size)
    return;
  if (self->size == self->capacity) {
    self->capacity = self->capacity == 0 ? 16 : self->capacity * 2;
    self->data = (T *)realloc(self->data, sizeof(T) * self->capacity);
  }
  memmove(&self->data[index + 1], &self->data[index], self->size - index);
  self->data[index] = val;
  self->size++;
}

/**
 * @brief appends a new element to the vector
 *
 * @param self pointer to the vector
 * @param val  value to append
 */
static inline void
_lc_membfunc(push_back)(Self *self, T val) {
  ASSERT((self != NULL), "Trying to call 'push_back' on a NULL vector\n");
  if (self->size == self->capacity) {
    self->capacity = self->capacity == 0 ? 16 : self->capacity * 2;
    self->data = (T *)realloc(self->data, sizeof(T) * self->capacity);
  }
  self->data[self->size++] = val;
}

/**
 * @brief removes the last element of the vector, returning the pointer
 * to it.
 *
 * @brief self pointer to the vector
 */
static inline T *
_lc_membfunc(pop_back)(Self *self) {
  ASSERT((self != NULL), "Trying to call 'pop_back' on a NULL vector\n");
  if (self->size == 0)
    return NULL;
  return &self->data[--self->size];
}

/**
 * @brief returns the pointer to the first element of the vector that satisfies
 * a unary predicate (NULL if none matches)
 *
 * @param self pointer to the vector
 * @param pred unary predicate to match
 */
static inline T *
_lc_membfunc(first_match)(Self *self, int (*pred)(T)) {
  ASSERT((self != NULL), "Trying to call 'first_match' on a NULL vector\n");
  for (size_t i = 0; i < self->size; i++) {
    if (pred(self->data[i]) != 0)
      return &self->data[i];
  }
  return NULL;
}

/**
 * @brief returns the pointer to the last element of the vector that satisfies
 * a unary predicate (NULL if none matches)
 *
 * @param self pointer to the vector
 * @param pred unary predicate to match
 */
static inline T *
_lc_membfunc(last_match)(Self *self, int (*pred)(T)) {
  ASSERT((self != NULL), "Trying to call 'last_match' on a NULL vector\n");
  for (size_t i = self->size; i >= 0; i--) {
    if (pred(self->data[i]) != 0)
      return &self->data[i];
    if (i == 0)
      break;
  }
  return NULL;
}

/**
 * @brief reverses the order of the elements inside the vector
 *
 * @param self pointer to the vector
 */
static inline void
_lc_membfunc(reverse)(Self *self) {
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

/**
 * @brief sorts the vector using the C stdlib quicksort implementation
 *
 * @param self pointer to the vector
 * @param cmp  function that compares two Ts (required for qsort)
 */
static inline void
_lc_membfunc(qsort)(Self *self, int (*cmp)(const T *, const T *)) {
  qsort(self->data, self->size, sizeof(T),
        (int (*)(const void *, const void *))cmp);
}

/**
 * @brief takes an arbitrary number of vectors of type T, combines all the
 * elements and sorts them. The elements are then put back inside the vectors.
 *
 * @param cmp function that compares two Ts (required for sorting)
 */
static inline bool
_lc_membfunc(sort_chained)(int (*cmp)(const T *, const T *), ...) {
  va_list args;
  va_start(args, cmp);
  Self *cur;
  size_t tot_size = 0;
  while ((cur = va_arg(args, Self *)) != NULL)
    tot_size += cur->size;
  va_end(args);

  if (tot_size == 0)
    return true;

  T *flat = (T *)calloc(tot_size, sizeof(T));

  if (!flat)
    return false;

  va_start(args, cmp);
  size_t offset = 0;
  while ((cur = va_arg(args, Self *)) != NULL) {
    memcpy(&flat[offset], cur->data, cur->size * sizeof(T));
    offset += cur->size;
  }
  va_end(args);

  qsort(flat, tot_size, sizeof(T), (int (*)(const void *, const void *))(cmp));

  va_start(args, cmp);
  offset = 0;
  while ((cur = va_arg(args, Self *)) != NULL) {
    memcpy(cur->data, &flat[offset], cur->size * sizeof(T));
    offset += cur->size;
  }
  va_end(args);
  free(flat);
  return true;
}

/**
 * @brief returns a new vector containing only the elements of the first vector
 * that satisfy a given unary predicate
 *
 * @param self pointer to the vector
 * @param pred unary predicate for filter
 */
static inline Self
_lc_membfunc(filter)(Self *self, bool (*pred)(T *)) {
  ASSERT((self != NULL), "Trying to call 'filter' on a NULL vector\n");
  Self n = {0};
  _lc_membfunc(init)(&n, self->size, self->drop);
  for (size_t i = 0; i < n.capacity; i++) {
    if (pred(&self->data[i]))
      _lc_membfunc(push_back)(&n, self->data[i]);
  }
  return n;
}

/**
 * @brief maps a vector to a new one of the same type using a function
 * passed as parameter
 *
 * @param self      pointer to the vector
 * @param transform function that maps a T to a new T
 */
static inline Self
_lc_membfunc(map)(Self *self, T (*transform)(T *)) {
  ASSERT((self != NULL), "Trying to call 'map' on a NULL vector\n");
  Self n = {0};
  _lc_membfunc(init)(&n, self->size, self->drop);
  for (size_t i = 0; i < n.capacity; i++) {
    _lc_membfunc(push_back)(&n, transform(&self->data[i]));
  }
  return n;
}

/**
 * @brief deallocates the memory linked to a vector, resetting it to a "base"
 * state
 *
 * @param self pointer to the vector
 */
static inline void
_lc_membfunc(destroy)(Self *self) {
  if (!self)
    return;
  if (self->drop) {
    for (size_t i = 0; i < self->size; i++) {
      self->drop(&self->data[i]);
    }
  }
  free(self->data);
  memset(self, 0, sizeof(*self));
}

#undef T
#undef Self
#undef __vec_t
#undef _lc_vec_pfx

#if defined(__cplusplus)
}
#endif // __cplusplus
