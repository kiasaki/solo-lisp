#include <stdlib.h>
#include "solo.h"

// Construct an new environment
lenv* lenv_new(void) {
  lenv* e = malloc(sizeof(lenv));
  e->parent = NULL;
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

lenv* lenv_copy(lenv* e) {
  lenv* ne = malloc(sizeof(lenv));
  ne->parent = e->parent;
  ne->count = e->count;

  ne->syms = malloc(sizeof(char*) * e->count);
  ne->vals = malloc(sizeof(lval*) * e->count);
  for (int i = 0; i < e->count; i++) {
    ne->syms[i] = malloc(strlen(e->syms[i]) + 1);
    strcpy(ne->syms[i], e->syms[i]);
    ne->vals[i] = lval_copy(e->vals[i]);
  }

  return ne;
}

lval* lenv_get(lenv* e, lval* k) {
  for (int i = 0; i < e->count; i++) {
    // If we found an existing values for that key (sym) return a copy
    if (strcmp(e->syms[i], k->sym) == 0) {
      return lval_copy(e->vals[i]);
    }
  }
  // check for parent, of not, no match :(
  if (e->parent) {
    return lenv_get(e->parent, k);
  } else {
    return lval_err("Unbound symbol '%s'", k->sym);
  }
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

void lenv_def(lenv* e, lval* k, lval* v) {
  // find top most parent
  while (e->parent) { e = e->parent; }
  // store it at this level
  lenv_put(e, k, v);
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
  lval* k = lval_sym(name);
  lval* v = lval_builtin(func);
  lenv_put(e, k, v);
  lval_delete(k); lval_delete(v);
}

void lenv_add_builtins(lenv* e) {
  // list
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "join", builtin_join);

  // mathematical
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);

  // variables
  lenv_add_builtin(e, "def", builtin_def);
  lenv_add_builtin(e, "let", builtin_let);
  lenv_add_builtin(e, "fn",  builtin_lambda);

  // comparison
  lenv_add_builtin(e, "if", builtin_if);
  lenv_add_builtin(e, "==", builtin_eq);
  lenv_add_builtin(e, "!=", builtin_ne);
  lenv_add_builtin(e, ">",  builtin_gt);
  lenv_add_builtin(e, "<",  builtin_lt);
  lenv_add_builtin(e, ">=", builtin_ge);
  lenv_add_builtin(e, "<=", builtin_le);

  // misc
  lenv_add_builtin(e, "load",  builtin_load);
  lenv_add_builtin(e, "print", builtin_print);
  lenv_add_builtin(e, "error", builtin_error);
  lenv_add_builtin(e, "type", builtin_type);
}
