# Cache Simulator Project

## Overview
This project implements a cache simulator in C. It allows users to simulate reading and writing bytes in memory with a configurable cache. The simulator handles:

- Cache hits
- Cold misses
- Conflict misses
- Frequency tracking for cache replacement
- Printing the cache state for debugging

---

## Features
- Initialize cache with configurable parameters:
  - `s` — number of set index bits
  - `t` — number of tag bits
  - `b` — number of block offset bits
  - `E` — number of lines per set (associativity)
- Simulate memory read/write with cache management
- Track cache hits, cold misses, and conflict misses
- Print cache contents for debugging

---

## Structures

```c
typedef unsigned char uchar;

typedef struct cache_line_s {
    uchar valid;
    uchar frequency;
    long int tag;
    uchar* block;
} cache_line_t;

typedef struct cache_s {
    uchar s;
    uchar t;
    uchar b;
    uchar E;
    cache_line_t** cache;
} cache_t;

typedef struct address_s {
    long offset;
    long set_index;
    long tag;
} address_t;
```

---

## Example
Size of data: 16
Input data >> 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
s t b E: 2 2 2 2
0
1
2
-1
