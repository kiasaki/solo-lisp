#include <stdio.h>
#include <stdlib.h>
#include "solo.h"

// Support windows minimally
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

// simple readline for windows compat
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

// fake add_history that's a noop
void add_history(char* unused) {}

#else
#include <editline/readline.h>
#endif

int main(int argc, char** argv) {
  // start a clean env, load builtins
  parser_setup();
  lenv* e = lenv_new();
  lenv_add_builtins(e);

  if (argc >= 2) {
    // run files
    for (int i = 1; i < argc; i++) {
      // argument list with a single argument, the filename
      lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));

      // pass to builtin load
      lval* x = builtin_load(e, args);

      // if the result is an error, print it
      if (x->type == LVAL_ERR) { lval_println(x); }
      lval_delete(x);
    }
    return 0;
  } else {
    // run repl
    puts("Solo v0.1.0");
    puts("Type :? for help, :q to quit\n");

    while(1) {
      char* input = readline("Σ: ");
      add_history(input);

      lval* v = lval_eval(e, lval_read_str(input));
      lval_println(v);
      lval_delete(v);

      free(input);
    }
  }

  lenv_delete(e);
  parser_free();
  return 0;
}
