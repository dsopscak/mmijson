//  json.h

#ifndef __mmijson_json_h
#define __mmijson_json_h

#ifndef __mmijson_stdio_h
#define __mmijson_stdio_h
#include <stdio.h>
#endif

#ifndef __mmijson_stdbool_h
#define __mmijson_stdbool_h
#include <stdbool.h>
#endif

typedef struct JSON JSON;
typedef struct JSON_DATA JSON_DATA;

#ifdef _GNU_SOURCE
JSON *json_parse_string(const char *);
#endif

JSON *json_parse_file(FILE *);
void json_dump(JSON *, FILE *);
void json_destroy(JSON *);

JSON_DATA *json_get_root(JSON *);

bool json_is_null(JSON_DATA *);

bool json_is_string(JSON_DATA *);
const char *json_string(JSON_DATA *); // NULL if not string

bool json_is_number(JSON_DATA *);
double json_number(JSON_DATA *); // NaN if not number

bool json_is_boolean(JSON_DATA *);
bool json_boolean(JSON_DATA *); // false if not boolean :-(

bool json_is_object(JSON_DATA *);

bool json_is_array(JSON_DATA *);
JSON_DATA **json_array(JSON_DATA *); // NULL-terminated array

JSON_DATA *json_get_data(JSON_DATA *, const char *query_string); 
// nestable query with comma-separated keys/indicies, starting at
// given data node. Return NULL if nothing found.

#endif
