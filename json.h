//  json.h

#ifndef __mmijson_json_h
#define __mmijson_json_h

#ifndef __mmijson_stdio_h
#define __mmijson_stdio_h
#include <stdio.h>
#endif

typedef struct JSON JSON;

JSON *init_json_file(FILE *);

#endif
