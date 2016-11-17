#include <stdlib.h>
#include <assert.h>
#include "vm.h"

word_t *u_stack = 0;
word_t *u_heap = 0;
size_t u_heaptop = 0;
size_t u_wordcount= 0;

void u_malloc(unsigned int typetag, size_t pointers, size_t bytes) {
  word_t *p = u_heap + u_heaptop;
  size_t size = bytes + 2*pointers;
  size_t heaptop = u_heaptop + 3 + (size + 1) / 2;
  --u_stack;
  // check for memory overflow
  assert(u_stack >= u_heap + heaptop);

  // save chunk word to stack
  *u_stack = (word_t) u_heaptop * 2;
  assert(u_heaptop * 2 == (word_t) u_heaptop * 2);

  // move heaptop top beginning of empty space)
  u_heaptop = heaptop;

  // initialise chunk header bytes
  p[0] = 0;

  assert(bytes == (word_t) bytes);
  p[1] = (word_t) bytes;

  assert(pointers < 512);
  size_t t = (typetag << 9) | (pointers & 511);
  assert(typetag == (word_t) typetag);
  p[2] = (word_t) t;

  // reset pointers
  for(int i = 0; i < pointers; ++i) {
    p[3+i] = 0;
  }
}

word_t int_to_word(int i) {
  // TODO handle overflow, by allocating int on heap.
  assert(2 * i == (word_t) 2 * i);
  return i * 2 | 1;
};
int word_to_int(word_t w) {
  // TODO try load integer from heap if ptr instead of int
  assert(word_is_short_int(w));
  return (int) w / 2;
};
word_t heap_index_to_word(size_t i) {
  return i * 2;
}
size_t word_to_heap_index(word_t w) {
  assert(word_is_ptr(w));
  return (size_t) w / 2;
}

void *u_init(size_t size) {
  void *p = realloc(u_heap, size);
  if(p) {
    u_heap = (word_t *) p;
    u_wordcount = size / BYTES_PER_WORD;
    u_stack = ((word_t *) p) + u_wordcount;
  }
  return p;
}

