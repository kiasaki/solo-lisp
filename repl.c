#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

int main(int argc, char** argv) {
  puts("REPL v0.0.1");
  puts("Press Ctrl+c to exit\n");

  while(1) {
    char* input = readline("λ: ");
    add_history(input);
    printf("okay: %s\n", input);
    free(input);
  }

  return 0;
}
