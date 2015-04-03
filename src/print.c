#include <stdlib.h>
#include "solo.h"

// type name for errors
char* ltype_name(int t) {
  switch(t) {
    case LVAL_FUN: return "function";
    case LVAL_NUM: return "number";
    case LVAL_STR: return "string";
    case LVAL_ERR: return "error";
    case LVAL_SYM: return "symbol";
    case LVAL_SEXPR: return "s-expr";
    case LVAL_QEXPR: return "q-expr";
    default: return "unknown";
  }
}

void lval_expr_print(lval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    lval_print(v->cell[i]);

    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lval* v) {
  switch (v->type) {
    case LVAL_NUM:   printf("%li", v->num); break;
    case LVAL_ERR:   printf("Error: %s", v->err); break;
    case LVAL_SYM:   printf("%s", v->sym); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
    case LVAL_FUN:
      if (v->builtin) {
        printf("<builtin>");
      } else {
        printf("(\\ "); lval_print(v->formals);
        putchar(' '); lval_print(v->body); putchar(')');
      }
    break;
  }
}

void lval_println(lval* v) {
  lval_print(v);
  putchar('\n');
}
