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
#define Self __bstree_t

typedef struct __bsnode_t {
  struct __bsnode_t *l, *r;
  T data;
} __bsnode_t;

typedef struct Self {
  size_t size;
  __bsnode_t *root;
  int (*cmp)(T, T);
  void (*dropfn)(T);
} Self;

/**
 * @brief initializes a binary search tree
 *
 * @param self pointer to the bst
 * @param cmp  function used to compare elements of type T
 * @param drop function used to deallocate elements (optional)
 */
static inline void
_lc_membfunc(init)(__bstree_t *self, int (*cmp)(T, T),
                           void (*drop)(T)) {
  ASSERT((self != NULL), "Trying to init a NULL binary search tree\n");
  self->size = 0;
  self->root = NULL;
  ASSERT((cmp != NULL),
         "Passing NULL for cmp parameter in binary search tree init\n");
  self->cmp = cmp;
  self->dropfn = drop;
}

/**
 * @brief inserts a new element into the tree
 *
 * @param self pointer to the bst
 * @param data element to insert
 */
static inline void
_lc_membfunc(insert)(__bstree_t *self, T data) {
  ASSERT((self != NULL), "Trying to insert into a NULL binary search tree\n");
  ASSERT((self->cmp != NULL),
         "Trying to insert into a binary search tree without cmp function");
  __bsnode_t **n = &self->root;
  while (*n) {
    int cmp = self->cmp((*n)->data, data);
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
  self->size++;
}

/**
 * @brief tries to find an element into the tree and returns the pointer to it.
 * returns NULL if the element is not found
 *
 * @param self pointer to the tree
 * @param data element to find
 */
static inline T *
_lc_membfunc(find)(__bstree_t *self, T data) {
  ASSERT((self != NULL), "Trying to call find on a NULL tree\n");
  ASSERT((self->cmp != NULL),
         "Trying to call find on a tree with no cmp function\n");
  __bsnode_t *n = self->root;
  while (n) {
    if (self->cmp(n->data, data) == 0)
      return &n->data;
    if (self->cmp(n->data, data) > 0)
      n = n->l;
    else
      n = n->r;
  }
  return NULL;
};

/**
 * @brief removes an element from the tree
 *
 * @param self pointer to the tree
 * @param data element to remove
 */
static inline bool
_lc_membfunc(remove)(__bstree_t *self, T data) {
  ASSERT((self != NULL), "Trying to call 'remove' on a NULL tree\n");
  ASSERT((self->cmp != NULL),
         "Trying to call 'remove' on a tree with no cmp function\n");
  __bsnode_t **n = &self->root;
  __bsnode_t *parent = NULL;
  while (*n != NULL) {
    int cmp = self->cmp((*n)->data, data);
    if (cmp < 0) {
      parent = *n;
      n = &(*n)->r;
    } else if (cmp > 0) {
      parent = *n;
      n = &(*n)->l;
    } else {
      break;
    }
  }
  if (*n == NULL)
    return false;

  __bsnode_t *node_to_remove = *n;
  if (node_to_remove->l == NULL && node_to_remove->r == NULL) {
    *n = NULL;
  } else if (node_to_remove->l == NULL || node_to_remove->r == NULL) {
    __bsnode_t *child =
        node_to_remove->l ? node_to_remove->l : node_to_remove->r;
    *n = child;
  } else {
    __bsnode_t **successor = &node_to_remove->r;
    while ((*successor)->l != NULL) {
      successor = &(*successor)->l;
    }
    node_to_remove->data = (*successor)->data;
    __bsnode_t *successor_node = *successor;
    *successor = successor_node->r;
    node_to_remove = successor_node;
  }
  if (self->dropfn) {
    self->dropfn(node_to_remove->data);
  }
  free(node_to_remove);
  self->size--;
  return true;
}

/**
 * @brief helper function used to calculate the depth of a tree
 *
 * @param n current node
 * @param curr current depth value
 */
static inline size_t
_lc_join(__bsnode_t, depth_rec)(__bsnode_t *n, size_t curr) {
  if (!n)
    return 0;
  size_t l = _lc_join(__bsnode_t, depth_rec)(n->l, curr);
  size_t r = _lc_join(__bsnode_t, depth_rec)(n->r, curr);
  return l > r ? l + 1 : r + 1;
}

/**
 * @brief calculates the depth of a tree
 *
 * @param self pointer to the tree
 */
static inline size_t
_lc_membfunc(depth)(__bstree_t *self) {
  ASSERT((self != NULL), "Trying to call depth on a NULL tree\n");
  return _lc_join(__bsnode_t, depth_rec)(self->root, 0);
}

/**
 * @brief returns the max value inside a tree (right most node)
 *
 * @param self pointer to the tree
 */
static inline T
_lc_membfunc(max)(__bstree_t *self) {
  ASSERT((self != NULL), "Trying to call max on a NULL tree\n");
  __bsnode_t *n = self->root;
  while (n->r)
    n = n->r;
  return n->data;
}

/**
 * @brief returns the min value inside a tree (left most node)
 *
 * @param self pointer to the tree
 */
static inline T
_lc_membfunc(min)(__bstree_t *self) {
  ASSERT((self != NULL), "Trying to call min on a NULL tree\n");
  __bsnode_t *n = self->root;
  while (n->l)
    n = n->l;
  return n->data;
}

/**
 * @brief helper function used for in-order visit
 *
 * @param n       current node
 * @param visitor function applied to the node
 */
static inline void
_lc_join(__bsnode_t, io_visit)(__bsnode_t *n, void (*visitor)(T *)) {
  if (!n)
    return;
  _lc_join(__bsnode_t, io_visit)(n->l, visitor);
  visitor(&n->data);
  _lc_join(__bsnode_t, io_visit)(n->r, visitor);
}

/**
 * @brief visits the tree in-order, calling the 'visitor' function on each node
 *
 * @param self    pointer to the tree
 * @param visitor function applied to each node
 */
static inline void
_lc_membfunc(visit_in_order)(__bstree_t *self, void (*visitor)(T *)) {
  ASSERT((self != NULL), "Trying to call visit_in_order on a NULL tree\n");
  _lc_join(__bsnode_t, io_visit)(self->root, visitor);
}

/**
 * @brief helper function used for pre-order visit
 *
 * @param n       current node
 * @param visitor function applied to the node
 */
static inline void
_lc_join(__bsnode_t, pre_visit)(__bsnode_t *n, void (*visitor)(T *)) {
  if (!n)
    return;
  visitor(&n->data);
  _lc_join(__bsnode_t, pre_visit)(n->l, visitor);
  _lc_join(__bsnode_t, pre_visit)(n->r, visitor);
}

/**
 * @brief visits the tree using pre-order, calling the 'visitor' function on
 * each node
 *
 * @param self    pointer to the tree
 * @param visitor function applied to each node
 */
static inline void
_lc_membfunc(visit_pre_order)(__bstree_t *self, void (*visitor)(T *)) {
  ASSERT((self != NULL), "Trying to call visit_pre_order on a NULL tree\n");
  _lc_join(__bsnode_t, pre_visit)(self->root, visitor);
}

/**
 * @brief helper function used for post-order visit
 *
 * @param n       current node
 * @param visitor function applied to the node
 */
static inline void
_lc_join(__bsnode_t, post_visit)(__bsnode_t *n, void (*visitor)(T *)) {
  if (!n)
    return;
  _lc_join(__bsnode_t, post_visit)(n->l, visitor);
  _lc_join(__bsnode_t, post_visit)(n->r, visitor);
  visitor(&n->data);
}

/**
 * @brief visits the tree using post-order, calling the 'visitor' function on
 * each node
 *
 * @param self    pointer to the tree
 * @param visitor function applied to each node
 */
static inline void
_lc_membfunc(visit_post_order)(__bstree_t *self, void (*visitor)(T *)) {
  ASSERT((self != NULL), "Trying to call visit_post_order on a NULL tree\n");
  _lc_join(__bsnode_t, post_visit)(self->root, visitor);
}

/**
 * @brief helper function used to destroy the tree nodes
 *
 * @param n    current node
 * @param drop function used to deallocate elements of type T
 */
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

/**
 * @brief deallocates the memory associated with a tree, resetting it to
 * a "base" state
 *
 * @param self pointer to the tree
 */
static inline void
_lc_membfunc(destroy)(__bstree_t *self) {
  ASSERT((self != NULL), "Trying to call destroy on a NULL tree\n");
  _lc_join(__bsnode_t, destroy_rec)(self->root, self->dropfn);
}

#undef T
#undef Self
#undef __bsnode_t
#undef __bstree_t
#undef _lc_bst_pfx

#if defined(__cplusplus)
}
#endif // __cplusplus
