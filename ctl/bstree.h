#include "common.h"

#ifndef T
#define T int
#endif // T

#ifndef ___bst_pfx
#define ___bst_pfx T
#endif // ___bst_pfx

#define __bsnode_t ___join(___bst_pfx, bsnode)
#define __bstree_t ___join(___bst_pfx, bstree)

typedef struct __bsnode_t {
  struct __bsnode_t *l, *r;
  T data;
} __bsnode_t;

typedef struct __bstree_t {
  size_t size;
  __bsnode_t *root;
  int (*cmp)(T *, T *);
  void (*dropfn)(T *);
} __bstree_t;

static inline void
___join(__bstree_t, init)(__bstree_t *t, int (*cmp)(T *, T *),
                          void (*drop)(T *)) {
  ASSERT((t != NULL), "Trying to init a NULL binary search tree\n");
  t->size = 0;
  t->root = NULL;
  ASSERT((cmp != NULL),
         "Passing NULL for cmp parameter in binary search tree init\n");
  t->cmp = cmp;
  t->dropfn = drop;
}

static inline void
___join(__bstree_t, insert)(__bstree_t *t, T *data) {
  ASSERT((t != NULL), "Trying to insert into a NULL binary search tree\n");
  ASSERT((t->cmp != NULL),
         "Trying to insert into a binary search tree without cmp function");
  __bsnode_t **n = &t->root;
  while (*n) {
    int cmp = t->cmp(&(*n)->data, data);
    if (cmp < 0)
      n = &(*n)->r;
    else if (cmp > 0)
      n = &(*n)->l;
    else
      return;
  }
  *n = (__bsnode_t *)malloc(sizeof(__bsnode_t));
  ASSERT((*n != NULL), "Failed to allocate new node for insertion");
  (*n)->data = *data;
  (*n)->l = NULL;
  (*n)->r = NULL;
  t->size++;
}

static inline T *
___join(__bstree_t, find)(__bstree_t *t, T *data) {
  ASSERT((t != NULL), "Trying to call find on a NULL tree\n");
  ASSERT((t->cmp != NULL),
         "Trying to call find on a tree with no cmp function\n");
  __bsnode_t *n = t->root;
  while (n) {
    if (t->cmp(&n->data, data) == 0)
      return &n->data;
    if (t->cmp(&n->data, data) > 0)
      n = n->l;
    else
      n = n->r;
  }
  return NULL;
};

static inline size_t
___join(__bsnode_t, depth_rec)(__bsnode_t *n, size_t curr) {
  if (!n)
    return 0;
  size_t l = ___join(__bsnode_t, depth_rec)(n->l, curr);
  size_t r = ___join(__bsnode_t, depth_rec)(n->r, curr);
  return l > r ? l + 1 : r + 1;
}

static inline size_t
___join(__bstree_t, depth)(__bstree_t *t) {
  ASSERT((t != NULL), "Trying to call depth on a NULL tree\n");
  return ___join(__bsnode_t, depth_rec)(t->root, 0);
}

static inline T
___join(__bstree_t, max)(__bstree_t *t) {
  ASSERT((t != NULL), "Trying to call max on a NULL tree\n");
  __bsnode_t *n = t->root;
  while (n->r)
    n = n->r;
  return n->data;
}

static inline T
___join(__bstree_t, min)(__bstree_t *t) {
  ASSERT((t != NULL), "Trying to call min on a NULL tree\n");
  __bsnode_t *n = t->root;
  while (n->l)
    n = n->l;
  return n->data;
}

static inline void
___join(__bsnode_t, io_visit)(__bsnode_t *n, void (*visitor)(T*)) {
  if (!n)
    return;
  ___join(__bsnode_t, io_visit)(n->l, visitor);
  visitor(&n->data);
  ___join(__bsnode_t, io_visit)(n->r, visitor);
}

static inline void
___join(__bstree_t, visit_in_order)(__bstree_t *t, void (*visitor)(T*)) {
  ASSERT((t != NULL), "Trying to call visit_in_order on a NULL tree\n");
  ___join(__bsnode_t, io_visit)(t->root, visitor);
}

static inline void
___join(__bsnode_t, pre_visit)(__bsnode_t *n, void (*visitor)(T*)) {
  if (!n)
    return;
  visitor(&n->data);
  ___join(__bsnode_t, pre_visit)(n->l, visitor);
  ___join(__bsnode_t, pre_visit)(n->r, visitor);
}

static inline void
___join(__bstree_t, visit_pre_order)(__bstree_t *t, void (*visitor)(T*)) {
  ASSERT((t != NULL), "Trying to call visit_pre_order on a NULL tree\n");
  ___join(__bsnode_t, pre_visit)(t->root, visitor);
}

static inline void
___join(__bsnode_t, post_visit)(__bsnode_t *n, void (*visitor)(T*)) {
  if (!n)
    return;
  ___join(__bsnode_t, post_visit)(n->l, visitor);
  ___join(__bsnode_t, post_visit)(n->r, visitor);
  visitor(&n->data);
}

static inline void
___join(__bstree_t, visit_post_order)(__bstree_t *t, void (*visitor)(T*)) {
  ASSERT((t != NULL), "Trying to call visit_post_order on a NULL tree\n");
  ___join(__bsnode_t, post_visit)(t->root, visitor);
}

static inline void
___join(__bsnode_t, destroy_rec)(__bsnode_t *n, void (*drop)(T*)) {
  if (n == NULL)
    return;
  ___join(__bsnode_t, destroy_rec)(n->l, drop);
  ___join(__bsnode_t, destroy_rec)(n->r, drop);
  if (drop)
    drop(&n->data);
  free(n);
}

static inline void
___join(__bstree_t, destroy)(__bstree_t *t) {
  ASSERT((t != NULL), "Trying to call destroy on a NULL tree\n");
  ___join(__bsnode_t, destroy_rec)(t->root, t->dropfn);
}

#undef T
#undef __bsnode_t
#undef __bstree_t
#undef ___bst_pfx
