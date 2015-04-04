#ifndef read_h
#define read_h

void reader_setup(void);
void reader_free(void);

enum {
  AST_ERR, AST_NUM, AST_DEC, AST_SYM, AST_FUN,
  AST_STR, AST_SEXPR, AST_QEXPR, AST_QQEXPR
};

struct AST {
  int type;

  // value
  char* str;
  long num;
  double dec;

  // childs
  int count;
  AST** cells;
} AST;

// constructors
AST* ast_str(int, char*);
AST* ast_num(long);
AST* ast_dec(double);

// operations
AST* ast_add(AST*, AST*);

// reader ops
AST* read_str(char*);

#endif
