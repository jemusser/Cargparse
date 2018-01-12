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
  char* long_flag;
  char short_flag;
  ArgType data;
  int exists;
  int required;
  const char* description;
};

void init_args(const char* desc);
void parse_args(int nargs, char* kwargs[]);
int add_integer_arg(const char* name, char* _short, char* _long, int _default, int required, const char* desc);
int add_string_arg(const char* name, char* _short, char* _long, char* _default, int required, const char* desc);
int add_boolean_arg(const char* name, char* _short, char* _long, const char* desc);

char* get_string_arg(char* name);
int get_boolean_arg(char* name);
int get_integer_arg(char* name);
void del_args();
