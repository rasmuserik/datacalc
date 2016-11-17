#include <stdint.h>

#ifndef __VM_H__
#define __VM_H__

typedef uint16_t word_t;
#define BYTES_PER_WORD 2
#define word_is_short_int(w) (w&1)
#define word_is_ptr(w) (!word_is_short_int(w))

extern word_t *u_stack;
extern word_t *u_heap;
extern size_t u_heaptop;
extern size_t u_wordcount;
void u_malloc(unsigned int typetag, size_t pointers, size_t databytes);
void *u_init(size_t);

word_t int_to_word(int i);
int word_to_int(word_t i);
word_t heap_index_to_word(size_t i);
size_t word_to_heap_index(word_t i);

#endif /* __VM_H__ */
