//  json.h

#ifndef __mmijson_json_h
#define __mmijson_json_h

#ifndef __mmijson_stdio_h
#define __mmijson_stdio_h
#include <stdio.h>
#endif

typedef struct JSON JSON;

JSON *parse_json_file(FILE *);
void destroy_json(JSON *);

#endif
