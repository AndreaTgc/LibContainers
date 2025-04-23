#include "common.h"

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if !defined(T)
#define T int
#endif // T

#if !defined(_lc_list_pfx)
#define _lc_list_pfx T
#define __list_t _lc_join(_lc_list_pfx, list)
#else
#define __list_t _lc_list_pfx
#endif // _lc_list_pfx

#define __lnode_t _lc_join(_lc_list_pfx, node)

typedef struct __lnode_t {
  struct __lnode_t *prev;
  struct __lnode_t *next;
  T data;
} __lnode_t;

typedef struct __list_t {
  __lnode_t *head;
  __lnode_t *tail;
  void (*drop)(T);
} __list_t;

static inline void
_lc_join(__list_t, add_tail)(__list_t *self, T el) {
  if (self->tail == NULL) {
    // Adding the first element to the list
    __lnode_t *n = (__lnode_t *)malloc(sizeof(__lnode_t));
    assert(n);
    n->data = el;
    n->next = NULL;
    n->prev = NULL;
    self->head = n;
    self->tail = n;
    return;
  }
  __lnode_t *n = (__lnode_t *)malloc(sizeof(__lnode_t));
  assert(n);
  n->data = el;
  n->next = NULL;
  n->prev = self->tail;
  self->tail = n;
}

static inline void
_lc_join(__list_t, add_head)(__list_t *self, T el) {
  if (self->head == NULL) {
    // Adding the first element to the list
    __lnode_t *n = (__lnode_t *)malloc(sizeof(__lnode_t));
    assert(n);
    n->data = el;
    n->next = NULL;
    n->prev = NULL;
    self->head = n;
    self->tail = n;
    return;
  }
  __lnode_t *n = (__lnode_t *)malloc(sizeof(__lnode_t));
  assert(n);
  n->data = el;
  n->next = self->head;
  n->prev = NULL;
  self->head = n;
}

static inline void
_lc_join(__list_t, pop_tail)(__list_t *self) {
  if (!self->tail)
    return;
  if (self->drop)
    self->drop(self->tail->data);
  __lnode_t *n = self->tail;
  self->tail = n->prev;
  free(n);
}

static inline void
_lc_join(__list_t, pop_head)(__list_t *self) {
  if (!self->head)
    return;
  if (self->drop)
    self->drop(self->head->data);
  __lnode_t *n = self->head;
  self->head = n->next;
  free(n);
}

static inline void
_lc_join(__list_t, destroy)(__list_t *self) {
  if (!self)
    return;
  while (self->head)
    _lc_join(__list_t, pop_head)(self);
}

#undef T

#if defined(__cplusplus)
}
#endif // __cplusplus
