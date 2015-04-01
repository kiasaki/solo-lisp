#include <stdlib.h>
#include "mpc.h"
#include "lang.h"

#define LASSERT(args, cond, err) \
    if (!(cond)) { lval_delete(args); return lval_err(err); }

// Construct an new environment
lenv* lenv_new(void) {
  lenv* e = malloc(sizeof(lenv));
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

void lenv_delete(lenv* e) {
  for (int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    lval_delete(e->vals[i]);
  }
  free(e->syms);
  free(e->vals);
  free(e);
}

lval* lenv_get(lenv* e, lval* k) {
  for (int i = 0; i < e->count; i++) {
    // If we found an existing values for that key (sym) return a copy
    if (strcmp(e->syms[i], k->sym) == 0) {
      return lval_copy(e->vals[i]);
    }
  }
  // No match :(
  return lval_err("unbound symbol!");
}

void lenv_put(lenv* e, lval* k, lval* v) {
  for (int i = 0; i < e->count; i++) {
    // Match found free space and assign new value
    if (strcmp(e->syms[i], k->sym) == 0) {
      lval_delete(e->vals[i]);
      e->vals[i] = lval_copy(v);
      return;
    }
  }

  // No existing entry found, allocate space and assign
  e->count++;
  e->vals = realloc(e->vals, sizeof(lval*) * e->count);
  e->syms = realloc(e->syms, sizeof(char*) * e->count);

  e->vals[e->count-1] = lval_copy(v);
  e->syms[e->count-1] = malloc(strlen(k->sym)+1);
  strcpy(e->syms[e->count-1], k->sym);
}

// Construct a pointer to a number
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

// Construct a pointer to an error
lval* lval_err(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
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

// Construct a pointer to a fun
lval* lval_fun(lbuiltin func) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->fun = func;
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
    // Copy functions and numbers directly
    case LVAL_FUN: x->fun = v->fun; break;
    case LVAL_NUM: x->num = v->num; break;

    // Copy strings using malloc and strcpy
    case LVAL_ERR:
      x->err = malloc(strlen(v->err) + 1);
      strcpy(x->err, v->err); break;

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
    // Do nothing special for numbers and funcs
    case LVAL_NUM: break;
    case LVAL_FUN: break;

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

// Printing
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
    case LVAL_FUN:   printf("<function>"); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
  }
}

void lval_println(lval* v) {
  lval_print(v);
  putchar('\n');
}

// Reading
lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
  // handle symbol or number
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  // handle root (>) or sexpr
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
  if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

// Evaluating
lval* lval_eval(lval* v);

lval* builtin_op(lval* a, char* op) {
  // ensure all elems left are numbers
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_delete(a);
      return lval_err("Cannot operate on non-number!");
    }
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

lval* builtin_head(lval* v) {
  LASSERT(v, v->count == 1,
    "Function 'head' passed too many arguments!");
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
    "Function 'head' passed incorrect type!");
  LASSERT(v, v->cell[0]->count != 0,
    "Function 'head' passed {}!");

  lval* x = lval_take(v, 0);
  // return head
  while(x->count > 1) { lval_delete(lval_pop(x, 1)); }
  return x;
}

lval* builtin_tail(lval* v) {
  LASSERT(v, v->count == 1,
    "Function 'tail' passed too many arguments!");
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
    "Function 'tail' passed incorrect type!");
  LASSERT(v, v->cell[0]->count != 0,
    "Function 'tail' passed {}!");

  lval* x = lval_take(v, 0);
  // return tail
  lval_delete(lval_pop(x, 0));
  return x;
}

lval* builtin_list(lval* v) {
  v->type = LVAL_QEXPR;
  return v;
}

lval* builtin_eval(lval* v) {
  LASSERT(v, v->count == 1,
    "Function 'eval' passed too many arguments!");
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
    "Function 'eval' passed incorrect type!");

  lval* x = lval_take(v, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(x);
}

lval* builtin_join(lval* v) {
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

lval* builtin(lval* v, char* func) {
  if (strcmp("list", func) == 0) { return builtin_list(v); }
  if (strcmp("head", func) == 0) { return builtin_head(v); }
  if (strcmp("tail", func) == 0) { return builtin_tail(v); }
  if (strcmp("join", func) == 0) { return builtin_join(v); }
  if (strcmp("eval", func) == 0) { return builtin_eval(v); }
  if (strstr("+-/*", func)) { return builtin_op(v, func); }
  lval_delete(v);
  return lval_err("Unknown builtin function!");
}

lval* lval_eval(lval* v) {
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
  return v;
}

lval* lval_eval_sexpr(lval* v) {
  // Eval chidrens first
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(v->cell[i]);
  }

  // Error checking
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }

  // Empty expr "()"
  if (v->count == 0) { return v; }

  // Single expr "(5)"
  if (v->count == 1) { return lval_take(v, 0); }

  // Ensure first elem is sym
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_SYM) {
    lval_delete(f); lval_delete(v);
    return lval_err("sexpr doesn't start with a symbol!");
  }

  // Call builtin operator
  lval* result = builtin(v, f->sym);
  lval_delete(f);
  return result;
}

lval* lval_read_str(char* input) {
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Symbol   = mpc_new("symbol");
  mpc_parser_t* Sexpr    = mpc_new("sexpr");
  mpc_parser_t* Qexpr    = mpc_new("qexpr");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT,
      "                                                    \
        number : /-?[0-9]+/ ;                              \
        symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;        \
        sexpr  : '(' <expr>* ')' ;                         \
        qexpr  : '{' <expr>* '}' ;                         \
        expr   : <number> | <symbol> | <sexpr> | <qexpr> ; \
        lispy  : /^/ <expr>* /$/ ;                         \
      ",
      Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

  mpc_result_t r;
  lval* v;

  if (mpc_parse("<stdin>", input, Lispy, &r)) {
    v = lval_read(r.output);
    mpc_ast_delete(r.output);
  } else {
    char* mess = mpc_err_string(r.error);
    v = lval_err(mess);
    mpc_err_delete(r.error);
    free(mess);
  }

  mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

  return v;
}
