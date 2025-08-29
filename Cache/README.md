Cache Simulator
Overview
This project implements a direct-mapped or set-associative cache simulator in C.
The simulator reads memory accesses, manages cache lines, handles cache hits/misses, and prints the cache content. It supports configurable cache parameters including block size, number of sets, associativity, and tag size.
It is designed to help understand cache behavior, memory hierarchy, and replacement policies (LFU in this case).
Features
Configurable cache parameters:
s: number of set index bits
t: number of tag bits
b: number of block offset bits
E: number of lines per set (associativity)
Handles reads and writes to memory.
Tracks cache hits, cold misses, and conflict misses.
Supports LFU (Least Frequently Used) replacement policy.
Prints cache contents including:
Valid bit
Frequency
Tag
Block contents
Works on arbitrary memory sizes with dynamic allocation.
