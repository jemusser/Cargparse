#include "argparse.h"
#include <xinu.h>
#include <stdlib.h>

Arg* head = 0;
Arg* tail = 0;
const char* description;

char* l_ptr;

extern void _fdoprnt(char*, va_list, int (*)(did32, char), int);

void assert(int v, char* format, ...) {
  if (!v) {
    va_list args;
    syscall putc(did32, char);
    
    va_start(args, format);
    _fdoprnt((char*)format, args, putc, stdout);
    va_end(args);
    exit();
  }
}

Arg* _find_long(char* value) {
  Arg* tmparg = head;
  while (tmparg) {
    if (strncmp(value, tmparg->long_flag, strlen(value)) == 0) break;
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
  int i = index;
  while (tmparg) {
    if (!tmparg->long_flag && ! tmparg->short_flag) {
      if (i == 0) break;
      i -= 1;
    }
    tmparg = tmparg->next;
  }
  assert(tmparg != NULL, "Unknown positional argument - %d\n", index + 1);
  return tmparg;
}

void print_help() {
  Arg* tmparg = head;
  
  printf("\n%s\n\n", description);
  while (tmparg) {
    int llen = (tmparg->long_flag == NULL ? 0 : strlen(tmparg->long_flag));
    
    printf("  ");
    if (tmparg->short_flag) printf("-%c", tmparg->short_flag);
    else printf("  ");
    printf("  ");
    if (tmparg->long_flag) printf("--%s", tmparg->long_flag);
    else printf("  ");
    for (int i=0; i < 15 - llen; i++) printf(" ");

    switch (tmparg->data.type) {
    case INTEGER: printf(" [integer] "); break;
    case STRING:  printf("  [string] "); break;
    case BOOLEAN: printf(" [boolean] "); break;
    }
    
    printf("  %s %s\n", (tmparg->required ? "(REQUIRED)":""), tmparg->description);
    tmparg = tmparg->next;
  }
}

void init_args(const char* desc) {
  head = 0;
  tail = 0;
  description = desc;
}

void parse_args(int nargs, char* kwargs[]) {
  int nflagless = 0;
  
  for (int i = 1; i < nargs; i++) {
    char* arg = kwargs[i];
    if (strncmp(arg, "--help", strlen(arg)) == 0 || strncmp(arg, "-h", 2) == 0) {
      print_help();
      exit();
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

int add_argument(const char* name, const char* _short, char* _long, ArgValue _default, ty _type, int required, const char* desc) {
  Arg* newArg = (Arg*) getmem(sizeof(Arg));
  newArg->name = name;
  newArg->short_flag = (_short == NULL ? 0 : _short[0]);
  newArg->long_flag = _long;
  newArg->data.type = _type;
  newArg->data.value = _default;
  newArg->exists = (required ? 0 : 1);
  newArg->required = required;
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

int add_integer_arg(const char* name, char* _short, char* _long, int _default, int required, const char* desc) {
  ArgValue val;
  val.i = _default;
  return add_argument(name, _short, _long, val, INTEGER, required, desc);
}
int add_string_arg(const char* name, char* _short, char* _long, char* _default, int required, const char* desc) {
  ArgValue val;
  val.s = _default;
  return add_argument(name, _short, _long, val, STRING, required, desc);
}
int add_boolean_arg(const char* name, char* _short, char* _long, const char* desc) {
  ArgValue val;
  val.i = 0;
  return add_argument(name, _short, _long, val, BOOLEAN, 0, desc);
}

ArgValue get_argument(char* name) {
  Arg* tmphead = head;
  while (tmphead) {
    if (strncmp(tmphead->name, name, strlen(name)) == 0) {
      break;
      return tmphead->data.value;
    }
    tmphead = tmphead->next;
  }
  assert(tmphead != NULL, "Argument does not exist - %s\n", name);
  return tmphead->data.value;
}

char* get_string_arg(char* name) {
  return get_argument(name).s;
}
int get_boolean_arg(char* name) {
  return get_argument(name).i;
}
int get_integer_arg(char* name) {
  return get_argument(name).i;
}

void del_args() {
  Arg* tmphead = head;
  Arg* current;
  while (tmphead) {
    current = tmphead;
    tmphead = tmphead->next;
    freemem((char*)current, sizeof(Arg));
  }
}
