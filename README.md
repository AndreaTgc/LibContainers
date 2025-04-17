# libcontainers

**libcontainers** is a modern C library that brings powerful, STL-inspired data structures and utilities to the C programming language — without sacrificing performance, safety, or clarity.

This library aims to bridge the gap between low-level C and the expressive, type-safe abstractions of C++'s Standard Template Library (STL), offering reusable, generic containers through macro-based instantiation.

---

## 🚀 Features

- **Type-safe generics** via macro instantiation

---

## 🧠 Design Philosophy

- ✅ *Type Safety:* All containers are instantiated for user-defined types (like `int`, `char*`, or custom structs).
- ✅ *Performance First:* No unnecessary abstraction overhead.
- ✅ *Minimal Dependencies:* Standard C11 only.

---

## 🔧 Usage

You instantiate containers per-type with macros:

```c
#define T char*
#define _lc_nodeset_pfx str
#include "containers/atomic_node_set.h"

str_node_set set;
str_node_set_init(&set, 128, hash_str, eq_str);
str_node_set_insert(&set, strdup("hello"));
```
