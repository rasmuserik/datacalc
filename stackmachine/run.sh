node genInterpreter.js &&
  cc -D __ULANG_MAIN__ *.c &&
  strip a.out &&
  ./a.out &&
  ls -l a.out
