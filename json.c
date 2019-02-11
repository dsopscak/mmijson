//  json.c
//
//  (c) 2019 Skip Sopscak
//  This code is licensed under MIT license (see LICENSE for details)

#include "json.h"
#include "string.h"
#include "pool.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h> // NAN

#define BUF_INC 1024
#define ARRAY_INC 2 
#define QUERY_DELIM ','

typedef struct MAP_NODE MAP_NODE;
typedef struct ARRAY ARRAY;

static void fatal(const char *msg)
    {
    fprintf(stderr, "Fatal JSON error: %s\n", msg);
    exit(-1);
    }

struct JSON_DATA
    {
    enum { map, array, string, number, boolean, null } type;
    union
        {
        char *string;
        MAP_NODE *map;
        ARRAY *array;
        } data;
    };

struct JSON
    {
    Pool *data_pool;
    Pool *map_pool;
    Pool *array_pool;
    JSON_DATA *data;
    char *work_buffer;
    char *p;
    size_t work_buffer_size;
    size_t i;
    };

static char *get_json_string_copy(JSON *json, const char *s)
    {
    char *copy = get_json_string(json, strlen(s));
    strcpy(copy, s);
    return copy;
    }

static JSON_DATA *create_data_boolean(JSON *json, int b)
    {
    JSON_DATA *data = PoolAlloc(json->data_pool);
    data->type = boolean;
    data->data.string = get_json_string_copy(json, b ? "true" : "false");
    return data;
    }

static JSON_DATA *create_data_null(JSON *json)
    {
    JSON_DATA *data = PoolAlloc(json->data_pool);
    data->type = null;
    data->data.string = get_json_string_copy(json, "null");
    return data;
    }

static JSON_DATA *create_data_string(JSON *json, const char *s)
    {
    JSON_DATA *data = PoolAlloc(json->data_pool);
    data->type = string;
    data->data.string = get_json_string_copy(json, s);
    return data;
    }

static JSON_DATA *create_data_number(JSON *json, char *s)
    {
    JSON_DATA *data = PoolAlloc(json->data_pool);
    data->type = number;
    data->data.string = get_json_string_copy(json, s);
    return data;
    }

struct MAP_NODE
    {
    char *key;
    JSON_DATA *data;
    MAP_NODE *next;
    };

static JSON_DATA *create_data_map(JSON *json)
    {
    JSON_DATA *data = PoolAlloc(json->data_pool);
    data->type = map;
    data->data.map = NULL;
    return data;
    }

static int skip_whitespace(FILE *f)
    {
    char c;
    while ((isspace(c = fgetc(f))))
        ;
    return c;
    }

static void put_data_map(JSON *json, char *key, JSON_DATA *data)
    {
    if (!json->data->data.map)
        {
        json->data->data.map = PoolAlloc(json->map_pool);
        json->data->data.map->key = key;
        json->data->data.map->data = data;
        json->data->data.map->next = NULL;
        }
    else
        {
        MAP_NODE *p = json->data->data.map;
        while (true)
            {
            if (strcmp(p->key, key))
                {
                if (p->next)
                    p = p->next;
                else
                    {
                    p->next = PoolAlloc(json->map_pool);
                    p->next->key = key;
                    p->next->data = data;
                    p->next->next = NULL;
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
    JSON_DATA **array;
    size_t size;
    size_t next;
    };

static JSON_DATA *create_data_array(JSON *json)
    {
    JSON_DATA *data = PoolAlloc(json->data_pool);
    data->type = array;
    data->data.array = PoolAlloc(json->array_pool);
    data->data.array->size = ARRAY_INC;
    data->data.array->next = 0;
    data->data.array->array = 
        calloc(data->data.array->size, sizeof(JSON_DATA *));
    return data;
    }

static void put_data_array(ARRAY *array, JSON_DATA *data)
    {
    array->array[array->next] = data;
    if (array->size == ++array->next)
        {
        array->size += ARRAY_INC;
        array->array = realloc(array->array, array->size * sizeof(JSON_DATA *));
        for (int i = array->next; i < array->size; ++i)
            array->array[i] = NULL;
        }
    }

static JSON *create_json(void)
    {
    JSON *json = malloc(sizeof(JSON));
    json->data_pool = PoolCreate(sizeof(JSON_DATA));
    json->map_pool = PoolCreate(sizeof(MAP_NODE));
    json->array_pool = PoolCreate(sizeof(ARRAY));
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
    ++(json->i);
    }

static char *parse_string(FILE *f, JSON *json)
    {
    int c;
    init_work_buffer(json);
    while ((c = fgetc(f)) != EOF)
        {
        if (c == '\\')
            {
            c = fgetc(f);
            switch (c)
                {
            case '"':
            case '/':
            case '\\':
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            default:
                fatal("invalid escape sequence");
                break;
                }
            }
        else if (c == '\n' || iscntrl(c))
            fatal("invalid string");
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


static void parse_into_map(FILE *f, MAP_NODE *map, JSON *json);

static void parse_into_array(FILE *f, ARRAY *array, JSON *json)
    {
    int c = skip_whitespace(f);
    JSON_DATA *data;
    switch (c)
        {
    case ']':
        if (array->next > 0)
            fatal("unexpected end of array");
        else
            return;
    case '{':
        data = create_data_map(json);
        parse_into_map(f, data->data.map, json);
        break;
    case '[':
        data = create_data_array(json);
        parse_into_array(f, data->data.array, json);
        break;
    case '"':
        data = create_data_string(json, parse_string(f, json));
        break;
    case 't':
    case 'f':
        data = create_data_boolean(json, parse_boolean(f));
        break;
    case 'n':
        parse_null(f);
        data = create_data_null(json);
        break;
    default:
        data = create_data_number(json, parse_number(f, c, json));
        }
    put_data_array(array, data);
    c = skip_whitespace(f);
    if (c == ',')
        parse_into_array(f, array, json); // recursion
    else if (c != ']')
        fatal("invalid array");
    }

static void parse_into_map(FILE *f, MAP_NODE *map, JSON *json)
    {
    int c = skip_whitespace(f);
    if (c == '}')
        {
        if (map)
            fatal("unexpected end of map");
        else
            return;
        }
    else if (c != '"')
        fatal("string expected");

    char *key = get_json_string_copy(json, parse_string(f, json));

    c = skip_whitespace(f);
    if (c != ':')
        fatal("invalid map-pair");
    c = skip_whitespace(f);

    JSON_DATA *data;
    switch (c)
        {
    case '{':
        data = create_data_map(json);
        parse_into_map(f, data->data.map, json);
        break;
    case '[':
        data = create_data_array(json);
        parse_into_array(f, data->data.array, json);
        break;
    case '"':
        data = create_data_string(json, parse_string(f, json));
        break;
    case 't':
    case 'f':
        data = create_data_boolean(json, parse_boolean(f));
        break;
    case 'n':
        parse_null(f);
        data = create_data_null(json);
        break;
    default:
        data = create_data_number(json, parse_number(f, c, json));
        }
    put_data_map(json, key, data);
    c = skip_whitespace(f);
    if (c == ',')
        parse_into_map(f, map, json); // recursion
    else if (c != '}')
        fatal("invalid map");
    }


static void recurse_and_destroy_map_node(JSON *, MAP_NODE *);
static void recurse_and_destroy_array(JSON *, ARRAY *);

static void recurse_and_destroy_data(JSON *json, JSON_DATA *doomed)
    {
    if (doomed)
        {
        if (doomed->type == map)
            recurse_and_destroy_map_node(json, doomed->data.map);
        else if (doomed->type == array)
            recurse_and_destroy_array(json, doomed->data.array);
        else
            ret_json_string(json, doomed->data.string);
        }
    }


static void recurse_and_destroy_array(JSON *json, ARRAY *doomed)
    {
    if (doomed)
        {
        for (int i = 0; i < doomed->next; ++i)
            recurse_and_destroy_data(json, doomed->array[i]);
        free(doomed->array);
        }
    }

static void recurse_and_destroy_map_node(JSON *json, MAP_NODE *doomed)
    {
    if (doomed)
        {
        recurse_and_destroy_map_node(json, doomed->next);
        recurse_and_destroy_data(json, doomed->data);
        ret_json_string(json, doomed->key);
        //PoolFree(json->map_pool, doomed);
        }
    }


static void dump_string(const char *string, FILE *f)
    {
    fputc('"', f);
    char const *p = string;
    while (*p)
        {
        switch (*p)
            {
        case '"':
            fprintf(f, "\\\"");
            break;
        case '\\':
            fprintf(f, "\\\\");
            break;
        case '\b':
            fprintf(f, "\\b");
            break;
        case '\f':
            fprintf(f, "\\f");
            break;
        case '\n':
            fprintf(f, "\\n");
            break;
        case '\r':
            fprintf(f, "\\r");
            break;
        case '\t':
            fprintf(f, "\\t");
            break;
        default:
            fputc(*p, f);
            break;
            }
        ++p; 
        }
    fputc('"', f);
    }

static void dump_map(MAP_NODE *, FILE *);
static void dump_array(ARRAY *, FILE *);

static void dump_data(JSON_DATA *data, FILE *f)
    {
    if (data)
        {
        switch (data->type)
            {
        case map:
            dump_map(data->data.map, f);
            break;
        case array:
            dump_array(data->data.array, f);
            break;
        case string:
            dump_string(data->data.string, f);
            break;
        default:
            fprintf(f, "%s", data->data.string);
            break;
            }
        }
    }


static void dump_array(ARRAY *array, FILE *f)
    {
    fputc('[', f);
    for (int i = 0; i < array->next; ++i)
        {
        dump_data(array->array[i], f);
        if (array->array[i+1])
            fputc(',', f);
        }
    fputc(']', f);
    }

static void dump_map_node(MAP_NODE *node, FILE *f)
    {
    if (node)
        {
        dump_string(node->key, f);
        fputc(':', f);
        dump_data(node->data, f);
        if (node->next)
            {
            fputc(',', f);
            dump_map_node(node->next, f); // recursion
            }
        }
     }

static void dump_map(MAP_NODE *map, FILE *f)
    {
    fputc('{', f);
    dump_map_node(map, f);
    fputc('}', f);
    }

JSON *json_parse_file(FILE *f)
    {
    JSON *json = create_json();
    int c = skip_whitespace(f);
    switch (c)
        {
    case '{':
        json->data = create_data_map(json);
        parse_into_map(f, json->data->data.map, json);
        break;
    case '[':
        json->data = create_data_array(json);
        parse_into_array(f, json->data->data.array, json);
        break;
    case '"':
        json->data = create_data_string(json, parse_string(f, json));
        break;
    case 't':
    case 'f':
        json->data = create_data_boolean(json, parse_boolean(f));
        break;
    case 'n':
        parse_null(f);
        json->data = create_data_null(json);
        break;
    default:
        json->data = create_data_number(json, parse_number(f, c, json));
        }

    if (skip_whitespace(f) != EOF)
        fatal("invalid json, garbage at end");

    return json;
    }

#ifdef _GNU_SOURCE

JSON *json_parse_string(const char *s)
    {
    FILE *f = fmemopen((void*)s, strlen(s), "r");
    JSON *json = json_parse_file(f);
    fclose(f);
    return json;
    }
#endif

void json_destroy(JSON *doomed)
    {
    recurse_and_destroy_data(doomed, doomed->data);
    free(doomed->work_buffer);
    PoolDestroy(doomed->data_pool);
    PoolDestroy(doomed->map_pool);
    PoolDestroy(doomed->array_pool);
    free(doomed);
    }

void json_dump(JSON *json, FILE *f)
    {
    dump_data(json->data, f);
    }

bool json_is_null(JSON_DATA *data)
    {
    return data->type == null;
    }

JSON_DATA *json_get_root(JSON *json)
    {
    return json->data;
    }

bool json_is_string(JSON_DATA *data)
    {
    return data->type == string;
    }

const char *json_string(JSON_DATA *data)
    {
    if (json_is_string(data))
        return data->data.string;
    return NULL;
    }

bool json_is_number(JSON_DATA *data)
    {
    return data->type == number;
    }

double json_number(JSON_DATA *data)
    {
    if (json_is_number(data))
        return atof(data->data.string);
    return NAN;
    }

bool json_is_boolean(JSON_DATA *data)
    {
    return data->type == boolean;
    }

JSON_DATA **json_array(JSON_DATA *data)
    {
    return data->data.array->array;
    }
    
bool json_boolean(JSON_DATA *data)
    {
    if (json_is_boolean(data))
        return data->data.string[0] == 't';
    return false; // :-(
    }

bool json_is_object(JSON_DATA *data)
    {
    return data->type == map;
    }

bool json_is_array(JSON_DATA *data)
    {
    return data->type == array;
    }

static JSON_DATA *find_map_data(MAP_NODE *map, const char *key)
    {
    MAP_NODE *node = map;
    while (node && strcmp(node->key, key))
        node = node->next;
    if (node)
        return node->data;
    else
        return NULL;
    }

static JSON_DATA *find_array_data(ARRAY *array, size_t i)
    {
    if (i >= array->size)
        return NULL;
    else
        return array->array[i];
    }

static JSON_DATA *_json_get_data(JSON_DATA *data, char *query)
    {
    size_t i = 0;
    char *p = query;
    while (*p && *p != QUERY_DELIM)
        {
        ++p;
        ++i;
        }
    if (*p == QUERY_DELIM)
        {
        *p = '\0';
        JSON_DATA *next_data = NULL;
        if (json_is_object(data))
            next_data = find_map_data(data->data.map, query);
        else if (isdigit(query[0]) && json_is_array(data))
            next_data = find_array_data(data->data.array, atoi(query));
        *p = QUERY_DELIM;
        if (next_data)
            return _json_get_data(next_data, p + 1);
        else
            return NULL;
        }
    else if (json_is_object(data))
        return find_map_data(data->data.map, query);
    else if (isdigit(query[0]) && json_is_array(data))
        return find_array_data(data->data.array, atoi(query));
    else
        return NULL;
    }

JSON_DATA *json_get_data(JSON_DATA *data, const char *query)
    {
    char *_query = strdup(query);
    JSON_DATA *rval = _json_get_data(data, _query);
    free(_query);
    return rval;
    }
