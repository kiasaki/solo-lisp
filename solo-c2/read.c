#include <stdlib.h>
#include <stdbool.h>
#include "mpc.h"
#include "read.h"

mpc_parser_t* Number;
mpc_parser_t* Decimal;
mpc_parser_t* Symbol;
mpc_parser_t* String;
mpc_parser_t* Comment;
mpc_parser_t* Sexpr;
mpc_parser_t* Qexpr;
mpc_parser_t* Qqexpr;
mpc_parser_t* Expr;
mpc_parser_t* Lang;

// constructors
AST* ast_str(int type, char* s) {
  AST* a = malloc(sizeof(AST));
  a->type = type;
  a->str = malloc(strlen(s) + 1);
  strcpy(a->str, s);
  return a;
}

AST* ast_num(long n) {
  AST* a = malloc(sizeof(AST));
  a->type = AST_NUM;
  a->num = n;
  return a;
}

AST* ast_dec(double d) {
  AST* a = malloc(sizeof(AST));
  a->type = AST_DEC;
  a->dec = d;
  return a;
}

AST* ast_list(int type) {
  AST* a = malloc(sizeof(AST));
  a->type = type;
  a->count = 0;
  a->cells = NULL;
  return a;
}

// operations
AST* ast_add(AST* a, AST* d) {
  a->count++;
  a->cells = realloc(a->cells, sizeof(AST*) * a->count);
  a->cells[a->count-1] = d;
  return a;
}

AST* read_tree(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) { return mpcv_as_long(t); }
  if (strstr(t->tag, "decimal")) { return mpcv_as_double(t); }
  if (strstr(t->tag, "symbol")) { return ast_str(AST_SYM, t->contents); }
  if (strstr(t->tag, "string")) { return mpcv_as_string(t); }

  // other types can only be a list
  int write_to_first_cell = false;
  AST* result = ast_list(AST_SEXPR);

  // is ' or ` was encountered, prepend coresponding quoting fn
  if (strstr(t->tag, "qexpr"))  {
    result = ast_add(result, ast_str(AST_SYM, "quote"));
    if (t->children_num > 1) {
      result = ast_add(result, ast_list(AST_SEXPR));
      write_to_first_cell = true;
    }
  }
  if (strstr(t->tag, "qqexpr"))  {
    result = ast_add(result, ast_str(AST_SYM, "quasiquote"));
    if (t->children_num > 1) {
      result = ast_add(result, ast_list(AST_SEXPR));
      write_to_first_cell = true;
    }
  }

  // loop in childrens, only append actual datums
  for (int i = 0; i < t->children_num; i++) {
    if (
      strstr(t->children[i]->tag, "number")
      || strstr(t->children[i]->tag, "decimal")
      || strstr(t->children[i]->tag, "symbol")
      || strstr(t->children[i]->tag, "string")
      || strstr(t->children[i]->tag, "sexpr")
      || strstr(t->children[i]->tag, "qexpr")
      || strstr(t->children[i]->tag, "qqexpr")
    ) {
      if (write_to_first_cell) {
        result->cells[1] = ast_add(result->cells[1],
          read_tree(t->children[i]));
      } else {
        result = ast_add(result, read_tree(t->children[i]));
      }
    }
  }

  return result;
}

AST* read_str(char* input) {
  AST* result;
  mpc_result_t r;

  if (mpc_parse("<stdin>", input, Lang, &r)) {
    mpc_ast_print(r.output);
    result = read_tree(r.output);
    mpc_ast_delete(r.output);
  } else {
    char* mess = mpc_err_string(r.error);
    result = ast_str(AST_ERR, mess);
    mpc_err_delete(r.error);
    free(mess);
  }

  return result;
}

void reader_setup() {
  Number  = mpc_new("number");
  Decimal = mpc_new("decimal");
  Symbol  = mpc_new("symbol");
  String  = mpc_new("string");
  Comment = mpc_new("comment");
  Sexpr   = mpc_new("sexpr");
  Qexpr   = mpc_new("qexpr");
  Qqexpr  = mpc_new("qqexpr");
  Expr    = mpc_new("expr");
  Lang    = mpc_new("lang");

  mpca_lang(MPCA_LANG_DEFAULT,
    "                                              \
      number  : /-?[0-9]+/ ;                       \
      decimal : /-?[0-9]+\\.[0-9]+/ ;              \
      symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ; \
      string  : /\"(\\\\.|[^\"])*\"/ ;             \
      comment : /;[^\\r\\n]*/ ;                    \
      sexpr   : '(' <expr>* ')'                    \
              | '{' <expr>* '}'                    \
              | '[' <expr>* ']' ;                  \
      qexpr   : '\'' <expr> ;                      \
      qqexpr  : '`' <expr> ;                       \
      expr    : <number> | <double> | <symbol>     \
              | <string> | <sexpr>  | <qexpr>      \
              | <qqexpr> | <comment> ;             \
      lang    : /^/ <expr>* /$/ ;                  \
    ",
    Number, Decimal, Symbol, String, Comment, Sexpr, Qexpr, Qqexpr, Expr, Lang);
}

void reader_free(void) {
  mpc_cleanup(10, Number, Decimal, Symbol, String, Comment, Sexpr, Qexpr, Qqexpr, Expr, Lang);
}

AST* mpcv_as_long(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    ast_num(x) : ast_str(AST_ERR, "invalid number");
}

AST* mpcv_as_double(mpc_ast_t* t) {
  errno = 0;
  double x = strtol(t->contents, NULL, 10);
  return (errno != ERANGE && errno != EINVAL) ?
    ast_dec(x) : ast_str(AST_ERR, "invalid decimal");
}

AST* mpcv_as_string(mpc_ast_t* t) {
  // cut final char, copy omiting the first " char
  t->contents[strlen(t->contents)-1] = '\0';
  char* unescaped = malloc(strlen(t->contents+1)+1);
  strcpy(unescaped, t->contents+1);

  unescaped = mpcf_unescape(unescaped);
  AST* str = ast_str(AST_STR, unescaped);

  // free the string and return
  free(unescaped);
  return str;
}
