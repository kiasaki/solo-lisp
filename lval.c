#include <stdlib.h>
#include "lang.h"

// Construct a pointer to a number
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

// Construct a pointer to an error
lval* lval_err(char* fmt, ...) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;

  va_list va;
  va_start(va, fmt);

  // printf err at a max of 511 chars
  v->err = malloc(512);
  vsnprintf(v->err, 511, fmt, va);

  // realloc to take only the bytes we need
  v->err = realloc(v->err, strlen(v->err)+1);
  va_end(va);
  return v;
}

// Construct a pointer to a symbol
lval* lval_sym(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(m) + 1);
  strcpy(v->sym, m);
  return v;
}

// Construct a pointer to a builtin
lval* lval_builtin(lbuiltin builtin) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->builtin = builtin;
  return v;
}

// Construct a pointer to a lambda
lval* lval_lambda(lval* formals, lval* body) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;

  // this is how we tell it's a lambda not a builtin
  v->builtin = NULL;
  v->env = lenv_new();

  v->formals = formals;
  v->body = body;
  return v;
}

// Construct a pointer to a sexpr
lval* lval_sexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

// Construct a pointer to a qexpr
lval* lval_qexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

// Add a new cell to an existing lval
lval* lval_add(lval* v, lval*x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

// Deep copy an lval
lval* lval_copy(lval* v) {
  lval* x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type) {
    // Copy numbers directly
    case LVAL_NUM: x->num = v->num; break;

    case LVAL_FUN:
      if (v->builtin) {
        x->builtin = v->builtin;
      } else {
        x->builtin = NULL;
        x->env = lenv_copy(v->env);
        x->formals = lval_copy(v->formals);
        x->body = lval_copy(v->body);
      }
    break;

    // Copy strings using malloc and strcpy
    case LVAL_ERR:
      x->err = malloc(strlen(v->err) + 1);
      strcpy(x->err, v->err);
    break;

    case LVAL_SYM:
      x->sym = malloc(strlen(v->sym) + 1);
      strcpy(x->sym, v->sym); break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      x->count = v->count;
      x->cell = malloc(sizeof(lval*) * x->count);
      for (int i = 0; i < x->count; i++) {
        x->cell[i] = lval_copy(v->cell[i]);
      }
    break;
  }

  return x;
}

// Free the head from an lval
void lval_delete(lval* v) {
  switch (v->type) {
    // Do nothing special for numbers
    case LVAL_NUM: break;

    case LVAL_FUN:
      if (!v->builtin) {
        lenv_delete(v->env);
        lval_delete(v->formals);
        lval_delete(v->body);
      }
    break;

    // For err and sym free the string data
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;

    // in a sexpr's case we need to recursively free lvals
    case LVAL_SEXPR:
    case LVAL_QEXPR:
      for (int i = 0; i < v->count; i++) {
        lval_delete(v->cell[i]);
      }
      free(v->cell);
    break;
  }

  free(v);
}

// lval eval helpers
lval* lval_pop(lval* v, int i) {
  lval* x = v->cell[i];

  // shift all elems left
  memmove(&v->cell[i], &v->cell[i+1],
      sizeof(lval*) * (v->count-i-1));

  v->count--;

  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  return x;
}

lval* lval_take(lval* v, int i) {
  lval* x = lval_pop(v, i);
  lval_delete(v);
  return x;
}

// Adds all cells in y to x
lval* lval_join(lval* x, lval* y) {
  while (y->count) {
    x = lval_add(x, lval_pop(y, 0));
  }

  lval_delete(y);
  return x;
}

// call builtin of eval user defined func
lval* lval_call(lenv* e, lval* f, lval* v) {
  if (f->builtin) { return f->builtin(e, v); }

  int given = v->count;
  int total = f->formals->count;

  // consume all arguments passed in
  while (v->count) {
    if (f->formals->count == 0) {
      lval_delete(v);
      return lval_err(
        "Function passed too many arguments. "
        "Got %i, Expected %i.", given, total);
    }

    // get args 0 and val 0 and bind them in function's scope
    lval* sym = lval_pop(f->formals, 0);
    lval* val = lval_pop(v, 0);
    lenv_put(f->env, sym, val);

    lval_delete(sym); lval_delete(val);
  }

  // all args were bound, discard it's container
  lval_delete(v);

  // do we have all needed args?
  if (f->formals->count == 0) {
    // set parent env on local scope, global are now accessible
    f->env->parent = e;
    // eval and return
    return builtin_eval(
      f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
  } else {
    // otherwise, return partially applied function
    return lval_copy(f);
  }
}
