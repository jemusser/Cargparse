#include "argparse.h"

Arg* head = 0;
Arg* tail = 0;

char* l_ptr;

void assert(int v, char* format, ...) {
  if (!v) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    exit(-1);
  }
}

Arg* _find_long(const char* value) {
  Arg* tmparg = head;
  while (tmparg) {
    if (strcmp(value, tmparg->long_flag) == 0) break;
    tmparg = tmparg->next;
  }
  assert(tmparg != NULL, "Unknown flag - %s\n", value);
  return tmparg;
}

Arg* _find_short(char value) {
  Arg* tmparg = head;
  while (tmparg) {
    if (value == tmparg->short_flag) break;
    tmparg = tmparg->next;
  }
  assert(tmparg != NULL, "Unknown flag - %c\n", value);
  return tmparg;
}

Arg* _find_flagless(int index) {
  Arg* tmparg = head;
  while (tmparg) {
    if (!tmparg->long_flag && ! tmparg->short_flag) {
      if (index == 0) break;
      index -= 1;
    }
    tmparg = tmparg->next;
  }
  assert(tmparg != NULL, "Unknown positional argument - %d\n", index);
  return tmparg;
}

void print_help() {
  Arg* tmparg = head;
  while (tmparg) {
    int llen = strlen(tmparg->long_flag);
    int slen = (tmparg->short_flag ? 1 : 0);
    
    for (int i=0; i < 1 - slen; i++) printf(" ");
    printf("-%c", tmparg->short_flag);
    for (int i=0; i < 15 - llen; i++) printf(" ");
    printf("--%s    %s\n", tmparg->long_flag, tmparg->desc);
    tmparg = tmparg->next;
  }
}

void parse_args(int nargs, char* kwargs[]) {
  int nflagless = 0;
  
  for (int i = 1; i < nargs; i++) {
    char* arg = kwargs[i];
    if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
      print_help();
      exit(0);
    }
    if (strncmp(arg, "--", 2) == 0) {
      /* long name */
      arg += 2;
      Arg* a = _find_long(arg);
      if (a) {
	if (a->data.type == BOOLEAN) a->data.value.i = 1;
	else {
	  i += 1;
	  assert(i < nargs && kwargs[i][0] != '-', "Value required for flag - %s\n", a->name);
	  if (a->data.type == INTEGER) a->data.value.i = atoi(kwargs[i]);
	  else a->data.value.s = kwargs[i];
	}
	a->exists = 1;
      }
    }
    else if (arg[0] == '-') {
      /* short name */
      arg += 1;
      int len = strlen(arg);
      for (int j=0; j < len; j++) {
	Arg* a = _find_short(arg[j]);
	if (a) {
	  if (a->data.type == BOOLEAN) a->data.value.i = 1;
	  else {
	    i += 1;
	    assert(i < nargs && j == len - 1 && kwargs[i][0] != '-', "Value required for flag - %s\n", a->name);
	    if (a->data.type == INTEGER) a->data.value.i = atoi(kwargs[i]);
	    else a->data.value.s = kwargs[i];
	  }
	  a->exists = 1;
	}
      }
    }
    else {
      /* unnamed arguments */
      Arg* a = _find_flagless(nflagless);
      nflagless += 1;
      if (a->data.type == STRING) a->data.value.s = arg;
      else a->data.value.i = atoi(arg);
      a->exists = 1;
    }
  }
  
  Arg* tmparg = head;
  while (tmparg) {
    assert(tmparg->exists > 0, "%s is required\n", tmparg->name);
    tmparg = tmparg->next;
  }
}

int add_argument(const char* name, const char* _short, const char* _long, ArgValue _default, ty _type, int required, const char* desc) {
  Arg* newArg = (Arg*) malloc(sizeof(Arg));
  newArg->name = name;
  newArg->short_flag = (_short == NULL ? 0 : _short[0]);
  newArg->long_flag = _long;
  newArg->data.type = _type;
  newArg->data.value = _default;
  newArg->exists = (required ? 0 : 1);
  newArg->description = desc;
  newArg->next = NULL;
  
  if (tail) {
    tail->next = newArg;
    tail = newArg;
  }
  else {
    head = newArg;
    tail = newArg;
  }
  
  return 1;
}

int add_integer_arg(const char* name, char* _short, const char* _long, int _default, int required, const char* desc) {
  ArgValue val;
  val.i = _default;
  return add_argument(name, _short, _long, val, INTEGER, required, desc);
}
int add_string_arg(const char* name, char* _short, const char* _long, char* _default, int required, const char* desc) {
  ArgValue val;
  val.s = _default;
  return add_argument(name, _short, _long, val, STRING, required, desc);
}
int add_boolean_arg(const char* name, char* _short, const char* _long, const char* desc) {
  ArgValue val;
  val.i = 0;
  return add_argument(name, _short, _long, val, BOOLEAN, 0, desc);
}

ArgValue get_argument(const char* name) {
  Arg* tmphead = head;
  while (tmphead) {
    if (strcmp(tmphead->name, name) == 0) {
      break;
      return tmphead->data.value;
    }
    tmphead = tmphead->next;
  }
  assert(tmphead != NULL, "Argument does not exist - %s\n", name);
  return tmphead->data.value;
}

char* get_string_arg(const char* name) {
  return get_argument(name).s;
}
int get_boolean_arg(const char* name) {
  return get_argument(name).i;
}
int get_integer_arg(const char* name) {
  return get_argument(name).i;
}

void del_argparse() {
  Arg* tmphead = head;
  Arg* current;
  while (tmphead) {
    current = tmphead;
    tmphead = tmphead->next;
    free(current);
  }
}
