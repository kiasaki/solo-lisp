#include <stdlib.h>
#include <string.h>
#include "print.h"
#include "read.h"

char* ast_print(AST* d, int* failure) {
  char* r = malloc(0);

  switch (d->type) {
    case AST_ERR:
      *failure = 1;
      return d->str;
      break;
    case AST_NUM: {
      char* buffer = malloc(60);
      sprintf(buffer, "%li", d->num);
      strcat(r, buffer);
      free(buffer);
    } break;
    case AST_DEC: {
      char* buffer = malloc(60);
      sprintf(buffer, "%f", d->dec);
      strcat(r, buffer);
      free(buffer);
    } break;
    case AST_SYM:
      strcat(r, d->str);
      break;
    case AST_STR:
      strcat(r, "\"");
      char* escaped = malloc(strlen(d->str) + 1);
      strcpy(escaped, d->str);
      escaped = mpcf_escape(d->str);
      strcat(r, escaped);
      free(escaped);
      strcat(r, "\"");
      break;
    case AST_SEXPR:
    case AST_ROOT:
      if (d->type != AST_ROOT) strcat(r, "(");
      char* cell_result = NULL;

      for (int i = 0; i < d->count; i++) {
        if (i != 0) { strcat(r, " "); }
        if (cell_result != NULL) { *cell_result = 0; }
        cell_result = ast_print(d->cells[i], failure);

        // if we had an error in one of the child, return only error
        if (*failure) { free(r); return cell_result; }

        r = realloc(r, strlen(r) + strlen(cell_result) + 1);
        strcpy(r + strlen(r), cell_result);
        free(cell_result);
      }

      if (d->type != AST_ROOT) strcat(r, ")");
      break;
    default:
      strcat(r, "<unknown>");
      break;
  }

  return r;
}

char* type_name(int t) {
  switch(t) {
    case AST_FUN: return "function";
    case AST_NUM: return "number";
    case AST_DEC: return "decimal";
    case AST_STR: return "string";
    case AST_ERR: return "error";
    case AST_SYM: return "symbol";
    case AST_SEXPR: return "s-expr";
    default: return "unknown";
  }
}
