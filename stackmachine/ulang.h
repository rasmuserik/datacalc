#include <stdint.h>

#ifndef __VM_H__
#define __VM_H__

typedef uint16_t word_t;
word_t u_run(uint8_t *code);
void *u_init(size_t);
void u_main();
#include "ops.h"
#endif /* __VM_H__ */
