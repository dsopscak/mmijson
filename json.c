// json.c

#include "json.h"
#include "string.h"
#include <stdlib.h>

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
    char *work_buffer;
    size_t work_buffer_size;
    };

static JSON *create_json(void)
    {
    JSON *json = malloc(sizeof(JSON));
    json->map = malloc(sizeof(MAP));
    json->map->head = NULL;
    json->work_buffer = malloc(1024);
    json->work_buffer_size = 1024;

    return json;
    }

static void parse_into_map(FILE *f, MAP *map)
    {
    }

JSON *init_json_file(FILE *f)
    {
    JSON *json = create_json();

    parse_into_map(f, json->map);

    return json;
    }

static void recurse_and_destroy_map(MAP *);
static void recurse_and_destroy_array(ARRAY *);

static void recurse_and_destroy_data(DATA *doomed)
    {
    if (doomed)
        {
        if (doomed->type == map)
            recurse_and_destroy_map(doomed->data.map);
        else if (doomed->type == array)
            recurse_and_destroy_array(doomed->data.array);
        else
            ret_json_string(doomed->data.string);
        free(doomed);
        }
    }

static void recurse_and_destroy_array_node(ARRAY_NODE *doomed)
    {
    if (doomed)
        {
        recurse_and_destroy_array_node(doomed->next);
        recurse_and_destroy_data(doomed->data);
        free(doomed);
        }
    }

static void recurse_and_destroy_array(ARRAY *doomed)
    {
    if (doomed)
        {
        recurse_and_destroy_array_node(doomed->head);
        free(doomed);
        }
    }

static void recurse_and_destroy_map_node(MAP_NODE *doomed)
    {
    if (doomed)
        {
        recurse_and_destroy_map_node(doomed->next);
        recurse_and_destroy_data(doomed->data);
        ret_json_string(doomed->key);
        free(doomed);
        }
    }

static void recurse_and_destroy_map(MAP *doomed)
    {
    if (doomed)
        {
        recurse_and_destroy_map_node(doomed->head);
        free(doomed);
        }
    }


void destroy_json(JSON *doomed)
    {
    recurse_and_destroy_map(doomed->map);
    free(doomed->work_buffer);
    free(doomed);
    }

