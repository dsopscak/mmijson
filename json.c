// json.c

#include "json.h"

typedef struct MAP MAP;
typedef struct MAP_NODE MAP_NODE;
typedef struct ARRAY ARRAY;
typedef struct ARRAY_NODE ARRAY_NODE;

typedef struct DATA
    {
    enum { map, array, string, number } type;
    union 
        {
        char *string;
        MAP *map;
        ARRAY *array;
        } data;
    } DATA;

struct ARRAY_NODE
    {
    DATA *data;
    ARRAY_NODE *next;
    };

struct MAP_NODE
    {
    char *key;
    DATA *data;
    MAP_NODE *next;
    };

struct MAP
    {
    MAP_NODE *head;
    };


struct ARRAY
    {
    ARRAY_NODE *head;
    };


struct JSON
    {
    MAP *map;
    };

JSON *init_json_file(FILE *f)
    {

    return NULL;
    }
