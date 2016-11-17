//{{{1 includes
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ulang.h"

//{{{1 defines
#define BYTES_PER_WORD 2
//{{{1 global variables
word_t *stack = 0;
word_t *heap = 0;
word_t *stackend= 0;
size_t heaptop = 0;
size_t u_wordcount= 0;
//{{{1 Memory allocation
void u_malloc(unsigned int typetag, size_t pointers, size_t bytes) {//{{{2
  word_t *p = heap + heaptop;
  size_t size = bytes + 2*pointers;
  assert(stack >= heap + heaptop + size);
  size_t chunk = heaptop;
  heaptop = heaptop + 3 + (size + 1) / 2;
  --stack;
  // check for memory overflow

  // save chunk word to stack
  *stack = (word_t) chunk * 2;
  assert(chunk * 2 == (word_t) chunk * 2);

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

void *u_init(size_t size) {//{{{2
  void *p = realloc(heap, size);
  if(p) {
    heap = (word_t *) p;
    u_wordcount = size / BYTES_PER_WORD;
    stack = ((word_t *) p) + u_wordcount;
    stackend = (word_t *) p + size;
  }
  return p;
}

//{{{1 Word access
#define word_is_short_int(w) (w&1)
#define word_is_ptr(w) (!word_is_short_int(w))
word_t int_to_word(int i) { //{{{2
  // TODO handle overflow, by allocating int on heap.
  assert(2 * i == (word_t) 2 * i);
  return i * 2 | 1;
};
int word_to_int(word_t w) {//{{{2
  // TODO try load integer from heap if ptr instead of int
  assert(word_is_short_int(w));
  return (int) w / 2;
};
word_t heap_index_to_word(size_t i) {//{{{2
  return i * 2;
}
size_t word_to_heap_index(word_t w) {//{{{2
  assert(word_is_ptr(w));
  return (size_t) w / 2;
}
//{{{1 chunks
size_t chunk_ptr_count(size_t idx) {//{{{2
  return heap[idx+2] & 511;
}
size_t chunk_byte_count(size_t idx) {//{{{2
  return heap[idx+1];
}
size_t chunk_type(size_t idx) {//{{{2
  return heap[idx+2] >> 9;
}
size_t chunk_size(size_t idx) {//{{{2
  return 3 + chunk_ptr_count(idx) + (1 + chunk_byte_count(idx)) / 2;
}
//{{{1 Garbage Collector
void visit(size_t *unvisited_top, word_t *p) {//{{{2
    if(word_is_ptr(*p)) {
      size_t idx = word_to_heap_index(*p);
      if(0 == heap[idx]) {
        heap[idx] = *unvisited_top;
        *unvisited_top = idx;
      }
    }
}
void u_mark(size_t generation) {//{{{2
  size_t unvisited_top = ~0;
  for(word_t *p = stack; p < stackend; ++p) {
    visit(&unvisited_top, p);
  }
  while(unvisited_top != (word_t) ~0) {
    size_t idx = unvisited_top;
    unvisited_top = heap[unvisited_top];
    size_t ptrs = chunk_ptr_count(idx);
    for(int i = 0; i < ptrs; ++i) {
      visit(&unvisited_top, heap + idx + 3 + i);
    }
  }
}
void u_address_calculation(size_t idx) {//{{{2
  size_t adr = idx;
  while(idx < heaptop) {
    size_t size = chunk_size(idx);
    if(heap[idx]) {
      heap[idx] = adr;
      adr += size;
      idx += size;
    } else {
      idx += size;
    }
  }
}
void u_gc(size_t generation) {//{{{2
  u_mark(generation);
  u_address_calculation(generation);
}
//{{{1 memdump
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

  for(idx = 0; idx < heaptop; idx += size) {
    ptrs = heap[idx + 2] & 511;
    bytes = heap[idx + 1]; 
    size = 3 + ptrs + (bytes + 1) / 2;
    fprintf(f, "X%d ", (int) idx);
    fprintf(f, "%d ", heap[idx+2]>>9);
    fprintf(f, "%d %d %d ", size, ptrs, bytes);
    fprintf(f, "%d ", heap[idx]);
    fprintf(f, "[");
    for(i = 0; i < ptrs; ++i) {
      print_word(f, heap[idx + i + 3]);
    }
    fprintf(f, " ] ");

    cp = (uint8_t *) (heap + idx + i + 3);
    for(i = 0; i < bytes; ++i) {
      print_c(f, *cp++);
    }
    fprintf(f, "\n");
  }

  fprintf(f, "STACK:");
  for(p = stack; p < heap + u_wordcount; ++p) {
    print_word(f, *p);
  }
  fprintf(f, "\n");
}
//{{{1 Main
#ifdef __ULANG_MAIN__
int main() {
  printf("word_t* heap stack;\n");
  printf("size_t* heap_top, u_wordcount;\n\n");
  u_init(200);
  u_malloc(1,0,10);
  u_malloc(2,1,0);
  u_malloc(3,0,1);
  u_malloc(4,0,1);
  u_malloc(5,0,1);
  printf("start\n");
  uint8_t code[] = {u_DUP, u_PUSH_INT, 123, u_PUSH_INT, 27, u_ADD, u_LOG_INT, u_QUIT};
  u_run(code);
  u_run((uint8_t[]){u_PUSH_INT, 123, u_PUSH_N, 3, u_QUIT});
  u_run((uint8_t[]){u_TUPLE, 2, u_TUPLE, 3, u_QUIT});
  memdump(stdout);
  u_gc(0);
  memdump(stdout);

  printf("stop\n");
  return 0;
}
#endif //__ULANG_MAIN__
//{{{1 Interpreter loop
#include "ops.impl"