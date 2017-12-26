// json.c

#include "json.h"

typedef struct JSON_MAP
    {
    } JSON_MAP;

typedef struct JSON_ARRAY
    {
    } JSON_ARRAY;

typedef struct DATA
{
  enum { map, array, string, number } type;
  union 
      {
      char *string;
      JSON_MAP *map;
      JSON_ARRAY *array;
      } data;
} DATA;

struct JSON
{

};

JSON *init_json_file(FILE *f)
{

  return NULL;
}
