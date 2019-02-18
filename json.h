//  json.h
//
//  (c) 2019 Skip Sopscak
//  This code is licensed under MIT license (see LICENSE for details)
//
//  Public interface for a simple json parser

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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JSON JSON;
typedef struct JSON_DATA JSON_DATA;

JSON *json_parse_string(char *, bool should_free);
// Parse the string into a JSON structure and return pointer to same.
// Control of the string is surrendered and contents will be altered
// by the call. Any subsequent changes to the string by the caller may
// invalidate the JSON object and results are undefined. The string
// will be freed on object destruction baseed on the value of
// should_free. Returns NULL if the string isn't valid JSON, but
// contents may still have been altered.

JSON *json_parse_file(FILE *);

void json_dump(JSON *, FILE *);
// JSON * must have been returned by one of the parse methods above.

void json_destroy(JSON *);
// JSON * must have been returned by one of the parse methods above.
// Releases all resources used by the JSON object, rendering it
// unusable.

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
// Nestable query with comma-separated keys/indicies, starting at
// given data node. Return NULL if nothing found.
//
// Example: "foo,7,bar"
//
// would find the boolean true object in the following json:
//
// { "foo": [ 0, 1, 2, 3, 4, 5, 6, { "bar": true }]}"

#ifdef __cplusplus
}
#endif

#endif
