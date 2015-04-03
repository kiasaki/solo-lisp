#include <stdlib.h>
#include "mpc.h"
#include "solo.h"

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

lval* builtin_let(lenv* e, lval* a) {
  return builtin_var(e, a, "let");
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
    if (strcmp(func, "let") == 0) {
      lenv_put(e, syms->cell[i], v->cell[i+1]);
    }
  }

  lval_delete(v);
  return lval_sexpr();
}

lval* builtin_lambda(lenv* e, lval* v) {
  LASSERT_NUM("fn", v, 2);
  LASSERT_TYPE("fn", v, 0, LVAL_QEXPR);
  LASSERT_TYPE("fn", v, 1, LVAL_QEXPR);

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

// comparison
lval* builtin_ord(lenv* e, lval* v, char* op) {
  LASSERT_NUM(op, v, 2);
  LASSERT_TYPE(op, v, 0, LVAL_NUM);
  LASSERT_TYPE(op, v, 1, LVAL_NUM);

  int r;
  if (strcmp(op, ">")  == 0) { r = (v->cell[0]->num >  v->cell[1]->num); }
  if (strcmp(op, "<")  == 0) { r = (v->cell[0]->num <  v->cell[1]->num); }
  if (strcmp(op, ">=") == 0) { r = (v->cell[0]->num >= v->cell[1]->num); }
  if (strcmp(op, "<=") == 0) { r = (v->cell[0]->num <= v->cell[1]->num); }
  lval_delete(v);
  return lval_num(r);
}

lval* builtin_gt(lenv* e, lval* v) { return builtin_ord(e, v, ">");  }
lval* builtin_lt(lenv* e, lval* v) { return builtin_ord(e, v, "<");  }
lval* builtin_ge(lenv* e, lval* v) { return builtin_ord(e, v, ">="); }
lval* builtin_le(lenv* e, lval* v) { return builtin_ord(e, v, "<="); }

lval* builtin_cmp(lenv* e, lval* v, char* op) {
  LASSERT_NUM(op, v, 2);
  int r;
  if (strcmp(op, "==") == 0) { r =  lval_eq(v->cell[0], v->cell[1]); }
  if (strcmp(op, "!=") == 0) { r = !lval_eq(v->cell[0], v->cell[1]); }
  lval_delete(v);
  return lval_num(r);
}

lval* builtin_eq(lenv* e, lval* a) { return builtin_cmp(e, a, "=="); }
lval* builtin_ne(lenv* e, lval* a) { return builtin_cmp(e, a, "!="); }

lval* builtin_if(lenv* e, lval* a) {
  LASSERT_NUM("if", a, 3);
  LASSERT_TYPE("if", a, 0, LVAL_NUM);
  LASSERT_TYPE("if", a, 1, LVAL_QEXPR);
  LASSERT_TYPE("if", a, 2, LVAL_QEXPR);

  lval* x;
  a->cell[1]->type = LVAL_SEXPR;
  a->cell[2]->type = LVAL_SEXPR;

  if (a->cell[0]->num) {
    x = lval_eval(e, lval_pop(a, 1));
  } else {
    x = lval_eval(e, lval_pop(a, 2));
  }

  lval_delete(a);
  return x;
}

// misc
lval* builtin_load(lenv* e, lval* v) {
  LASSERT_NUM("load", v, 1);
  LASSERT_TYPE("load", v, 0, LVAL_STR);

  // parse file given by string name
  mpc_result_t r;
  if (mpc_parse_contents(v->cell[0]->str, Lang, &r)) {

    // read contents
    lval* expr = lval_read(r.output);
    mpc_ast_delete(r.output);

    // evaluate each expression
    while (expr->count) {
      lval* x = lval_eval(e, lval_pop(expr, 0));
      // if evaluation leads to error return it
      if (x->type == LVAL_ERR) {
        lval_delete(expr);
        lval_delete(v);
        return x;
      }
      lval_delete(x);
    }

    // delete expressions and arguments
    lval_delete(expr);
    lval_delete(v);

    return lval_sexpr();
  } else {
    // get parse error as string
    char* err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);

    // create new error message using it
    lval* err = lval_err("Could not load Library %s", err_msg);
    free(err_msg);
    lval_delete(v);

    return err;
  }
}

lval* builtin_print(lenv* e, lval* v) {
  // Print each argument followed by a space
  for (int i = 0; i < v->count; i++) {
    lval_print(v->cell[i]); putchar(' ');
  }

  // Print a newline and delete arguments
  putchar('\n');
  lval_delete(v);

  return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* v) {
  LASSERT_NUM("error", v, 1);
  LASSERT_TYPE("error", v, 0, LVAL_STR);

  // construct error from first argument
  lval* err = lval_err(v->cell[0]->str);

  // Delete arguments and return
  lval_delete(v);
  return err;
}

lval* builtin_type(lenv* e, lval* v) {
  LASSERT_NUM("error", v, 1);

  lval* str = lval_str(ltype_name(v->cell[0]->type));

  lval_delete(v);
  return str;
}
