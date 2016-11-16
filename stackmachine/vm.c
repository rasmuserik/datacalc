#include <stdlib.h>
#include "vm.h"

word_t *u_stack = 0;
word_t *u_heap = 0;
word_t heap_top = 0;
size_t u_size = 0;

void *u_init(size_t size) {
  void *p = realloc(u_heap, size);
  if(p) {
    u_heap = (word_t *) p;
    u_size = size / BYTES_PER_WORD;
    u_stack = ((word_t *) p) + u_size;
  }
  return p;
}

