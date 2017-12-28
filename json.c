// json.c

#include "json.h"
#include "string.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>

#define BUF_INC 1024

typedef struct MAP MAP;
typedef struct MAP_NODE MAP_NODE;
typedef struct ARRAY ARRAY;
typedef struct ARRAY_NODE ARRAY_NODE;

static void fatal(const char *msg)
    {
    fprintf(stderr, "Fatal JSON error: %s\n", msg);
    exit(-1);
    }

typedef struct DATA
    {
    enum { map, array, string, number, boolean, null } type;
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

static DATA *create_data_boolean(int b)
    {
    DATA *data = malloc(sizeof(DATA));
    data->type = boolean;
    data->data.string = get_json_string_copy(b ? "true" : "false");
    return data;
    }

 static DATA *create_data_null()
    {
    DATA *data = malloc(sizeof(DATA));
    data->type = null;
    data->data.string = get_json_string_copy("null");
    return data;
    }

static DATA *create_data_string(const char *s)
    {
    DATA *data = malloc(sizeof(DATA));
    data->type = string;
    data->data.string = get_json_string_copy(s);
    return data;
    }

static DATA *create_data_number(char *s)
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

static DATA *create_data_map()
    {
    DATA *data = malloc(sizeof(DATA));
    data->type = map;
    data->data.map = malloc(sizeof(MAP));
    data->data.map->head = NULL;
    return data;
    }

static int skip_whitespace(FILE *f)
    {
    char c;
    while ((isspace(c = fgetc(f))))
        ;
    return c;
    }

static void put_data_map(MAP *map, char *key, DATA *data)
    {
    if (!map->head)
        {
        map->head = malloc(sizeof(MAP_NODE));
        map->head->key = key;
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
                    p->next->key = key;
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

static DATA *create_data_array()
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
    DATA *data;
    char *work_buffer;
    char *p;
    size_t work_buffer_size;
    size_t i;
    };

static JSON *create_json(void)
    {
    JSON *json = malloc(sizeof(JSON));
    json->data = NULL;
    json->work_buffer = malloc(BUF_INC);
    json->work_buffer_size = BUF_INC;

    return json;
    }

static void init_work_buffer(JSON *json)
    {
    json->p = json->work_buffer;
    json->i = 0;
    }

static void putc_work_buffer(JSON *json, char c)
    {
    if (json->i == json->work_buffer_size)
        {
        json->work_buffer_size += BUF_INC;
        json->work_buffer = realloc(json->work_buffer, json->work_buffer_size);
        json->p = json->work_buffer + json->i;
        }
    *json->p++ = c;
    ++json->i;
    }

static char *parse_string(FILE *f, JSON *json)
    {
    int c;
    init_work_buffer(json);
    while ((c = fgetc(f)) != EOF)
        {
        if (c == '\\')
            {
            putc_work_buffer(json, c);
            c = fgetc(f);
            }
        else if (c == '"')
            break;

        putc_work_buffer(json, c);
        }

    if (c != '"')
        fatal("EOF before end of string");

    putc_work_buffer(json, 0);
    return json->work_buffer;
    }

static char *parse_number(FILE *f, int c, JSON *json)
    {
    init_work_buffer(json);
    bool is_dotted = false;
    if (c == '-' || isdigit(c))
        {
        putc_work_buffer(json, c);
        while ((c = fgetc(f)) != EOF)
            {
            if (c == '.')
                {
                if (is_dotted)
                    fatal("invalid number");
                else
                    is_dotted = true;
                }
            else if (!isdigit(c))
                {
                ungetc(c, f);
                break;
                }
            putc_work_buffer(json, c);
            }
        putc_work_buffer(json, 0);
        return json->work_buffer;
        }
    fatal("invalid number");
    return NULL;
    }

static int parse_boolean(FILE *f)
    {
    int c;
    if ((c =fgetc(f)) == 'r')
        if (fgetc(f) == 'u')
            if (fgetc(f) == 'e')
                return 1;
    if (c == 'a')
        if (fgetc(f) == 'l')
            if (fgetc(f) == 's')
                if (fgetc(f) == 'e')
                    return 0;
    fatal("invalid boolean value");
    return -1;
    }

static void parse_null(FILE *f)
    {
    if (fgetc(f) == 'u')
        if (fgetc(f) == 'l')
            if (fgetc(f) == 'l')
                return;
    fatal("invalid null value");
    }


static void parse_into_map(FILE *f, MAP *map, JSON *json);

static void parse_into_array(FILE *f, ARRAY *array, JSON *json)
    {
    int c = skip_whitespace(f);
    DATA *data;
    switch (c)
        {
    case ']':
        return;
    case '{':
        data = create_data_map();
        parse_into_map(f, data->data.map, json);
        break;
    case '[':
        data = create_data_array();
        parse_into_array(f, data->data.array, json);
        break;
    case '"':
        data = create_data_string(parse_string(f, json));
        break;
    case 't':
    case 'f':
        data = create_data_boolean(parse_boolean(f));
        break;
    case 'n':
        parse_null(f);
        data = create_data_null();
        break;
    default:
        data = create_data_number(parse_number(f, c, json));
        }
    put_data_array(array, data);
    c = skip_whitespace(f);
    if (c == ',')
        parse_into_array(f, array, json); // recursion
    else if (c != ']')
        fatal("invalid array");
    }

static void parse_into_map(FILE *f, MAP *map, JSON *json)
    {
    int c = skip_whitespace(f);
    if (c == '}')
        return;
    else if (c != '"')
        fatal("string expected");

    char *key = get_json_string_copy(parse_string(f, json));

    c = skip_whitespace(f);
    if (c != ':')
        fatal("invalid map-pair");
    c = skip_whitespace(f);

    DATA *data;
    switch (c)
        {
    case '{':
        data = create_data_map();
        parse_into_map(f, data->data.map, json);
        break;
    case '[':
        data = create_data_array();
        parse_into_array(f, data->data.array, json);
        break;
    case '"':
        data = create_data_string(parse_string(f, json));
        break;
    case 't':
    case 'f':
        data = create_data_boolean(parse_boolean(f));
        break;
    case 'n':
        parse_null(f);
        data = create_data_null();
        break;
    default:
        data = create_data_number(parse_number(f, c, json));
        }
    put_data_map(map, key, data);
    c = skip_whitespace(f);
    if (c == ',')
        parse_into_map(f, map, json); // recursion
    else if (c != '}')
        fatal("invalid map");
    }


JSON *init_json_file(FILE *f)
    {
    JSON *json = create_json();
    int c = skip_whitespace(f);
    switch (c)
        {
    case '{':
        json->data = create_data_map();
        parse_into_map(f, json->data->data.map, json);
        break;
    case '[':
        json->data = create_data_array();
        parse_into_array(f, json->data->data.array, json);
        break;
    case '"':
        json->data = create_data_string(parse_string(f, json));
        break;
    case 't':
    case 'f':
        json->data = create_data_boolean(parse_boolean(f));
        break;
    case 'n':
        parse_null(f);
        json->data = create_data_null();
        break;
    default:
        json->data = create_data_number(parse_number(f, c, json));
        }

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
    recurse_and_destroy_data(doomed->data);
    free(doomed->work_buffer);
    free(doomed);
    }
