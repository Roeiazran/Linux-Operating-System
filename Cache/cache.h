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

typedef struct address_s
{
    long offset;
    long set_index;
    long tag;
} address_t;

cache_t initialize_cache(uchar s, uchar t, uchar b, uchar E);
uchar read_byte(cache_t cache, uchar* start, long int off);
void write_byte(cache_t cache, uchar* start, long int off, uchar new);
address_t* extract_bts_from_address(long int addr, uchar s, uchar t, uchar b);
int get_status(cache_line_t *set, uchar num_of_lines, long tag, int *line_index);
void insert_data_to_block(cache_line_t *line, uchar* start, long int off, uchar b, long tag);
