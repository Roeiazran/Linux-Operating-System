#include "cache.h"
#include <stdio.h>
#include <stdlib.h>

#define COLD_MISS 1
#define CONFLICT_MISS 2
#define CACHE_HIT 3

/**
 * @brief Initializes a cache structure with S sets, E lines per set, and B-byte blocks.
 *
 * Allocates memory for the cache structure, the sets, each line, and the blocks.
 * Initializes each line's `valid` and `frequency` fields to 0, and `tag` to 0.
 *
 * @param s Number of set index bits (S = 2^s sets)
 * @param t Number of tag bits
 * @param b Number of block offset bits (B = 2^b bytes per block)
 * @param E Number of lines per set (associativity)
 * @return Initialized cache_t structure (by value)
 */
cache_t initialize_cache(uchar s, uchar t, uchar b, uchar E) {
    int S = 1 << s;  // Number of sets
    int B = 1 << b;  // Block size in bytes

    // Allocate memory for cache structure
    cache_t * cache_array = (cache_t*)malloc(sizeof(cache_line_t));

    // Set cache parameters
    cache_array->b = b;
    cache_array->s = s;
    cache_array->E = E;
    cache_array->t = t;

    // Allocate memory for sets (array of pointers to lines)
    cache_array->cache = (cache_line_t**)malloc(sizeof(cache_line_t*) * S);

    // Initialize each set
    for (int i = 0; i < S; i++) {

        // Allocate memory for E lines in the set
        cache_array->cache[i] = (cache_line_t*)malloc(sizeof(cache_line_t) * E);

        // Initialize each line in the set
        for (int j = 0; j < E; j++) {
            // Allocate memory for the block
            cache_array->cache[i][j].block = (uchar*)malloc(B);
            
            // Initialize metadata
            cache_array->cache[i][j].frequency = 0;
            cache_array->cache[i][j].valid = 0;
            cache_array->cache[i][j].tag = 0;
        }
    }

    // Return the initialized cache structure by value
    return (*cache_array);
}

/**
 * @brief Prints the entire cache contents in a readable format.
 *
 * Iterates over all sets and lines, printing the valid bit, frequency,
 * tag, and the contents of each block in hexadecimal.
 *
 * @param cache The cache structure to print (by value)
 */
void print_cache(cache_t cache) {
    int S = 1 << cache.s; // Number of sets
    int B = 1 << cache.b; // Block size in bytes

    // Iterate through all sets
    for (int i = 0; i < S; i++) {
        printf("Set %d\n", i);

        // Iterate through all lines in the current set
        for (int j = 0; j < cache.E; j++) {
            // Print line metadata: valid, frequency, and tag
            printf("%1d %d 0x%0*lx ", cache.cache[i][j].valid,
                cache.cache[i][j].frequency, cache.t, cache.cache[i][j].tag);

            // Print each byte in the block in hexadecimal
            for (int k = 0; k < B; k++) {
                printf("%02x ", cache.cache[i][j].block[k]);
            }
            puts(""); // Newline after each line
        }
    }
}

/**
 * @brief Reads a byte from the cache, handling hits and misses.
 *
 * Extracts set index, tag, and block offset from the address, checks
 * the cache for a hit or miss, updates frequency or inserts data as needed,
 * and finally returns the requested byte.
 *
 * @param cache The cache structure (by value)
 * @param start Pointer to the start of main memory (from which data is fetched on miss)
 * @param off The memory address offset to read
 * @return The byte value read from the cache line/block
 */
uchar read_byte(cache_t cache, uchar* start, long int off) {

    // Extract set index, tag, and block offset from the memory address
    address_t* addr_type = extract_bts_from_address(off, cache.s, cache.t, cache.b);

    // Get pointer to the set corresponding to the address
    cache_line_t* set = cache.cache[addr_type->set_index];
    
    int line_index = 0;

    // Check cache status: hit, cold miss, or conflict miss
    int status = get_status(set, cache.E, addr_type->tag, &line_index);

    // Get pointer to the specific cache line
    cache_line_t *line = set + line_index;

    // Handle cache hit/miss accordingly
    switch (status)
    {
        case COLD_MISS:
        case CONFLICT_MISS:
            // Insert data from memory to cache block on miss
            insert_data_to_block(line, start, off, cache.b, addr_type->tag);
            break; 
    
        case CACHE_HIT:
            // Increment frequency counter on hit
            line->frequency += 1;
            break;

        default:
            break;
    }

    // Return the byte from the line/block at the offset
    return line->block[addr_type->offset];
}

/**
 * @brief Inserts a block of data from memory into a cache line.
 *
 * Aligns the memory address to the start of the block, sets the cache
 * line metadata (valid, frequency, tag), and copies B bytes from memory
 * into the cache line's block.
 *
 * @param line Pointer to the cache line to fill
 * @param start Pointer to the start of main memory
 * @param off Memory address offset to start copying from
 * @param b Number of block offset bits (B = 2^b bytes per block)
 * @param tag Tag value to assign to the cache line
 */
void insert_data_to_block(cache_line_t *line, uchar* start, long int off, uchar b, long tag) {

    int B = 1 << b;  // Calculate block size in bytes

    // Align the offset to the start of the block
    while (off % B != 0) {
        off -= 1;
    }
    
    // Set cache line metadata
    line->valid = 1;
    line->frequency = 1;
    line->tag = tag;
    
    // Copy B bytes from memory into the cache line's block
    for (int i = 0; i < B; i++) {
        line->block[i] = *(start + off + i);
    }
}

/**
 * @brief Determines the cache status for a given tag in a set.
 *
 * Checks each line in the set for a cache hit, cold miss, or conflict miss.
 * Updates the line index to indicate which line should be used/evicted.
 *
 * @param set Pointer to the cache set (array of lines)
 * @param num_of_lines Number of lines in the set (E)
 * @param tag The tag to search for in the set
 * @param line_index Pointer to store the index of the line to use/evict
 * @return Status of the cache access: CACHE_HIT, COLD_MISS, or CONFLICT_MISS
 */
int get_status(cache_line_t *set, uchar num_of_lines, long tag, int *line_index) {

    // Initialize: assume first line as victim for possible eviction
    int min_frequency = set[0].frequency;
    *line_index = 0;

    // Loop through all lines in the set
    for (int i = 0; i < num_of_lines; i++) {

        // If an empty line found (never used)
        if (!set[i].valid) {
            *line_index = i; // Line available for insertion
            return COLD_MISS; // Cold miss (first-time use)
        }

        // If tag matches, it is a cache hit
        if (set[i].tag == tag) {
            *line_index = i; // Line with requested data
            return CACHE_HIT;
        }

        // Update victim line if current line has lower frequency
        if (set[i].frequency < min_frequency) {
            min_frequency = set[i].frequency;
            *line_index = i; // Candidate for eviction (conflict miss)
        }
    }

    // No hit and no empty line found: conflict miss
    return CONFLICT_MISS;
}
/**
 * @brief Extracts offset, set index, and tag bits from a memory address.
 *
 * Converts a decimal address to its cache components based on block, set, and tag sizes.
 *
 * @param dec_addr Memory address as decimal
 * @param s Number of set index bits
 * @param t Number of tag bits
 * @param b Number of block offset bits
 * @return Pointer to a dynamically allocated address_t structure
 */
address_t* extract_bts_from_address(long int dec_addr, uchar s, uchar t, uchar b) {

    address_t* address_ptr = (address_t*)malloc(sizeof(address_t));

    // Extract block offset bits (least significant b bits)
    for (int i = 0; i < b; i++) {
        if (dec_addr % 2) {
            address_ptr->offset += (1 << i);
        }
        dec_addr /= 2;
    }

    // Extract set index bits (next s bits)
    for (int i = 0; i < s; i++) {
        if (dec_addr % 2) {
            address_ptr->set_index += (1 << i);
        }
        dec_addr /= 2;
    }

    // Remaining bits are tag bits
    int i = 0;
    while (dec_addr) {
        if (dec_addr % 2) {
            address_ptr->tag += (1 << i);
        }
        i++;
        dec_addr /= 2;
    }

    return address_ptr;
}

/**
 * @brief Writes a byte to memory and updates the cache if there is a cache hit.
 *
 * Checks the cache for the target address, updates the cache line on hit, 
 * and always writes to main memory.
 *
 * @param cache The cache structure
 * @param start Pointer to main memory
 * @param off Memory address offset
 * @param new New byte value to write
 */
void write_byte(cache_t cache, uchar* start, long int off, uchar new) {
    
    address_t* addr_type = extract_bts_from_address(off, cache.s, cache.t, cache.b);
    cache_line_t* set = cache.cache[addr_type->set_index];
    
    int line_index = 0;
    int status = get_status(set, cache.E, addr_type->tag, &line_index);
    cache_line_t *line = set + line_index; // pointer to the cache line

    // Update cache line only on hit
    if (status == CACHE_HIT) {
        line->frequency += 1;
        line->block[addr_type->offset] = new;
    }
    
    // Always write to main memory
    *(start + off) = new;
}

/**
 * @brief Main function: reads memory, initializes cache, and simulates cache accesses.
 *
 * Prompts for memory size and data, cache parameters, and a series of memory reads.
 * Prints cache content at the end.
 */
int main() {

    int n;
    printf("Size of data: ");
    scanf("%d", &n);

    // Allocate memory
    uchar* mem = malloc(n);
    printf("Input data >> ");

    for (int i = 0; i < n; i++)
        scanf("%hhd", mem + i);

    // Read cache parameters
    int s, t, b, E;
    printf("s t b E: ");
    scanf("%d %d %d %d", &s, &t, &b, &E);

    // Initialize cache
    cache_t cache = initialize_cache(s, t, b, E);

    // Simulate cache reads until negative number is entered
    while (1) {
        scanf("%d", &n);
        if (n < 0) break;
        read_byte(cache, mem, n);
    }

    // Print cache contents
    puts("");
    print_cache(cache);

    // Free allocated memory
    free(mem);
}
