#ifndef __mmijson_string_h
#define __mmijson_string_h

#ifndef __stddef_h
#define __stddef_h
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

char *get_json_string(size_t);
void ret_json_string(char *);

#ifdef __cplusplus
}
#endif

#endif // __mmijson_string_h
