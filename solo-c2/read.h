#ifndef read_h
#define read_h
#include "mpc.h"

void reader_setup(void);
void reader_free(void);

enum {
  AST_ERR, AST_NUM, AST_DEC, AST_SYM, AST_FUN,
  AST_STR, AST_SEXPR
};

struct AST;
typedef struct AST AST;

struct AST {
  int type;

  // value
  char* str;
  long num;
  double dec;

  // childs
  int count;
  AST** cells;
};

// constructors
AST* ast_str(int, char*);
AST* ast_num(long);
AST* ast_dec(double);
AST* ast_list(int);

// operations
AST* ast_add(AST*, AST*);

// reader ops
AST* read_str(char*);

// conversion from raw mpc value
AST* mpcv_as_long(mpc_ast_t*);
AST* mpcv_as_double(mpc_ast_t*);
AST* mpcv_as_string(mpc_ast_t*);

#endif
