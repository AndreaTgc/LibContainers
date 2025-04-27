#include "../libcore.h"
#include "_lc_templating.h"

#ifndef T
#define T int
#endif // T

#ifndef lcore_pfx
#define lcore_pfx _lcore_join(T, list)
#endif // lcore_pfx

#define Self lcore_pfx
#define _Node _lcore_join(Self, node)

typedef struct _Node {
  T data;
  struct _Node *prev, *next;
} _Node;

typedef struct Self {
  _Node *head, *tail;
} Self;

// clang-format off
LCORE_API void _lcore_mfunc(add_tail)(Self* self, T value);
LCORE_API void _lcore_mfunc(add_head)(Self* self, T value);
LCORE_API void _lcore_mfunc(destroy)(Self* self);
LCORE_API T    _lcore_mfunc(pop_head)(Self* self);
LCORE_API T    _lcore_mfunc(pop_tail)(Self* self);
// clang-format on

LCORE_API void
_lcore_mfunc(add_tail)(Self *self, T value) {
  _Node *n = lcore_malloc(_Node, sizeof(_Node));
  if (!n)
    return;
  n->data = value;
  n->prev = n->next = NULL;
  if (!self->tail)
    self->tail = self->head = n;
  else {
    n->prev = self->tail;
    self->tail->next = n;
    self->tail = n;
  }
}

LCORE_API void
_lcore_mfunc(add_head)(Self *self, T value) {
  _Node *n = lcore_malloc(_Node, sizeof(_Node));
  if (!n)
    return;
  n->data = value;
  n->prev = n->next = NULL;
  if (!self->head)
    self->head = self->tail = n;
  else {
    n->next = self->head;
    self->head->prev = n;
    self->head = n;
  }
}
