#ifndef vec_H_
#define vec_H_

// Macro-based implementation of a generic vector
// USAGE: you can either use the 'CTL_DEF_VEC_DEFAULT' macro or 'CTL_DEF_VEC'
// CTL_DEF_VEC_DEFAULT generates a full vector implementation based on the type
// you pass into the macro (the vector type will be named 'type'_vec_t)
// CTL_DEF_VEC does the same thing but uses the prefix you pass as second
// parameter to generate the vector type name ('prefix'vector_t)
//
// This vector implementation is currently a work in progress but here's a list
// of the generates functions that you'll be able to use out of the box, please
// note that i'll omit the prefix in the functions name as it will be generated
// based on the macro you use.
//
// - vec_init: initializes the vector with a given capacity (if you pass 0 as a
// parameter it defaults to 16) and a 'drop' function that will be used in
// 'vec_destroy' (if you define the vector with a "base" type like int you can
// simply pass NULL)
//
// - vec_reserve: grows the vector up to the desired capacity, doesn't grow if 
// the vector's capacity is bigger than the requested one
//
// - vec_push_back: appends an element of type T (the type used in the vec_def
// macro) to the back of the vector, automatically growing the vector when
// needed
//
// - vec_pop_back: removes the element at the end of the vector and returns its
// value, currently does not trigger a realloc if the vector's size goes below a
// certain threshold
//
// - vec_at: returns the value of the element found at the index passed as
// parameter, trying to access the vector out of bounds triggers an assertion
// fail. NOTE: if you compile your program with NDEBUG the assertion will not be
// triggered, resulting in undefined behaviour
//
// - vec_first_match: returns the index of the first value that matches a
// predicate passed as parameter, the predicate is a function that takes a
// single arguemnt of type T and returns a boolean. returns -1 there isn't an
// element that matches that predicate
//
// - vec_last_match: same as 'vec_first_match' but returns the index of the last
// element that matches the predicate
//
// - vec_destroy: deallocates the vector's heap-allocated memory (optionally
// calls the 'drop' function passed in 'vec_init')

#include <assert.h>
#include <stdlib.h>

#define CTL_DEF_VEC_DEFAULT(T) CTL_DEF_VEC(T, T##_)

#define CTL_DEF_VEC(T, PREFIX)                                                 \
  typedef struct PREFIX##vec {                                                 \
    size_t size;                                                               \
    size_t capacity;                                                           \
    void (*drop)(T);                                                           \
    T *data;                                                                   \
  } PREFIX##vec_t;                                                             \
                                                                               \
  void PREFIX##vec_init(PREFIX##vec_t *vec, size_t initial_cap,                \
                        void (*drop)(T)) {                                     \
    vec->capacity = initial_cap == 0 ? 16 : initial_cap;                       \
    vec->size = 0;                                                             \
    vec->data = calloc(vec->capacity, sizeof(T));                              \
    vec->drop = drop;                                                          \
  }                                                                            \
                                                                               \
  void PREFIX##vec_reserve(PREFIX##vec_t *vec, size_t c) {                     \
    if (c > vec->capacity) {                                                   \
      realloc(vec->data, sizeof(T) * c);                                       \
      vec->capacity = c;                                                       \
    }                                                                          \
  }                                                                            \
                                                                               \
  void PREFIX##vec_push_back(PREFIX##vec_t *vec, T val) {                      \
    if (vec->size == vec->capacity) {                                          \
      vec->capacity = vec->capacity == 0 ? 16 : vec->capacity * 2;             \
      vec->data = realloc(vec->data, vec->capacity * sizeof(T));               \
    }                                                                          \
    vec->data[vec->size++] = val;                                              \
  }                                                                            \
                                                                               \
  T PREFIX##vec_pop_back(PREFIX##vec_t *vec) {                                 \
    assert(vec->size > 0);                                                     \
    return vec->data[--vec->size];                                             \
  }                                                                            \
                                                                               \
  T PREFIX##vec_at(PREFIX##vec_t *vec, size_t index) {                         \
    assert(index < vec->size);                                                 \
    return vec->data[index];                                                   \
  }                                                                            \
                                                                               \
  int PREFIX##vec_first_match(PREFIX##vec_t *vec, bool (*pred)(T)) {           \
    for (size_t i = 0; i < vec->size; i++) {                                   \
      if (pred(PREFIX##vec_at(vec, i)))                                        \
        return (int)i;                                                         \
    }                                                                          \
    return -1;                                                                 \
  }                                                                            \
                                                                               \
  int PREFIX##vec_last_match(PREFIX##vec_t *vec, bool (*pred)(T)) {            \
    if (vec->size == 0)                                                        \
      return -1;                                                               \
    for (size_t i = vec->size - 1;; i--) {                                     \
      if (pred(PREFIX##vec_at(vec, i)))                                        \
        return (int)i;                                                         \
      if (i == 0)                                                              \
        break;                                                                 \
    }                                                                          \
    return -1;                                                                 \
  }                                                                            \
                                                                               \
  void PREFIX##vec_destroy(PREFIX##vec_t *vec) {                               \
    if (!vec)                                                                  \
      return;                                                                  \
    if (vec->drop)                                                             \
      for (size_t i = 0; i < vec->size; i++)                                   \
        vec->drop(PREFIX##vec_at(vec, i));                                     \
    if (vec->data) {                                                           \
      free(vec->data);                                                         \
    }                                                                          \
    vec->size = 0;                                                             \
    vec->capacity = 0;                                                         \
    vec->drop = NULL;                                                          \
  }

#endif // vec_H_
