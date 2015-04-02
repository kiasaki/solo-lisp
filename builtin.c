#include <stdlib.h>
#include "lang.h"

lval* builtin_op(lenv* e, lval* a, char* op) {
  // ensure all elems left are numbers
  for (int i = 0; i < a->count; i++) {
    LASSERT_TYPE(op, a, i, LVAL_NUM);
  }

  lval* x = lval_pop(a, 0);

  // no more elem and "-" -> unary negation
  if (strcmp(op, "-") == 0 && a->count == 0) {
    x->num = -x->num;
  }

  // while there are elems left
  while (a->count > 0) {
    lval* y = lval_pop(a, 0);
    if (strcmp(op, "+") == 0) { x->num += y->num; }
    if (strcmp(op, "-") == 0) { x->num -= y->num; }
    if (strcmp(op, "*") == 0) { x->num *= y->num; }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        lval_delete(x); lval_delete(y);
        x = lval_err("Division by zero!"); break;
      }
      x->num /= y->num;
    }

    lval_delete(y);
  }

  lval_delete(a); return x;
}

lval* builtin_add(lenv* e, lval* a) {
  return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
  return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
  return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
  return builtin_op(e, a, "/");
}

lval* builtin_head(lenv* e, lval* v) {
  LASSERT(v, v->count == 1,
    "Function 'head' passed too many arguments! Got '%i', Expected '1'", v->count);
  LASSERT_TYPE("head", v, 0, LVAL_QEXPR);
  LASSERT(v, v->cell[0]->count != 0,
    "Function 'head' passed {}!");

  lval* x = lval_take(v, 0);
  // return head
  while(x->count > 1) { lval_delete(lval_pop(x, 1)); }
  return x;
}

lval* builtin_tail(lenv* e, lval* v) {
  LASSERT(v, v->count == 1,
    "Function 'tail' passed too many arguments! Got '%i', Expected '1'", v->count);
  LASSERT_TYPE("tail", v, 0, LVAL_QEXPR);
  LASSERT(v, v->cell[0]->count != 0,
    "Function 'tail' passed {}!");

  lval* x = lval_take(v, 0);
  // return tail
  lval_delete(lval_pop(x, 0));
  return x;
}

lval* builtin_list(lenv* e, lval* v) {
  v->type = LVAL_QEXPR;
  return v;
}

lval* builtin_eval(lenv* e, lval* v) {
  LASSERT(v, v->count == 1,
    "Function 'eval' passed too many arguments!");
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
    "Function 'eval' passed incorrect type!");

  lval* x = lval_take(v, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

lval* builtin_join(lenv* e, lval* v) {
  for (int i = 0; i < v->count; i++) {
    LASSERT(v, v->cell[i]->type == LVAL_QEXPR,
      "Function 'join' passsed incorrect type!");
  }

  lval* x = lval_pop(v, 0);

  while(v->count) {
    x = lval_join(x, lval_pop(v, 0));
  }

  lval_delete(v);
  return x;
}

lval* builtin_def(lenv* e, lval* a) {
  return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a) {
  return builtin_var(e, a, "=");
}

lval* builtin_var(lenv* e, lval* v, char* func) {
  LASSERT_TYPE(func, v, 0, LVAL_QEXPR);

  lval* syms = v->cell[0];
  for (int i = 0; i < syms->count; i++) {
    LASSERT(v, syms->cell[i]->type == LVAL_SYM,
      "Function '%s' cannot define non-symbol. "
      "Got %s, Expected %s.", func,
      ltype_name(syms->cell[i]->type),
      ltype_name(LVAL_SYM));
  }

  LASSERT(v, syms->count == v->count-1,
    "Function '%s' passed too many atguments for symbols. "
    "Got %i, Expected %i.", func, syms->count, v->count-1);

  for (int i = 0; i < syms->count; i++) {
    // def -> global, = (put) -> local
    if (strcmp(func, "def") == 0) {
      lenv_def(e, syms->cell[i], v->cell[i+1]);
    }
    if (strcmp(func, "=") == 0) {
      lenv_put(e, syms->cell[i], v->cell[i+1]);
    }
  }

  lval_delete(v);
  return lval_sexpr();
}

lval* builtin_lambda(lenv* e, lval* v) {
  LASSERT_NUM("\\", v, 2);
  LASSERT_TYPE("\\", v, 0, LVAL_QEXPR);
  LASSERT_TYPE("\\", v, 1, LVAL_QEXPR);

  // check that first qexpr contains only syms
  for (int i = 0; i < v->cell[0]->count; i++) {
    LASSERT(v, v->cell[0]->cell[i]->type == LVAL_SYM,
      "Cannot define non-symbol. Got %s, Expected %s.",
      ltype_name(v->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
  }

  // pop first two args and pass to lval_lambda
  lval* formals = lval_pop(v, 0);
  lval* body = lval_pop(v, 0);
  lval_delete(v);

  return lval_lambda(formals, body);
}
