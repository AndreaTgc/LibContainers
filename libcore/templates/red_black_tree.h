#include "../libcore.h"
#include "_lc_templating.h"

#ifndef T
#define T int
#endif

#ifndef lcore_pfx
#define lcore_pfx _lcore_join(T, rb_tree)
#endif

#ifndef lcore_cmp_fn
#define lcore_cmp_fn(a, b) ((a) - (b))
#endif

#ifndef lcore_drop_fn
#define lcore_drop_fn(x)
#endif

#define Self lcore_pfx
#define _Node _lcore_join(Self, node)

// ========== Node and Tree Types ==========

typedef struct _Node {
  struct _Node *parent, *left, *right;
  u8 red; // 1 = Red, 0 = Black
  T data;
} _Node;

typedef struct Self {
  _Node *root;
  usize size;
} Self;

// ========== Public API (Visible to User) ==========
// These functions are meant to be used to interact with the tree
// structure, you should never access the struct memebers directly

// Initializes the empty tree
LCORE_API void _lcore_mfunc(init)(Self *self);
// Inserts a new element into the tree, returns false if the insertion fails,
LCORE_API bool _lcore_mfunc(insert)(Self *self, T value);
// Removes an element from the tree, returns false if the removal fails
LCORE_API bool _lcore_mfunc(remove)(Self *self, T value);
// Checks whether an element is part of the tree
LCORE_API bool _lcore_mfunc(contains)(Self *self, T value);
// Deallocates the tree-related memory (all nodes)
LCORE_API void _lcore_mfunc(destroy)(Self *self);

// ========== Internal Helpers (Private) ==========
// These functions are not meant to be used by the user, they're just
// helper function that make the Public API implementation easier to read and
// maintain

LCORE_API _Node *_lcore_mfunc(make_node)(T value);
LCORE_API void _lcore_mfunc(rotate_left)(Self *self, _Node *x);
LCORE_API void _lcore_mfunc(rotate_right)(Self *self, _Node *x);
LCORE_API void _lcore_mfunc(insert_fixup)(Self *self, _Node *z);
LCORE_API void _lcore_mfunc(delete_fixup)(Self *self, _Node *x);
LCORE_API void _lcore_mfunc(transplant)(Self *self, _Node *u, _Node *v);
LCORE_API _Node *_lcore_mfunc(minimum)(_Node *node);
LCORE_API void _lcore_mfunc(destroy_recursive)(_Node *node);

// ========== Public API Implementations ==========

LCORE_API void
_lcore_mfunc(init)(Self *self) {
  self->root = NULL;
  self->size = 0;
}

LCORE_API bool
_lcore_mfunc(insert)(Self *self, T value) {
  _Node *z = _lcore_mfunc(make_node)(value);
  if (!z)
    return false;

  _Node *y = NULL;
  _Node *x = self->root;

  while (x) {
    y = x;
    int cmp = lcore_cmp_fn(z->data, x->data);
    if (cmp < 0)
      x = x->left;
    else if (cmp > 0)
      x = x->right;
    else {
      lcore_drop_fn(z->data);
      free(z);
      return false;
    }
  }

  z->parent = y;
  if (!y)
    self->root = z;
  else if (lcore_cmp_fn(z->data, y->data) < 0)
    y->left = z;
  else
    y->right = z;

  _lcore_mfunc(insert_fixup)(self, z);
  self->size++;
  return true;
}

LCORE_API bool
_lcore_mfunc(remove)(Self *self, T value) {
  _Node *z = self->root;
  while (z) {
    int cmp = lcore_cmp_fn(value, z->data);
    if (cmp == 0)
      break;
    z = (cmp < 0) ? z->left : z->right;
  }

  if (!z)
    return false;

  _Node *y = z;
  _Node *x = NULL;
  u8 y_original_red = y->red;

  if (!z->left) {
    x = z->right;
    _lcore_mfunc(transplant)(self, z, z->right);
  } else if (!z->right) {
    x = z->left;
    _lcore_mfunc(transplant)(self, z, z->left);
  } else {
    y = _lcore_mfunc(minimum)(z->right);
    y_original_red = y->red;
    x = y->right;
    if (y->parent != z) {
      _lcore_mfunc(transplant)(self, y, y->right);
      y->right = z->right;
      if (y->right)
        y->right->parent = y;
    }
    _lcore_mfunc(transplant)(self, z, y);
    y->left = z->left;
    if (y->left)
      y->left->parent = y;
    y->red = z->red;
  }

  lcore_drop_fn(z->data);
  free(z);

  if (y_original_red == 0 && x)
    _lcore_mfunc(delete_fixup)(self, x);

  self->size--;
  return true;
}

LCORE_API bool
_lcore_mfunc(contains)(Self *self, T value) {
  _Node *cur = self->root;
  while (cur) {
    int cmp = lcore_cmp_fn(value, cur->data);
    if (cmp == 0)
      return true;
    cur = (cmp < 0) ? cur->left : cur->right;
  }
  return false;
}

LCORE_API void
_lcore_mfunc(destroy)(Self *self) {
  _lcore_mfunc(destroy_recursive)(self->root);
  self->root = NULL;
  self->size = 0;
}

// ========== Internal Helper Implementations ==========

LCORE_API _Node *
_lcore_mfunc(make_node)(T value) {
  _Node *node = lcore_malloc(_Node, sizeof(_Node));
  if (!node)
    return NULL;
  node->parent = node->left = node->right = NULL;
  node->red = 1; // New node is red
  node->data = value;
  return node;
}

LCORE_API void
_lcore_mfunc(rotate_left)(Self *self, _Node *x) {
  _Node *y = x->right;
  x->right = y->left;
  if (y->left)
    y->left->parent = x;
  y->parent = x->parent;
  if (!x->parent)
    self->root = y;
  else if (x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;
  y->left = x;
  x->parent = y;
}

LCORE_API void
_lcore_mfunc(rotate_right)(Self *self, _Node *x) {
  _Node *y = x->left;
  x->left = y->right;
  if (y->right)
    y->right->parent = x;
  y->parent = x->parent;
  if (!x->parent)
    self->root = y;
  else if (x == x->parent->right)
    x->parent->right = y;
  else
    x->parent->left = y;
  y->right = x;
  x->parent = y;
}

LCORE_API void
_lcore_mfunc(insert_fixup)(Self *self, _Node *z) {
  while (z->parent && z->parent->red) {
    if (z->parent == z->parent->parent->left) {
      _Node *y = z->parent->parent->right;
      if (y && y->red) {
        z->parent->red = 0;
        y->red = 0;
        z->parent->parent->red = 1;
        z = z->parent->parent;
      } else {
        if (z == z->parent->right) {
          z = z->parent;
          _lcore_mfunc(rotate_left)(self, z);
        }
        z->parent->red = 0;
        z->parent->parent->red = 1;
        _lcore_mfunc(rotate_right)(self, z->parent->parent);
      }
    } else {
      _Node *y = z->parent->parent->left;
      if (y && y->red) {
        z->parent->red = 0;
        y->red = 0;
        z->parent->parent->red = 1;
        z = z->parent->parent;
      } else {
        if (z == z->parent->left) {
          z = z->parent;
          _lcore_mfunc(rotate_right)(self, z);
        }
        z->parent->red = 0;
        z->parent->parent->red = 1;
        _lcore_mfunc(rotate_left)(self, z->parent->parent);
      }
    }
  }
  self->root->red = 0;
}

LCORE_API void
_lcore_mfunc(delete_fixup)(Self *self, _Node *x) {
  while (x != self->root && (!x || !x->red)) {
    if (x == x->parent->left) {
      _Node *w = x->parent->right;
      if (w && w->red) {
        w->red = 0;
        x->parent->red = 1;
        _lcore_mfunc(rotate_left)(self, x->parent);
        w = x->parent->right;
      }
      if ((!w->left || !w->left->red) && (!w->right || !w->right->red)) {
        if (w)
          w->red = 1;
        x = x->parent;
      } else {
        if (!w->right || !w->right->red) {
          if (w->left)
            w->left->red = 0;
          w->red = 1;
          _lcore_mfunc(rotate_right)(self, w);
          w = x->parent->right;
        }
        if (w)
          w->red = x->parent->red;
        x->parent->red = 0;
        if (w && w->right)
          w->right->red = 0;
        _lcore_mfunc(rotate_left)(self, x->parent);
        x = self->root;
      }
    } else {
      _Node *w = x->parent->left;
      if (w && w->red) {
        w->red = 0;
        x->parent->red = 1;
        _lcore_mfunc(rotate_right)(self, x->parent);
        w = x->parent->left;
      }
      if ((!w->right || !w->right->red) && (!w->left || !w->left->red)) {
        if (w)
          w->red = 1;
        x = x->parent;
      } else {
        if (!w->left || !w->left->red) {
          if (w->right)
            w->right->red = 0;
          w->red = 1;
          _lcore_mfunc(rotate_left)(self, w);
          w = x->parent->left;
        }
        if (w)
          w->red = x->parent->red;
        x->parent->red = 0;
        if (w && w->left)
          w->left->red = 0;
        _lcore_mfunc(rotate_right)(self, x->parent);
        x = self->root;
      }
    }
  }
  if (x)
    x->red = 0;
}

LCORE_API void
_lcore_mfunc(transplant)(Self *self, _Node *u, _Node *v) {
  if (!u->parent)
    self->root = v;
  else if (u == u->parent->left)
    u->parent->left = v;
  else
    u->parent->right = v;
  if (v)
    v->parent = u->parent;
}

LCORE_API _Node *
_lcore_mfunc(minimum)(_Node *node) {
  while (node->left)
    node = node->left;
  return node;
}

LCORE_API void
_lcore_mfunc(destroy_recursive)(_Node *node) {
  if (!node)
    return;
  _lcore_mfunc(destroy_recursive)(node->left);
  _lcore_mfunc(destroy_recursive)(node->right);
  lcore_drop_fn(node->data);
  free(node);
}
