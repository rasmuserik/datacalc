#include <stdio.h>
#include "vm.h"
#include "interpreter.h"

void memdump(); // memdump.c
int main() {
  printf("word_t* u_heap u_stack;\n");
  printf("size_t* heap_top, u_wordcount;\n\n");
  u_init(200);
  u_malloc(1,0,10);
  u_malloc(2,1,0);
  u_malloc(0,0,1);
  u_malloc(0,0,1);
  printf("start\n");
  uint8_t code[] = {HELLO, PUSH_INT, 123, PUSH_INT, 27, ADD, LOG_INT, QUIT};
  u_run(code);
  u_run((uint8_t[]){HELLO, QUIT});
  u_run((uint8_t[]){PUSH_INT, 123});
  memdump(stdout);
  printf("stop\n");
  return 0;
}
