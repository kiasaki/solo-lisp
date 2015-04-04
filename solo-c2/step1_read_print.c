#include <stdio.h>
#include <stdlib.h>
#include "readline.h"
#include "read.h"

AST* read(char* line) {
  AST* datum;
  datum = read_str(line);
  return datum;
}

AST* eval(AST* datum) {
  return datum;
}

void print(AST* _) {
  printf("%s\n", "!");
}

void rep(void) {
  char* line;
  line = readline("user> ");
  add_history(line);

  print(eval(read(line)));

  free(line);
}

int main(int argc, char** argv) {
  reader_setup();

  while (1) {
    rep();
  }

  reader_free();
}
