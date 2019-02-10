//  string.c
//
//  (c) 2019 Skip Sopscak
//  This code is licensed under MIT license (see LICENSE for details)

#include "string.h"
#include <stdlib.h>

char *get_json_string(size_t length)
  {
  return malloc(length + 1);
  }

void ret_json_string(char *s)
  {
  free(s);
  }
