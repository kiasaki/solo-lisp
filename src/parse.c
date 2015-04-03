#include <stdlib.h>
#include "mpc.h"
#include "solo.h"

lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    lval_num(x) : lval_err("invalid number");
}

lval* lval_read_string(mpc_ast_t* t) {
  // cut off the final quote character
  t->contents[strlen(t->contents)-1] = '\0';
  // copy the string missing out the first quote character
  char* unescaped = malloc(strlen(t->contents+1)+1);
  strcpy(unescaped, t->contents+1);
  // pass through the unescape function
  unescaped = mpcf_unescape(unescaped);
  // construct a new lval using the string
  lval* str = lval_str(unescaped);
  // free the string and return
  free(unescaped);
  return str;
}

lval* lval_read(mpc_ast_t* t) {
  // handle symbol or number
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "string")) { return lval_read_string(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  // handle root (>) or sexpr or qexpr
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }

  if (strstr(t->tag, "qexpr"))  {
    // a qexpr is always 2 childs (quote and the item after)
    // if item after is a list we want to pop it
    x = lval_qexpr();
    // 0 is "'", 1 is what is after
    // if it's a sexpr, lower ourselves 1 level
    if (strstr(t->children[1]->tag, "sexpr")) {
      t = t->children[1];
    }
  }

  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "'") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    if (strstr(t->children[i]->tag,  "comment")) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

void parser_setup(void) {
  Number  = mpc_new("number");
  Symbol  = mpc_new("symbol");
  String  = mpc_new("string");
  Comment = mpc_new("comment");
  Sexpr   = mpc_new("sexpr");
  Qexpr   = mpc_new("qexpr");
  Expr    = mpc_new("expr");
  Lang    = mpc_new("lang");

  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                        \
      number  : /-?[0-9]+/ ;                                 \
      symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+|\\.\\.\\./ ; \
      string  : /\"(\\\\.|[^\"])*\"/ ;                       \
      comment : /;[^\\r\\n]*/ ;                              \
      sexpr   : '(' <expr>* ')' ;                            \
      qexpr   : '\'' <expr> ;                                \
      expr    : <number> | <symbol> | <comment>              \
              | <string> | <sexpr>  | <qexpr> ;              \
      lang    : /^/ <expr>* /$/ ;                            \
    ",
    Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lang);
}

void parser_free(void) {
  mpc_cleanup(8, Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lang);
}

lval* lval_read_str(char* input) {
  mpc_result_t r;
  lval* v;

  if (mpc_parse("<stdin>", input, Lang, &r)) {
    v = lval_read(r.output);
    mpc_ast_delete(r.output);
  } else {
    char* mess = mpc_err_string(r.error);
    v = lval_err(mess);
    mpc_err_delete(r.error);
    free(mess);
  }

  return v;
}
