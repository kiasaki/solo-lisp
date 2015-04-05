#include <stdio.h>
#include <stdlib.h>
#include "readline.h"

char* read(char* line) {
  return line;
}

char* eval(char* line) {
  return line;
}

void print(char* line) {
  printf("%s\n", line);
}

void rep(void) {
  char* line;
  line = readline("user> ");

  print(eval(read(line)));

  free(line);
}

int main(int argc, char** argv) {
  while (1) {
    rep();
  }
}
