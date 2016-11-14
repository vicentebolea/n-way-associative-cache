#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

// Global & types declaration {{{
#define WORD_WIDTH 0 // Bits
#define WORD_SIZE 4  // Bytes

enum cache_block_state {
  INVALID,
  VALID
};

typedef unsigned uint;

typedef struct cache_block{
  enum cache_block_state status;
  uint tag;
  uint time;
}cache_block;

uint sets;
uint assoc;
uint block;
uint block_log2 = 0;
uint total_cache_accesses = 0;
uint total_cache_misses = 0;
uint sim_time = 0;
struct cache_block *cache;
//}}}
// Declarations {{{
uint         index_of_addr(uint addr, uint index_width, uint block_width);
uint         tag_of_addr(uint addr, uint index_width, uint block_width);
cache_block* get_block(uint offset, uint tag);
void         insert_block(uint offset, uint tag);
cache_block* evict_block(uint offset);
bool         is_full(uint offset);
cache_block* find_next_empty(uint offset);
void         increment_all(uint offset, uint tag);
void         cache_access(char inst_type, uint addr);
// }}}
// index_of_addr {{{
// It will compute the index of a given address
uint index_of_addr(uint addr, uint index_width, uint block_width) {
  uint output = 0;

  block_width += WORD_WIDTH;
  uint index_pos = index_width + block_width;
  uint mask_left  = ((1 << (index_pos)) - 1); //! left = 4  then mask: 0000 1111 1111

  output = addr & mask_left;

  output >>= block_width;

  return output;
}
// }}}
// tag_of_addr {{{
// It will compute the tag of a given address
uint tag_of_addr(uint addr, uint index_width, uint block_width) {
  int shift = index_width + block_width + WORD_WIDTH;
  addr >>= shift;
  assert(addr < (1 << (WORD_SIZE - (index_width + block_width)))); // Invariant

  return addr;
}
// }}}
// get_block {{{
// It will return the block inside the set given its tag
cache_block* get_block(uint offset, uint tag) {
  int i;
  for (i = 0; i < assoc; i++) {
    cache_block* block = &cache[offset + i];
    if (block->tag == tag)
      return block;
  }

  return NULL;
}
// }}}
// insert_block {{{
// It will insert the block inside the cache
// If there is no space it will evict the LRU block
void insert_block(uint index, uint tag) {
  uint offset = index * assoc;
  cache_block* block = get_block(offset, tag);

  if (block != NULL) {          //! If we have a cache hit
    block->time = 0;

  } else {                      //! If we have a cache miss
    total_cache_misses++;
    cache_block *block = NULL;

    if (is_full(offset)) {           //! If our cache is full, evict
      block = evict_block(offset);   //! the LRU block

    } else {                             //! If it is not full, 
      block = find_next_empty(offset);   //! place it in the first empty slot
    }

    block->tag = tag;
    block->status = VALID;
    block->time = 0;
  }
  increment_all(offset, tag);  //! Increment the time for each element but the one we just placed it
}
// }}}
// evict_block {{{
// It will return the LRU block of a given set
cache_block* evict_block(uint offset) {
  int i = 0;                
  uint max = 0;
  int max_index = 0;
  for (; i < assoc; i++) {
    cache_block* block = &cache[offset + i];
    if (block->time > max) {
      max = block->time;
      max_index = i;
    }
  }
  return &cache[offset + max_index];
}
// }}}
// is_full {{{
// returns whether the set is full
bool is_full(uint offset) {
  int i;
  for (i = 0; i < assoc; i++) {
    cache_block* block = &cache[offset + i];
    if (block->status == INVALID) return false;
  }

  return true;
}
// }}}
// find_next_empty {{{
// It will return the next empty slot
cache_block* find_next_empty(uint offset) {
  int i;
  for (i = 0; i < assoc; i++) {
    cache_block* block = &cache[offset + i];
    if (block->status == INVALID) return block;
  }
  return NULL;
}
// }}}
// increment_all {{{
// Increment the index of all the block in the give set
// but the one specified in the tag
void increment_all(uint offset, uint tag) {
  int i;
  for (i = 0; i < assoc; i++) {
    cache_block* block = &cache[offset + i];

    if (block->tag != tag) {
      block->time = block->time + 1;
    }
  }
}
// }}}
// cache_access {{{
void cache_access(char inst_type, uint addr) {
  uint index = index_of_addr(addr, log2(sets), block_log2);
  uint tag   = tag_of_addr(addr, log2(sets), block_log2);

  assert(index < sets);                                // Invariant
  assert((addr >> block_log2) == (index + (tag << (int)log2(sets)))); // Invariant: well formated tag and index

  insert_block(index, tag);
}
// }}}
// main {{{
int main(int argc, char** argv)
{
  if (argc != 5) {
    printf("Usage: %s <number_of_sets> <associativity> <cache_block_size> <trace_file>\n", argv[0]);
    return -1;
  }
  /*
     input parameters
     argv[1] = number of sets
     argv[2] = associativity
     argv[3] = cache block size
     argv[4] = trace file name
     */
  char inst_type = 0;
  uint addr = 0;
  sets = atoi(argv[1]);
  assoc = atoi(argv[2]);
  block = atoi(argv[3]);
  while (block >>= 1) ++block_log2;

  FILE *fp = fopen(argv[4], "r");
  if (fp == NULL){
    printf("trace file does not exist\n");
    return -1;
  }

  cache=malloc(sizeof(struct cache_block)*sets*assoc);
  int i=0;
  for(i=0;i<sets*assoc;i++){
    cache[i].time=0;
    cache[i].tag=0;
    cache[i].status=INVALID;
  }

  while (fscanf(fp, "%c 0x%x\n", &inst_type, &addr) != EOF) {
    cache_access(inst_type, addr);
    total_cache_accesses++;
    sim_time++;
  }

  printf("Cache accesses = %u\n", total_cache_accesses);
  printf("Cache misses = %u\n", total_cache_misses);

  free(cache);
  fclose(fp);

  return 0;
}
// }}}
