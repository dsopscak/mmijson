/* string.c
*/
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
