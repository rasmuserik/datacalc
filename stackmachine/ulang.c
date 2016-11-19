//#define NDEBUG 1 //1KB
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ops.h"

//{{{1 defines
#define STRING_TYPE 0
#define ATOM_TYPE 1
#define ARRAY_TYPE 2
#define HAMT_TYPE 3
#define true 1
#define false 0
typedef uint16_t word_t;
typedef size_t chunk_t;
word_t u_run(uint8_t *code);

#define BYTES_PER_WORD 2
#define BITS_PER_WORD 16
#define LOG_BITS_PER_WORD 4


void push_atom(char *);
//{{{1 global variables
word_t *stack = 0;
word_t *heap = 0;
word_t *stackend= 0;
size_t heaptop = 0;
size_t u_wordcount= 0;
//{{{1 Memory allocation
void ensure_stack_space() {
    assert(heap + heaptop < stack - 8);
}
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
  void *p = malloc(size);
  if(p) {
    heap = (word_t *) p;
    u_wordcount = size / BYTES_PER_WORD;
    stack = ((word_t *) p) + u_wordcount;
    stackend = (word_t *) p + size;
    push_atom("false");
    push_atom("true");
    push_atom("null");
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
word_t chunk_to_word(size_t i) {//{{{2
  return i * 2;
}
size_t word_to_chunk(word_t w) {//{{{2
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
//{{{1 Garbage Collector 1KB
//#define NO_GC
#ifdef NO_GC
void u_gc(size_t generation) {}
#else
void visit(size_t *unvisited_top, word_t *p) {//{{{2
    if(word_is_ptr(*p)) {
      size_t idx = word_to_chunk(*p);
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
    }
    idx += size;
  }
}
void u_update_address(word_t *word) {//{{{2
  word_t w = *word;
  if(word_is_ptr(w)) {
    *word = chunk_to_word(heap[word_to_chunk(w)]);
  }
}
void u_address_update(size_t idx) {  //{{{2
  while(idx < heaptop) {
    size_t size = chunk_size(idx);
    if(heap[idx]) {
      size_t ptrs = chunk_ptr_count(idx);
      for(int i = 0; i < ptrs; ++i) {
        u_update_address(heap + 3 + idx + i);
      }
    }
    idx += size;
  }
  for(word_t *p = stack; p < stackend; ++p) {
    u_update_address(p);
  }
}
void u_compact(size_t idx) { //{{{2
  size_t end = 0;
  while(idx < heaptop) {
    size_t size = chunk_size(idx);
    size_t dst = heap[idx];
    if(dst && dst != idx) {
      for(int i = 0; i < size; ++i) {
        heap[dst + i] = heap[idx + i];
      }
      end = dst + size;
    }
    idx += size;
  }
  heaptop = end;
}
// NB/TODO first object in heap has some issues around GC
void u_gc(size_t generation) {//{{{2
  u_mark(generation);
  u_address_calculation(generation);
  u_address_update(generation);
  u_compact(generation);
}
#endif /* NO_GC */
//{{{1 memdump 1KB
//#define NO_MEMDUMP
#ifdef NO_MEMDUMP
void memdump(FILE *f) {
}
#else
void print_word(FILE *f, word_t w) {
  if(word_is_ptr(w)) {
    fprintf(f, " X%d", (int) word_to_chunk(w));
  } else {
    fprintf(f, " %d", word_to_int(w));
  }
}

void print_c(FILE *f, uint8_t c) {
  if(c < 32 || c >= '{') {
    fprintf(f, "{%d}", c);
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
#endif /* NO_MEMDUMP */
//{{{1 Push to stack
void push_int(int i) {
  *--stack = int_to_word(i);
}
void push_short_int(int i) {
  *--stack = (word_t) i * 2 + 1;
  ensure_stack_space();
}
void push_string(char *s) {
  int i = 0;
  char c;
  while((c = s[i])) {
    push_short_int(c);
    ++i;
  }
  push_int(i);
  u_run((uint8_t[]){u_NEW_STRING, u_QUIT});
}
void push_atom(char *s) {
  push_string(s);
  heap[word_to_chunk(*stack) + 2] |= 512;
}
//{{{1 u_eval
int char_in_str(char c, char* p) {
  for(;;) {
    char c2 = *p;
    if(c2 == 0) {
      return 0;
    }
    if(c == c2) {
      return 1;
    }
    ++p;
  }
}
char *numeric = "-1234567890";
char *whitespace = " \t\n";
void u_eval(char *p) {
  for(;;) {
    char c = *p++;
    if(!c) return;
    if(char_in_str(c, whitespace)) {
      // Skip whitespace
      continue;
    } else if(char_in_str(c, numeric)) {
      // Tokenise integers
      int negative = 0;
      int result = 0;
      if(c == '-') {
        negative = 1;
        c = *p++;
      }
      do {
        result = result * 10 + c - '0';
        c = *p++;
      } while(char_in_str(c, numeric));
      result = negative ? -result : result;
      push_int(result);
    } else if(c == '"') {
      c = *p++;
      int len = 0;
      while(c != '"') {
        if(c == '\\') {
          c = *p++;
        }
        push_short_int(c);
        ++len;
        c = *p++;
      }
      push_int(len);
      u_run((uint8_t[]){u_NEW_STRING, u_QUIT});
    } else {
      // Tokenise Symbols
      char buf[20];
      char *cp = buf;
      do {
        *cp = c;
        c = *p++;
      } while(!char_in_str(c, whitespace));
    }
  }
}
//{{{1 EQUALS
int deep_equal(word_t w1, word_t w2) {
  if(w1 == w2) {
    return true;
  }
  if(word_is_short_int(w1) || word_is_short_int(w2)) {
    return false;
  }
  word_t *p1 = heap + word_to_chunk(w1);
  word_t *p2 = heap + word_to_chunk(w2);
  if(p1[1] != p2[1] || p1[2] != p2[2]) {
    return false;
  }
  size_t offset = 2 + (p1[2] & 511);
  uint8_t *cp1 = (uint8_t *) p1 + offset;
  uint8_t *cp2 = (uint8_t *) p2 + offset;
  size_t bytes = p1[1];
  for(int i = 0; i < bytes; ++i) {
    if(cp1[i] != cp2[i]) {
      return false;
    }
  }
  for(int i = 2; i < offset; ++i) {
    // TODO make non-recursive to preserve stack
    if(!deep_equal(p1[i], p2[i])) {
      return false;
    }
  }
  return true;
}
//{{{1 HAMT
//
// TODO:
// 
// - new generic update_hamt
// - update HAMT to four options
//
// - three options:
//   - single key/val
//   - hash-collision: link to 
//
// Data-types
// - HAMT-node:
//   - bitmap
//   - pointers
// - result-node
//   - key
//   - val
//   - [next] (if hash collision)
void empty_hamt() {//{{{2
  u_malloc(HAMT_TYPE, 0, 2 * BYTES_PER_WORD);
  word_t *chunk = heap + word_to_chunk(*stack);
  chunk[4] = chunk[5] = 0;
}
uint32_t word_hash(word_t w) { //{{{2
  if(word_is_short_int(w)) {
    return w / 2;
  }
  chunk_t chunk = word_to_chunk(w);
  size_t offset = chunk_ptr_count(chunk);
  size_t bytes = chunk_byte_count(chunk);
  // FNV-1a hash of content
  uint32_t hash = 2166136261;
  uint8_t *cp = (uint8_t *)(heap + chunk + offset);
  for(int i = 0; i < bytes; ++i) {
    hash = (hash ^ cp[i]) * 16777619;
  }
  return hash;
}
//{{{2 define HAMT_UPDATE_TYPES
#define HAMT_ADD_VALUE 0
#define HAMT_CHANGE_VALUE 1
#define HAMT_VALUE_TO_HAMT 2
#define HAMT_HAMT_TO_VALUE 3
#define HAMT_ADD_HAMT 4
#define HAMT_CHANGE_HAMT 5
#define HAMT_REMOVE_HAMT 6
word_t update_hamt(int type, chunk_t src, int pos, int bit, word_t w1, word_t w2) { //{{{2
  // this should be more like
  // src: 1 2 a  d   b 2 2  1 1 MASK1 MASK2
  //
  // for(i = 0; i < dst_len + 2; ++i) {
  //   dstp[i] = i < a 
  //             ? srcp[i]
  //             : (
  //             i < dst_len - 2 * b
  //             ? srcp[i + d]
  //             : srcp[i - dst_len + src_len]
  //             );
  // }
  // if(pos1 < 1000) {
  //   dstp[pos1 < 0 ? dst_len - i : i] = w1;
  // }
  // if(pos2 < 1000) {
  //   dstp[pos2 < 0 ? dst_len - i : i] = w2;
  // }
  int src_len = chunk_ptr_count(src);
  switch(type) {
    case HAMT_ADD_VALUE:
      {
        int dst_len = src_len + 2;
        u_malloc(HAMT_TYPE, dst_len, 2 * BYTES_PER_WORD);
        chunk_t dst = word_to_chunk(*stack);
        for(int i = 0; i < dst_len - pos; ++i) {
          heap[dst + 3 + i] = heap[src + 3 + i];
        }
        for(int i = 0; i <= pos * 2; ++i) {
          heap[dst + 3 + dst_len - i] = heap[src + 3 + src_len - i];
        }
        heap[dst + 3 + dst_len - pos * 2 - 2] = w1;
        heap[dst + 3 + dst_len - pos * 2 - 1] = w2;
        heap[dst + 3 + dst_len + 0] = heap[src + 3 + src_len + 0];
        heap[dst + 3 + dst_len + 1] = heap[src + 3 + src_len + 1] | bit;
        return chunk_to_word(dst);
      }
    case HAMT_CHANGE_VALUE:
      {
        int dst_len = src_len;
        u_malloc(HAMT_TYPE, dst_len, 2 * BYTES_PER_WORD);
        chunk_t dst = word_to_chunk(*stack);
        for(int i = 0; i < dst_len +2; ++i) {
          heap[dst + 3 + i] = heap[src + 3 + i];
        }
        heap[dst + 3 + dst_len - pos * 2 - 1] = w1;
        return chunk_to_word(dst);
      }
    break;
  }
  return 0;
}
word_t _hamt_insert(uint32_t hash, word_t key, word_t val, chunk_t hamt, int depth) { //{{{2

  uint32_t bit = 1 << ((hash >> depth) & (BITS_PER_WORD - 1));
  word_t *masks = heap + hamt + 3 + chunk_ptr_count(hamt);
  if(bit & masks[0]) { // sub-hamt-exists
    printf("a %d %d\n", bit, masks[0]);
    assert(0);
  } else if(bit & masks[1]) { // value exists
    int pos = __builtin_popcount(masks[1] & (bit - 1));
    if(deep_equal(heap[hamt + 3 + chunk_ptr_count(hamt) - pos * 2 - 2], key)) {
      return update_hamt(HAMT_CHANGE_VALUE, hamt, pos, bit, val, 0);
    }
    assert(0);
  } else {
    int pos = __builtin_popcount(masks[1] & (bit - 1));
    return update_hamt(HAMT_ADD_VALUE, hamt, pos, bit, key, val);
  }

  memdump(stdout);

  return 0;
}
void hamt_insert() {  //{{{2
  // key val hamt -> hamt
  word_t key = stack[0];
  word_t val = stack[1];
  chunk_t hamt = word_to_chunk(stack[2]);
  word_t *p = stack+2;
  *p = _hamt_insert(word_hash(key), key, val, hamt, 0);
  //*p = update_hamt(HAMT_ADD_VALUE, hamt, 0, 1, key, val);
  stack = p;
  memdump(stdout);
}
//{{{1 Main
#ifdef __ULANG_MAIN__
int main() {
  u_init(200);
  empty_hamt();
  /*
  push_string("a");
  push_int(0);
  hamt_insert();
  */
  push_string("b");
  push_int(1);
  hamt_insert();
  memdump(stdout);
  u_gc(0);
  memdump(stdout);
  u_gc(0);
  /*
  push_string("c");
  push_int(15);
  hamt_insert();
  u_gc(0);
  push_string("d");
  push_int(1);
  hamt_insert();
  u_gc(0);
  */
  memdump(stdout);
  /*
  printf("word_t* heap stack;\n");
  printf("size_t* heap_top, u_wordcount;\n\n");
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
  push_string("hello\n");
  memdump(stdout);

  printf("stop\n");
  */
  return 0;
}
#endif //__ULANG_MAIN__
//{{{1 Interpreter loop
#include "ops.impl"
