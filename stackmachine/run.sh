node genInterpreter.js &&
  cc -Os -D __ULANG_MAIN__ *.c &&
  strip a.out &&
  ./a.out &&
  ls -l a.out && size a.out
