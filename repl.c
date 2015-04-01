#include <stdio.h>
#include <stdlib.h>
#include "lang.h"

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* simple readline for windows compat */
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

/* face add_history that's a noop */
void add_history(char* unused) {}

#else
#include <editline/readline.h>
#endif

int main(int argc, char** argv) {
  puts("REPL v0.0.1");
  puts("Press Ctrl+c to exit\n");

  // start a clean env, outside loop
  lenv* e = lenv_new();
  lenv_add_builtins(e);

  while(1) {
    char* input = readline("λ: ");
    add_history(input);

    lval* v = lval_eval(e, lval_read_str(input));
    lval_println(v);
    lval_delete(v);

    free(input);
  }

  lenv_delete(e);

  return 0;
}
