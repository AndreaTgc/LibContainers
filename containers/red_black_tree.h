#include "_lc_templating.h"

#ifndef T
#define T int
#endif // T

#ifndef lcore_pfx
#define lcore_pfx _lcore_join(T, rbtree)
#endif // lcore_pfx

#ifndef lcore_cmp_fn
#define lcore_cmp_fn(a, b) ((a) - (b))
#endif // lcore_cmp_fn

#ifndef lcore_drop_fn
#define lcore_drop_fn(x)
#endif // lcore_drop_fn

#define Self lcore_pfx
#define _Node _lcore_join(Self, node)

// ========== STRUCTS DEFINITIONS ============== //

typedef struct _Node {
  struct _Node *left, *right; // left and right children
  struct _Node *parent;       // parent node
  uint8_t red;                // node color (either red (1) or black (0))
  T data;                     // node payload
} _Node;

typedef struct Self {
  _Node *root; // root of the tree
  size_t size; // number of nodes
} Self;

// ============== PUBLIC API ==================== //
// These functions are meant to be used to interact with the tree structure
// you are NOT supposed to modify the struct memebers directly.

static inline void _lcore_mfunc(init)(Self *self);
static inline void _lcore_mfunc(destroy)(Self *self);
static inline uint8_t _lcore_mfunc(insert)(Self *self, T val);
static inline uint8_t _lcore_mfunc(remove)(Self *self, T val);
static inline uint8_t _lcore_mfunc(contains)(Self *self, T val);

// ============= PRIVATE FUNCTIONS ============== //
// These functions are not meant to be called directly, they are helpers used
// inside the public API implementation

static inline _Node *_lcore_mfunc_priv(max_node)(_Node *x);
static inline _Node *_lcore_mfunc_priv(min_node)(_Node *x);
static inline _Node *_lcore_mfunc_priv(new_node)(T value);
static inline void _lcore_mfunc_priv(transplant)(Self *self, _Node *x,
                                                 _Node *y);
static inline void _lcore_mfunc_priv(rotate_left)(Self *self, _Node *x);
static inline void _lcore_mfunc_priv(rotate_right)(Self *self, _Node *x);
static inline void _lcore_mfunc_priv(fix_insert)(Self *self, _Node *x);
static inline void _lcore_mfunc_priv(fix_delete)(Self *self, _Node *x,
                                                 _Node *x_p);

// ========== PUBLIC API IMPLEMENTATION ========= //

static inline void _lcore_mfunc(init)(Self *self) {
  memset(self, 0, sizeof(*self));
}

static inline void _lcore_mfunc(destroy)(Self *self) {
}

static inline uint8_t _lcore_mfunc(insert)(Self *self, T val) {
  _Node *cur = self->root;
  _Node *parent = NULL;

  // Traverse to find the correct insertion point
  while (cur) {
    parent = cur;
    int cmp = lcore_cmp_fn(val, cur->data);
    if (cmp == 0)
      return 0; // Value already exists
    else if (cmp < 0)
      cur = cur->left;
    else
      cur = cur->right;
  }
  _Node *n = _lcore_mfunc_priv(new_node)(val);
  if (!n)
    return 0; // Allocation failed
  n->parent = parent;
  if (!parent) {
    self->root = n; // Tree was empty
  } else if (lcore_cmp_fn(val, parent->data) < 0) {
    parent->left = n;
  } else {
    parent->right = n;
  }

  // Fix any red-black tree violations
  _lcore_mfunc_priv(fix_insert)(self, n);
  self->size += 1;
  return 1; // Successfully inserted
}

static inline uint8_t _lcore_mfunc(remove)(Self *self, T val) {
  _Node *z = self->root;
  while (z) {
    int c = lcore_cmp_fn(val, z->data);
    if (c == 0)
      break;
    else if (c < 0)
      z = z->left;
    else
      z = z->right;
  }

  if (!z)
    return 0; // Value not found

  _Node *y = z;
  _Node *x = NULL;
  _Node *x_parent = NULL;
  uint8_t y_original_red = y->red;

  if (!z->left) {
    x = z->right;
    x_parent = z->parent;
    _lcore_mfunc_priv(transplant)(self, z, z->right);
  } else if (!z->right) {
    x = z->left;
    x_parent = z->parent;
    _lcore_mfunc_priv(transplant)(self, z, z->left);
  } else {
    y = _lcore_mfunc_priv(min_node)(z->right);
    y_original_red = y->red;
    x = y->right;

    if (y->parent == z) {
      if (x)
        x->parent = y;
      x_parent = y;
    } else {
      _lcore_mfunc_priv(transplant)(self, y, y->right);
      y->right = z->right;
      if (y->right)
        y->right->parent = y;
      x_parent = y->parent;
    }

    _lcore_mfunc_priv(transplant)(self, z, y);
    y->left = z->left;
    if (y->left)
      y->left->parent = y;
    y->red = z->red;
  }

  lcore_drop_fn(z->data);
  free(z);
  self->size--;

  if (y_original_red == 0)
    _lcore_mfunc_priv(fix_delete)(self, x, x_parent);

  return 1;
}

static inline uint8_t _lcore_mfunc(contains)(Self *self, T val) {
  _Node *cur = self->root;
  while (cur) {
    int c = lcore_cmp_fn(cur->data, val);
    if (c == 0)
      return 1;
    cur = c > 0 ? cur->left : cur->right;
  }
  return 0;
}

// ========= PRIVATE API IMPLEMENTATION ========= //

static inline _Node *_lcore_mfunc_priv(max_node)(_Node *x) {
  _Node *tmp = x;
  while (tmp->right)
    tmp = tmp->right;
  return tmp;
}

static inline _Node *_lcore_mfunc_priv(min_node)(_Node *x) {
  _Node *tmp = x;
  while (tmp->left)
    tmp = tmp->left;
  return tmp;
}

static inline _Node *_lcore_mfunc_priv(new_node)(T value) {
  _Node *n = lc_malloc(_Node, sizeof(_Node));
  if (!n)
    return NULL;
  n->left = n->right = n->parent = NULL;
  n->red = 1;
  n->data = value;
  return n;
}

static inline void _lcore_mfunc_priv(transplant)(Self *self, _Node *x,
                                                 _Node *y) {
  if (!x->parent)
    self->root = y;
  else if (x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;

  if (y)
    y->parent = x->parent;
}

static inline void _lcore_mfunc_priv(rotate_left)(Self *self, _Node *x) {
  _Node *r = x->right;
  x->right = r->left;
  if (x->right)
    x->right->parent = x;
  r->parent = x->parent;
  if (!x->parent)
    self->root = r;
  else if (x == x->parent->left)
    x->parent->left = r;
  else
    x->parent->right = r;
  r->left = x;
  x->parent = r;
}

static inline void _lcore_mfunc_priv(rotate_right)(Self *self, _Node *x) {
  _Node *l = x->left;
  x->left = l->right;
  if (x->left)
    x->left->parent = x;
  l->parent = x->parent;
  if (!x->parent)
    self->root = l;
  else if (x == x->parent->right)
    x->parent->right = l;
  else
    x->parent->left = l;
  l->right = x;
  x->parent = l;
}

static inline void _lcore_mfunc_priv(fix_insert)(Self *self, _Node *x) {
  while (x != self->root && x->parent && x->parent->red) {
    _Node *parent = x->parent;
    _Node *grandparent = parent->parent;

    if (!grandparent)
      break; // Needed for safety

    if (parent == grandparent->left) {
      _Node *uncle = grandparent->right;

      // Case 1: Uncle is red → recolor
      if (uncle && uncle->red) {
        parent->red = 0;
        uncle->red = 0;
        grandparent->red = 1;
        x = grandparent;
      } else {
        // Case 2: x is right child → left rotate
        if (x == parent->right) {
          x = parent;
          _lcore_mfunc_priv(rotate_left)(self, x);
          parent = x->parent;
          grandparent = parent->parent;
        }

        // Case 3: x is left child → right rotate
        parent->red = 0;
        grandparent->red = 1;
        _lcore_mfunc_priv(rotate_right)(self, grandparent);
      }
    } else {
      _Node *uncle = grandparent->left;

      // Case 1: Uncle is red → recolor
      if (uncle && uncle->red) {
        parent->red = 0;
        uncle->red = 0;
        grandparent->red = 1;
        x = grandparent;
      } else {
        // Case 2: x is left child → right rotate
        if (x == parent->left) {
          x = parent;
          _lcore_mfunc_priv(rotate_right)(self, x);
          parent = x->parent;
          grandparent = parent->parent;
        }

        // Case 3: x is right child → left rotate
        parent->red = 0;
        grandparent->red = 1;
        _lcore_mfunc_priv(rotate_left)(self, grandparent);
      }
    }
  }
  self->root->red = 0;
}

static inline void _lcore_mfunc_priv(fix_delete)(Self *self, _Node *x,
                                                 _Node *x_p) {
  while ((x != self->root) && (!x || x->red == 0)) {
    if (x == (x_p ? x_p->left : NULL)) {
      _Node *w = x_p->right;
      if (w && w->red) {
        w->red = 0;
        x_p->red = 1;
        _lcore_mfunc_priv(rotate_left)(self, x_p);
        w = x_p->right;
      }

      if ((!w->left || !w->left->red) && (!w->right || !w->right->red)) {
        w->red = 1;
        x = x_p;
        x_p = x->parent;
      } else {
        if (!w->right || !w->right->red) {
          if (w->left)
            w->left->red = 0;
          w->red = 1;
          _lcore_mfunc_priv(rotate_right)(self, w);
          w = x_p->right;
        }

        w->red = x_p->red;
        x_p->red = 0;
        if (w->right)
          w->right->red = 0;
        _lcore_mfunc_priv(rotate_left)(self, x_p);
        x = self->root;
        break;
      }
    } else {
      _Node *w = x_p->left;
      if (w && w->red) {
        w->red = 0;
        x_p->red = 1;
        _lcore_mfunc_priv(rotate_right)(self, x_p);
        w = x_p->left;
      }

      if ((!w->left || !w->left->red) && (!w->right || !w->right->red)) {
        w->red = 1;
        x = x_p;
        x_p = x->parent;
      } else {
        if (!w->left || !w->left->red) {
          if (w->right)
            w->right->red = 0;
          w->red = 1;
          _lcore_mfunc_priv(rotate_left)(self, w);
          w = x_p->left;
        }

        w->red = x_p->red;
        x_p->red = 0;
        if (w->left)
          w->left->red = 0;
        _lcore_mfunc_priv(rotate_right)(self, x_p);
        x = self->root;
        break;
      }
    }
  }

  if (x)
    x->red = 0;
}
