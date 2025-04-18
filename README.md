# libcontainers

**libcontainers** is a modern C library that brings powerful, STL-inspired data structures and utilities to the C programming language — without sacrificing performance, safety, or clarity.

This library aims to bridge the gap between low-level C and the expressive, type-safe abstractions of C++'s Standard Template Library (STL), offering reusable, generic containers through macro-based instantiation.

---

## Features

- **Type-safe generics** via macro instantiation

---

## Design Philosophy

- _Type Safety:_ All containers are instantiated for user-defined types (like `int`, `char*`, or custom structs).
- _Performance First:_ No unnecessary abstraction overhead.
- _Minimal Dependencies:_ Standard C11 only.

---

## Usage

You instantiate containers per-type with macros:

```c
#define T char *
#define _lc_nodeset_pfx str
#define _lc_profile_enabled
#include "containers/node_set.h"
#include <string.h>

uint64_t hash(char **s) {
    return (uint64_t) strlen(*s);
}

bool eq(char **s1, char **s2) {
  return strcmp(*s1, *s2) == 0;
}

char* clone(char **s) {
  return strdup(*s);
}

int main(void) {
  str_node_set s = {0};
  str_node_set_init(&s, 32, hash, NULL, eq, clone);
  str_node_set_insert(&s, &(char *){"Hello"});
  if(str_node_set_contains(&s, &(char*){"Hello"})){
      printf("Everything went well!\n");
  } else {
      printf("What happened here?\n");
  }
  str_node_set_print_profiling_data(&s, "TestSet");
  return 0;
}
```
