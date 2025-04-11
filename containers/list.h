#ifndef CTL_LIST_H_
#define CTL_LIST_H_

#define CTL_DEF_LIST_DEFAULT(T) CTL_DEF_LIST(T, T)

#define CTL_DEF_LIST(T, PREFIX)                                                \
  typedef struct PREFIX##_node {                                               \
    T data;                                                                    \
    struct PREFIX##_node *next;                                                \
    struct PREFIX##_node *prev;                                                \
  } PREFIX##_list_node;                                                        \
                                                                               \
  typedef struct _PREFIX##_list {                                              \
    PREFIX##_list_node *head;                                                  \
    PREFIX##_list_node *tail;                                                  \
  } PREFIX##_list_t;                                                           \
                                                                               \
  void PREFIX##_list_init(PREFIX##_list_t *l) {                                \
    l->head = NULL;                                                            \
    l->tail = NULL;                                                            \
  }                                                                            \
                                                                               \
  void PREFIX##_list_add_tail(PREFIX##_list_t *l, T data) {                    \
    if (!l->tail) {                                                            \
      l->tail = malloc(sizeof(PREFIX##_list_node));                            \
      l->tail->data = data;                                                    \
      l->tail->prev = NULL;                                                    \
      l->tail->next = NULL;                                                    \
      l->head = l->tail;                                                       \
      return;                                                                  \
    }                                                                          \
    l->tail->next = malloc(sizeof(PREFIX##_list_node));                        \
    l->tail->next->next = NULL;                                                \
    l->tail->next->prev = l->tail;                                             \
    l->tail->next->data = data;                                                \
    l->tail = l->tail->next;                                                   \
  }                                                                            \
                                                                               \
  void PREFIX##_list_add_head(PREFIX##_list_t *l, T data) {                    \
    if (!l->head) {                                                            \
      l->head = malloc(sizeof(PREFIX##_list_node));                            \
      l->head->data = data;                                                    \
      l->head->prev = NULL;                                                    \
      l->head->next = NULL;                                                    \
      l->tail = l->head;                                                       \
      return;                                                                  \
    }                                                                          \
    l->head->prev = malloc(sizeof(PREFIX##_list_node));                        \
    l->head->prev->next = l->head;                                             \
    l->head->prev->prev = NULL;                                                \
    l->head->prev->data = data;                                                \
    l->head = l->head->prev;                                                   \
  }                                                                            \
                                                                               \
  void PREFIX##_list_pop_tail(PREFIX##_list_t *l) {                            \
    if (!l->tail)                                                              \
      return;                                                                  \
    PREFIX##_list_node *to_free = l->tail;                                     \
    l->tail = l->tail->prev;                                                   \
    free(to_free);                                                             \
  }                                                                            \
                                                                               \
  void PREFIX##_list_pop_head(PREFIX##_list_t *l) {                            \
    if (!l->head)                                                              \
      return;                                                                  \
    PREFIX##_list_node *to_free = l->head;                                     \
    l->head = l->head->next;                                                   \
    free(to_free);                                                             \
  }                                                                            \
                                                                               \
  void PREFIX##_list_destroy(PREFIX##_list_t *l) {                             \
    PREFIX##_list_node *n = l->head;                                           \
    while (n) {                                                                \
      PREFIX##_list_node *next = n->next;                                      \
      free(n);                                                                 \
      n = next;                                                                \
    }                                                                          \
    l->head = NULL;                                                            \
    l->tail = NULL;                                                            \
  }

#endif // CTL_LIST_H_
