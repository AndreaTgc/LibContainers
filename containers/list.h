#include "common.h"

#ifndef T
#define T int
#endif // T

#ifndef _lc_list_pfx
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
_lc_join(__list_t, add_tail)(__list_t *l, T el) {
  if (l->tail == NULL) {
    // Adding the first element to the list
    __lnode_t *n = (__lnode_t *)malloc(sizeof(__lnode_t));
    assert(n);
    n->data = el;
    n->next = NULL;
    n->prev = NULL;
    l->head = n;
    l->tail = n;
    return;
  }
  __lnode_t *n = (__lnode_t *)malloc(sizeof(__lnode_t));
  assert(n);
  n->data = el;
  n->next = NULL;
  n->prev = l->tail;
  l->tail = n;
}

static inline void
_lc_join(__list_t, add_head)(__list_t *l, T el) {
  if (l->head == NULL) {
    // Adding the first element to the list
    __lnode_t *n = (__lnode_t *)malloc(sizeof(__lnode_t));
    assert(n);
    n->data = el;
    n->next = NULL;
    n->prev = NULL;
    l->head = n;
    l->tail = n;
    return;
  }
  __lnode_t *n = (__lnode_t *)malloc(sizeof(__lnode_t));
  assert(n);
  n->data = el;
  n->next = l->head;
  n->prev = NULL;
  l->head = n;
}

static inline void
_lc_join(__list_t, pop_tail)(__list_t *l, T el) {
  if (!l->tail)
    return;
  if (l->drop)
    l->drop(l->tail->data);
  __lnode_t *n = l->tail;
  l->tail = n->prev;
  free(n);
}

static inline void
_lc_join(__list_t, pop_head)(__list_t *l, T el) {
  if (!l->head)
    return;
  if (l->drop)
    l->drop(l->head->data);
  __lnode_t *n = l->head;
  l->head = n->next;
  free(n);
}

#undef T
