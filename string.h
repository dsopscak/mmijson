//  string.h
//
//  (c) 2019 Skip Sopscak
//  This code is licensed under MIT license (see LICENSE for details)
//
//  Public interface for a string allocation abstraction

#ifndef __mmijson_string_h
#define __mmijson_string_h

#ifndef __stddef_h
#define __stddef_h
#include <stddef.h>
#endif

#ifndef __json_h
#define __json_h
#include "json.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

char *get_json_string(JSON *, size_t);
void ret_json_string(JSON *, char *);

#ifdef __cplusplus
}
#endif

#endif // __mmijson_string_h
