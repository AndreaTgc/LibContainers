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
- _Minimal Dependencies:_ Just requires the C's stdlib.
- _Value Ownership:_ The containers take full ownership of the inserted values and are responsible for cleaning it up via
user defined drop functions.

---

## Implemented containers

- Vector
- Doubly linked list
- Binary search tree
- Hash sets and Hash maps

---
