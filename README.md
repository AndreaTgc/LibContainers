# libcontainers

**libcontainers** is a modern C library that brings powerful, STL-inspired data structures and utilities to the C programming language — without sacrificing performance, safety, or clarity.

This library aims to bridge the gap between low-level C and the expressive, type-safe abstractions of C++'s Standard Template Library (STL), offering reusable, generic containers through macro-based instantiation.

---

## 🚀 Features

- **Type-safe generics** via macro instantiation

---

## 🧠 Design Philosophy

- _Type Safety:_ All containers are instantiated for user-defined types (like `int`, `char*`, or custom structs).
- _Performance First:_ No unnecessary abstraction overhead.
- _Minimal Dependencies:_ Standard C11 only.

---

## 🔧 Usage

You instantiate containers per-type with macros:

```c
#define T char*
#define _lc_nodeset_pfx str
// _lc_nodeset_pfx can be left out if you
// want to use your type as prefix (does not work with pointers)
#include "containers/node_set.h"

// hash_str and eq_str are user defined functions

int main(void){
  str_node_set set = {0};
  str_node_set_init(&set, 128, hash_str, eq_str);
  str_node_set_insert(&set, strdup("hello"));
  if(str_node_set_contains(&set, "hello") == false) {
    printf("Whoops! something went wrong\n");
    return 1;
  } else {
    printf("Everything worked as expected\n");
    return 0;
  }
}
```
