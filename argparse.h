#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define STRING 0
#define INTEGER 1
#define BOOLEAN 2

typedef int ty;
typedef union {
  int i;
  char* s;
} ArgValue;

typedef struct t_arg_type {
  ty type;
  ArgValue value;
} ArgType;

typedef struct t_arg Arg;
struct t_arg {
  Arg* next;
  const char* name;
  const char* long_flag;
  char short_flag;
  ArgType data;
  int exists;
  int required;
  const char* description;
};

void parse_args(int nargs, char* kwargs[], const char* desc);
int add_integer_arg(const char* name, char* _short, const char* _long, int _default, int required, const char* desc);
int add_string_arg(const char* name, char* _short, const char* _long, char* _default, int required, const char* desc);
int add_boolean_arg(const char* name, char* _short, const char* _long, const char* desc);

char* get_string_arg(const char* name);
int get_boolean_arg(const char* name);
int get_integer_arg(const char* name);
void del_argparse();
