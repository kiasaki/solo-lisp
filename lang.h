#ifndef lang_h
#define lang_h

#include <stdlib.h>
#include "mpc.h"

enum {
  LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_FUN,
  LVAL_SEXPR, LVAL_QEXPR
};

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

// builtin func
typedef lval*(*lbuiltin)(lenv*, lval*);

// lenv
struct lenv {
  lenv* parent;
  int count;
  char** syms;
  lval** vals;
};

lenv* lenv_new(void);
void lenv_delete(lenv*);
lenv* lenv_copy(lenv*);
lval* lenv_get(lenv*, lval*);
void lenv_put(lenv*, lval*, lval*);
void lenv_def(lenv*, lval*, lval*);
void lenv_add_builtin(lenv*, char*, lbuiltin);
void lenv_add_builtins(lenv*);

// lval

struct lval {
  int type;

  // basics
  long num;
  char* err;
  char* sym;

  // function
  lbuiltin builtin;
  lenv* env;
  lval* formals;
  lval* body;

  // expression
  int count;
  lval** cell;
};

lval* lval_num(long);
lval* lval_err(char*, ...);
lval* lval_sym(char*);
lval* lval_builtin(lbuiltin);
lval* lval_lambda(lval*, lval*);
lval* lval_sexpr(void);
lval* lval_qexpr(void);

lval* lval_add(lval*, lval*);
lval* lval_copy(lval*);

void lval_delete(lval*);

lval* lval_pop(lval*, int);
lval* lval_take(lval*, int);
lval* lval_join(lval*, lval*);
lval* lval_call(lenv*, lval*, lval*);

// other
char* ltype_name(int t);

void lval_expr_print(lval*, char, char);
void lval_print(lval*);
void lval_println(lval*);

lval* lval_read_num(mpc_ast_t*);
lval* lval_read(mpc_ast_t* t);

lval* builtin_op(lenv*, lval*, char*);
lval* builtin_head(lenv*, lval*);
lval* builtin_tail(lenv*, lval*);
lval* builtin_list(lenv*, lval*);
lval* builtin_eval(lenv*, lval*);
lval* builtin_join(lenv*, lval*);
lval* builtin_def(lenv*, lval*);
lval* builtin_lambda(lenv*, lval*);
lval* builtin_add(lenv*, lval*);
lval* builtin_sub(lenv*, lval*);
lval* builtin_mul(lenv*, lval*);
lval* builtin_div(lenv*, lval*);
lval* builtin_var(lenv*, lval*, char*);
lval* builtin_put(lenv*, lval*);
lval* builtin(lval*, char*);

lval* lval_eval(lenv*, lval*);
lval* lval_eval_sexpr(lenv*, lval*);
lval* lval_read_str(char*);

#endif
