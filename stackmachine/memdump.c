#include <stdio.h>
#include <stdint.h>
#include "vm.h"

void print_word(FILE *f, word_t w) {
  if(word_is_ptr(w)) {
    fprintf(f, " X%d", (int) word_to_heap_index(w));
  } else {
    fprintf(f, " %d", word_to_int(w));
  }
}

void print_c(FILE *f, uint8_t c) {
  if(c < 32 || c >= '{') {
    fprintf(f, "{%x}", c);
  } else {
    fprintf(f, "%c", c);
  }
}

void memdump(FILE *f) {
  fprintf(f, "-~=#| MEMORY DUMP |#=~-\n");
  word_t *p;
  uint8_t *cp;
  size_t idx;
  int i, ptrs, bytes, size = 0;


  fprintf(f, "ptr type size ptrs bytes gc PTRS BYTES\n");


  for(idx = 0; idx < u_heaptop; idx += size) {
    ptrs = u_heap[idx + 2] & 511;
    bytes = u_heap[idx + 1]; 
    size = 3 + ptrs + (bytes + 1) / 2;
    fprintf(f, "X%d ", (int) idx);
    fprintf(f, "%d ", u_heap[idx+2]>>9);
    fprintf(f, "%d %d %d ", size, ptrs, bytes);
    fprintf(f, "%d ", u_heap[0]);
    fprintf(f, "[");
    for(i = 0; i < ptrs; ++i) {
      print_word(f, u_heap[idx + i + 3]);
    }
    fprintf(f, " ] ");

    cp = (uint8_t *) (u_heap + idx + i + 3);
    for(i = 0; i < bytes; ++i) {
      print_c(f, *cp++);
    }
    fprintf(f, "\n");
  }

  fprintf(f, "STACK:");
  for(p = u_stack; p < u_heap + u_wordcount; ++p) {
    print_word(f, *p);
  }
  fprintf(f, "\n");
}
