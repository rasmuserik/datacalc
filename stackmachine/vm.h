#include <stdint.h>

#ifndef __VM_H__
#define __VM_H__

typedef uint16_t word_t;
#define BYTES_PER_WORD 2

extern word_t *u_stack;
extern word_t *u_heap;
word_t heap_top;
void *u_init(size_t);

#endif /* __VM_H__ */
