// json.c

#include "json.h"
#include "string.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

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

static char *get_json_string_copy(const char *s)
    {
    char *copy = get_json_string(strlen(s));
    strcpy(copy, s);
    return copy;
    }

static DATA *create_data_node_string(const char *s)
    {
    DATA *data = malloc(sizeof(DATA));
    data->type = string;
    data->data.string = get_json_string_copy(s);
    return data;
    }

static DATA *create_data_node_number(char *s)
    {
    DATA *data = malloc(sizeof(DATA));
    data->type = number;
    data->data.string = get_json_string_copy(s);
    return data;
    }

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

static DATA *create_data_node_map()
    {
    DATA *data = malloc(sizeof(DATA));
    data->type = map;
    data->data.map = malloc(sizeof(MAP));
    data->data.map->head = NULL;
    return data;
    }

static void put_data_map(MAP *map, const char *key, DATA *data)
    {
    if (!map->head)
        {
        map->head = malloc(sizeof(MAP_NODE));
        map->head->key = get_json_string_copy(key);
        map->head->data = data;
        map->head->next = NULL;
        }
    else
        {
        MAP_NODE *p = map->head;
        while (true)
            {
            if (strcmp(p->key, key))
                {
                if (p->next)
                    p = p->next;
                else
                    {
                    p->next = malloc(sizeof(MAP_NODE));
                    p->next->key = get_json_string_copy(key);
                    p->next->data = data;
                    break;
                    }
                }
            else
                {
                p->data = data;
                break;
                }
            }
        }
    }


struct ARRAY
    {
    ARRAY_NODE *head;
    ARRAY_NODE *tail;
    };

static DATA *create_data_node_array(char *s)
    {
    DATA *data = malloc(sizeof(DATA));
    data->type = array;
    data->data.array = malloc(sizeof(ARRAY));
    data->data.array->head = NULL;
    data->data.array->tail = NULL;
    return data;
    }

static void put_data_array(ARRAY *array, DATA *data)
    {
    ARRAY_NODE *node = malloc(sizeof(ARRAY_NODE));
    node->data = data;
    node->next = NULL;
    if (array->head)
        {
        assert(!array->tail->next);
        array->tail->next = node;
        array->tail = node;
        }
    else
        array->head = array->tail = node;
    }

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

static void parse_into_array(FILE *f, ARRAY *array)
    {
    }

static void parse_into_map(FILE *f, MAP *map)
    {

    }

JSON *init_json_file(FILE *f)
    {
    int c;
    while ((c = fgetc(f)) != EOF)
        if (c == '{') break;
    if (c != '{')
        return NULL;

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

