#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>

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
