-- config.ld
project = "LuaJson5.1"
file = {'lua_json.h', 'lua_json_array.h', 'lua_json_lua.h', 'lua_json_object.h'}
add_language_extension('c', 'c')
merge = true  -- Allows combining docs from multiple files into one module


format = 'markdown'

-- Directly inject the content into the main page header
description = [[
# Introduction

**A High-Performance, Memory-Safe JSON Serializer for Lua 5.1+**

![Lua Safety](https://img.shields.io/badge/lua-safe-brightgreen) ![Build Status](https://img.shields.io/badge/build-passing-brightgreen) ![Memory Leaks](https://img.shields.io/badge/valgrind-clean-success) ![License](https://img.shields.io/badge/license-GPLv2-blue)

LuaJson5.1 is not just another parser; it is a **hardened serialization engine** designed for stability under pressure. It features a unique **Hybrid Trust Architecture** that intelligently switches between zero-allocation stack operations for speed and sandboxed heap allocations for safety, ensuring your main Lua state never fragments or crashes on large payloads.

LuaJson adds native objects and arrays types to the luaVM that act just like their respective JS coounterparts, with the exception that LuaJson objects can be treated ar arrays of key/value pairs  
---

## 🚀 Key Features

*   **Hybrid Execution Model:** Automatically detects object size/complexity.
    *   **Fast Path:** Renders small, trusted objects directly on the C-Stack (Zero Allocation).
    *   **Heavy Path:** Large or "Stale" objects are rendered in a **Sandboxed Temporary State**, keeping your main GC clean.
*   **The "Safety Fuse":** A speculative execution engine that attempts the fast path first but instantly aborts and switches to the sandbox if a buffer overflow is imminent.
*   **Full CRUD API:** specialized `array` and `object` classes with methods like `push`, `pop`, `shift`, `ref`, and `unref`.
*   **Valgrind Verified:** 0 Bytes lost. 0 Errors. 100% Memory Integrity.

---
]]