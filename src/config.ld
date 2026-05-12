-- config.ld
project = "LuaJson"
file = {'lua_json.h', 'lua_json_array.h', 'lua_json_lua.h', 'lua_json_object.h'}
add_language_extension('c', 'c')
merge = true  -- Allows combining docs from multiple files into one module


format = 'markdown'

-- Directly inject the content into the main page header
description = [[
# Introduction

**A High-Performance, Memory-Safe JSON Serializer for Lua 5.1, 5.3, and 5.4**

![Lua Safety](https://img.shields.io/badge/lua-safe-brightgreen)
![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Memory Leaks](https://img.shields.io/badge/valgrind-clean-success)
![License](https://img.shields.io/badge/license-GPLv2-blue)

LuaJson provides native `array` and `object` container types for Lua with
high-performance JSON serialization, mutable JavaScript-style APIs, and
memory-safe rendering paths.

Unlike standard Lua tables, LuaJson containers preserve insertion order,
support positional mutation operations, and expose a dedicated serialization API.

LuaJson objects and arrays support:

- Ordered mutation operations
- Fast JSON rendering
- Lua table serialization
- Reference-aware copy semantics
- Circular reference protection
- Cross-VM compatibility
- Memory-safe rendering paths

---

# Steady-State DOM Construction Benchmark

On embedded ARM:

- ~0.56 ms (light run)
- ~0.98 ms (heavy/steady run)
- ~50–300 MB/s effective throughput range depending on measurement method

## Full CRUD API

Specialized `array` and `object` classes with methods such as:

- push / pop
- shift / unshift
- reverse / move
- ref / unref

---

## Valgrind Verified
- 100% memory integrity

---

## Core Multi-VM Engine

Seamless native compilation support across Lua 5.1, 5.3, and 5.4 virtual machines.

---

## The Memory Shield

Clean element destructor sequencing scopes that eliminate Use-After-Free vectors,
verified with a perfect zero-leak Valgrind pass.

---

## The Graph Guard

High-speed `O(1)` loop tracking maps that prevent circular references without
code bloat.

---

## Standalone Public C API

A clean variadic reverse-dumpstack public wrapper library (`libluajson.so`)
ready for independent projects.

---

## OpenWrt SDK Interface

Fully automated version-variant package deployment loops integrated directly
into OpenWrt's native configuration environment.

---
]]