#include "common.h"

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if !defined(T)
#define T int
#endif // T

#if !defined(_lc_bst_pfx)
#define _lc_bst_pfx T
#define __bstree_t _lc_join(_lc_bst_pfx, bstree)
#else
#define __bstree_t _lc_bst_pfx
#endif // _lc_bst_pfx

#define __bsnode_t _lc_join(_lc_bst_pfx, bsnode)

typedef struct __bsnode_t {
  struct __bsnode_t *l, *r;
  T data;
} __bsnode_t;

typedef struct __bstree_t {
  size_t size;
  __bsnode_t *root;
  int (*cmp)(T, T);
  void (*dropfn)(T);
} __bstree_t;

static inline void
_lc_join(__bstree_t, init)(__bstree_t *t, int (*cmp)(T, T), void (*drop)(T)) {
  ASSERT((t != NULL), "Trying to init a NULL binary search tree\n");
  t->size = 0;
  t->root = NULL;
  ASSERT((cmp != NULL),
         "Passing NULL for cmp parameter in binary search tree init\n");
  t->cmp = cmp;
  t->dropfn = drop;
}

static inline void
_lc_join(__bstree_t, insert)(__bstree_t *t, T data) {
  ASSERT((t != NULL), "Trying to insert into a NULL binary search tree\n");
  ASSERT((t->cmp != NULL),
         "Trying to insert into a binary search tree without cmp function");
  __bsnode_t **n = &t->root;
  while (*n) {
    int cmp = t->cmp((*n)->data, data);
    if (cmp < 0)
      n = &(*n)->r;
    else if (cmp > 0)
      n = &(*n)->l;
    else
      return;
  }
  *n = (__bsnode_t *)malloc(sizeof(__bsnode_t));
  ASSERT((*n != NULL), "Failed to allocate new node for insertion");
  (*n)->data = data;
  (*n)->l = NULL;
  (*n)->r = NULL;
  t->size++;
}

static inline T *
_lc_join(__bstree_t, find)(__bstree_t *t, T data) {
  ASSERT((t != NULL), "Trying to call find on a NULL tree\n");
  ASSERT((t->cmp != NULL),
         "Trying to call find on a tree with no cmp function\n");
  __bsnode_t *n = t->root;
  while (n) {
    if (t->cmp(n->data, data) == 0)
      return &n->data;
    if (t->cmp(n->data, data) > 0)
      n = n->l;
    else
      n = n->r;
  }
  return NULL;
};

static inline bool
_lc_join(__bstree_t, remove)(__bstree_t *t, T data) {
  ASSERT((t != NULL), "Trying to call 'remove' on a NULL tree\n");
  ASSERT((t->cmp != NULL),
         "Trying to call 'remove' on a tree with no cmp function\n");
  __bsnode_t **n = &t->root;
  __bsnode_t *parent = NULL;
  // Traverse to find the node to be removed
  while (*n != NULL) {
    int cmp = t->cmp((*n)->data, data);
    if (cmp < 0) {
      parent = *n;
      n = &(*n)->r;
    } else if (cmp > 0) {
      parent = *n;
      n = &(*n)->l;
    } else {
      break; // Node found
    }
  }
  if (*n == NULL)
    return false; // Node not found

  __bsnode_t *node_to_remove = *n;
  // Case 1: Node has no children (leaf node)
  if (node_to_remove->l == NULL && node_to_remove->r == NULL) {
    *n = NULL; // Remove the node
  }
  // Case 2: Node has one child
  else if (node_to_remove->l == NULL || node_to_remove->r == NULL) {
    __bsnode_t *child =
        node_to_remove->l ? node_to_remove->l : node_to_remove->r;
    *n = child; // Replace the node with its child
  }
  // Case 3: Node has two children
  else {
    // Find the in-order successor (minimum of the right subtree)
    __bsnode_t **successor = &node_to_remove->r;
    while ((*successor)->l != NULL) {
      successor = &(*successor)->l;
    }
    // Replace the node's data with the successor's data
    node_to_remove->data = (*successor)->data;
    // Remove the successor node
    __bsnode_t *successor_node = *successor;
    *successor = successor_node->r;
    node_to_remove = successor_node;
  }
  if (t->dropfn) {
    t->dropfn(node_to_remove->data);
  }
  free(node_to_remove);
  t->size--;
  return true;
}

static inline size_t
_lc_join(__bsnode_t, depth_rec)(__bsnode_t *n, size_t curr) {
  if (!n)
    return 0;
  size_t l = _lc_join(__bsnode_t, depth_rec)(n->l, curr);
  size_t r = _lc_join(__bsnode_t, depth_rec)(n->r, curr);
  return l > r ? l + 1 : r + 1;
}

static inline size_t
_lc_join(__bstree_t, depth)(__bstree_t *t) {
  ASSERT((t != NULL), "Trying to call depth on a NULL tree\n");
  return _lc_join(__bsnode_t, depth_rec)(t->root, 0);
}

static inline T
_lc_join(__bstree_t, max)(__bstree_t *t) {
  ASSERT((t != NULL), "Trying to call max on a NULL tree\n");
  __bsnode_t *n = t->root;
  while (n->r)
    n = n->r;
  return n->data;
}

static inline T
_lc_join(__bstree_t, min)(__bstree_t *t) {
  ASSERT((t != NULL), "Trying to call min on a NULL tree\n");
  __bsnode_t *n = t->root;
  while (n->l)
    n = n->l;
  return n->data;
}

static inline void
_lc_join(__bsnode_t, io_visit)(__bsnode_t *n, void (*visitor)(T *)) {
  if (!n)
    return;
  _lc_join(__bsnode_t, io_visit)(n->l, visitor);
  visitor(&n->data);
  _lc_join(__bsnode_t, io_visit)(n->r, visitor);
}

static inline void
_lc_join(__bstree_t, visit_in_order)(__bstree_t *t, void (*visitor)(T *)) {
  ASSERT((t != NULL), "Trying to call visit_in_order on a NULL tree\n");
  _lc_join(__bsnode_t, io_visit)(t->root, visitor);
}

static inline void
_lc_join(__bsnode_t, pre_visit)(__bsnode_t *n, void (*visitor)(T *)) {
  if (!n)
    return;
  visitor(&n->data);
  _lc_join(__bsnode_t, pre_visit)(n->l, visitor);
  _lc_join(__bsnode_t, pre_visit)(n->r, visitor);
}

static inline void
_lc_join(__bstree_t, visit_pre_order)(__bstree_t *t, void (*visitor)(T *)) {
  ASSERT((t != NULL), "Trying to call visit_pre_order on a NULL tree\n");
  _lc_join(__bsnode_t, pre_visit)(t->root, visitor);
}

static inline void
_lc_join(__bsnode_t, post_visit)(__bsnode_t *n, void (*visitor)(T *)) {
  if (!n)
    return;
  _lc_join(__bsnode_t, post_visit)(n->l, visitor);
  _lc_join(__bsnode_t, post_visit)(n->r, visitor);
  visitor(&n->data);
}

static inline void
_lc_join(__bstree_t, visit_post_order)(__bstree_t *t, void (*visitor)(T *)) {
  ASSERT((t != NULL), "Trying to call visit_post_order on a NULL tree\n");
  _lc_join(__bsnode_t, post_visit)(t->root, visitor);
}

static inline void
_lc_join(__bsnode_t, destroy_rec)(__bsnode_t *n, void (*drop)(T)) {
  if (n == NULL)
    return;
  _lc_join(__bsnode_t, destroy_rec)(n->l, drop);
  _lc_join(__bsnode_t, destroy_rec)(n->r, drop);
  if (drop)
    drop(n->data);
  free(n);
}

static inline void
_lc_join(__bstree_t, destroy)(__bstree_t *t) {
  ASSERT((t != NULL), "Trying to call destroy on a NULL tree\n");
  _lc_join(__bsnode_t, destroy_rec)(t->root, t->dropfn);
}

#undef T
#undef __bsnode_t
#undef __bstree_t
#undef _lc_bst_pfx

#if defined(__cplusplus)
}
#endif // __cplusplus
