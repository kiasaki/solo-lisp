#ifndef lang_h
#define lang_h

#include <stdlib.h>
#include "mpc.h"

enum {
  LVAL_ERR = 0,
  LVAL_NUM = 1,
  LVAL_SYM = 2,
  LVAL_SEXPR = 3
};

typedef struct lval {
  int type;
  long num;

  char* err;
  char* sym;

  int count;
  struct lval** cell;
} lval;

lval* lval_num(long);
lval* lval_err(char*);
lval* lval_sym(char*);
lval* lval_sexpr(void);

lval* lval_add(lval*, lval*);

void lval_delete(lval*);

lval* lval_read_num(mpc_ast_t*);
lval* lval_read(mpc_ast_t* t);

void lval_expr_print(lval*, char, char);
void lval_print(lval*);
void lval_println(lval*);

lval* parse_lval_str(char*);

#endif
