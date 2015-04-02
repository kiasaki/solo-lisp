#include <stdlib.h>
#include "mpc.h"
#include "lang.h"

#define LASSERT(args, cond, fmt, ...) \
    if (!(cond)) { \
      lval* err = lval_err(fmt, ##__VA_ARGS__); \
      lval_delete(args); \
      return err; \
    }

#define LASSERT_TYPE(fnname, value, index, expected) \
    if (value->cell[index]->type != expected) { \
      lval* err = lval_err( \
        "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.", \
        fnname, index+1, ltype_name(value->cell[index]->type), ltype_name(expected)); \
      lval_delete(value); \
      return err; \
    }

#define LASSERT_NUM(fnname, value, exp_count) \
    if (value->count != exp_count) { \
      lval* err = lval_err( \
        "Function '%s' passed incorrect number of arguments. Got %i, Expected %i.", \
        fnname, value->count, exp_count); \
      lval_delete(value); \
      return err; \
    }

#define LASSERT_CELL_TYPES(args, type, fmt, ...) \
    for (int i = 0; i < args->count; i++) { \
      if (args->cell[i]->type != type) { \
        lval* err = lval_err(fmt, ##__VA_ARGS__); \
        lval_delete(args); \
        return err; \
      } \
    }

// type name for errors
char* ltype_name(int t) {
  switch(t) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    default: return "Unknown";
  }
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
