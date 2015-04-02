#include <stdlib.h>
#include "lang.h"

lval* lval_eval(lenv* e, lval* v) {
  if (v->type == LVAL_SYM) {
    lval* x = lenv_get(e, v);
    lval_delete(v);
    return x;
  }
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
  return v;
}

lval* lval_eval_sexpr(lenv* e, lval* v) {
  // Eval chidrens first
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(e, v->cell[i]);
  }

  // Error checking
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }

  // Empty and unary case () & (6)
  if (v->count == 0) { return v; }
  if (v->count == 1) { return lval_take(v, 0); }

  // Ensure first elem is sym
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_FUN) {
    lval* err = lval_err(
      "S-Expression starts with incorrect type. "
      "Got %s, Expected %s.",
      ltype_name(f->type), ltype_name(LVAL_FUN));
    lval_delete(f); lval_delete(v);
    return err;
  }

  // Call builtin operator
  lval* result = lval_call(e, f, v);
  lval_delete(f);
  return result;
}
