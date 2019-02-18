//  json.c
//
//  (c) 2019 Skip Sopscak
//  This code is licensed under MIT license (see LICENSE for details)

#include "json.h"
#include "pool.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h> // NAN

#define BUF_INC 1024
#define ARRAY_INC 2 
#define QUERY_DELIM ','
#define NOT_CHAR 10000

typedef struct MAP_NODE MAP_NODE;
typedef struct ARRAY ARRAY;


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
    enum { none = 0, bad_map, bad_array, bad_string, bad_number, 
           bad_boolean, bad_null } error;
    Pool *data_pool;
    Pool *map_pool;
    Pool *array_pool;
    JSON_DATA *data;
    char *token;
    int char_ahead;
    char *work_buffer;
    char *p;
    char *buffer;
    };


static char jgetc(JSON *json)
    {
    if (json->char_ahead == NOT_CHAR)
        return *json->p++;
    char rval = json->char_ahead;
    json->char_ahead = NOT_CHAR;
    return rval;
    }


static JSON_DATA *create_data_boolean(JSON *json)
    {
    JSON_DATA *data = PoolAlloc(json->data_pool);
    data->type = boolean;
    data->data.string = json->token;
    return data;
    }

static JSON_DATA *create_data_null(JSON *json)
    {
    JSON_DATA *data = PoolAlloc(json->data_pool);
    data->type = null;
    data->data.string = json->token;
    return data;
    }

static JSON_DATA *create_data_string(JSON *json)
    {
    JSON_DATA *data = PoolAlloc(json->data_pool);
    data->type = string;
    data->data.string = json->token;
    return data;
    }

static JSON_DATA *create_data_number(JSON *json)
    {
    JSON_DATA *data = PoolAlloc(json->data_pool);
    data->type = number;
    data->data.string = json->token;
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

static int skip_whitespace(JSON *json)
    {
    char c;
    while ((isspace(c = jgetc(json))))
        ;
    return c;
    }

static void put_data_map(JSON *json, JSON_DATA *map, char *key, JSON_DATA *data)
    {
    if (!map->data.map)
        {
        map->data.map = PoolAlloc(json->map_pool);
        map->data.map->key = key;
        map->data.map->data = data;
        map->data.map->next = NULL;
        }
    else
        {
        MAP_NODE *p = map->data.map;
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
    json->error = none;
    json->data_pool = PoolCreate(sizeof(JSON_DATA));
    json->map_pool = PoolCreate(sizeof(MAP_NODE));
    json->array_pool = PoolCreate(sizeof(ARRAY));
    json->data = NULL;
    json->buffer = NULL;
    json->work_buffer = NULL;
    json->char_ahead = NOT_CHAR;
    json->token = NULL;

    return json;
    }


static void terminate_token(JSON *json)
    {
    json->char_ahead = *json->p;
    *json->p = '\0';
    ++json->p;
    }

static void init_work_buffer(JSON *json)
    {
    json->token = json->work_buffer = json->p - 1;
    }

static void putc_work_buffer(JSON *json, char c)
    {
    *json->work_buffer++ = c;
    }

static int parse_string(JSON *json)
    {
    int c;
    init_work_buffer(json);
    while ((c = jgetc(json)) != '\0')
        {
        if (c == '\\')
            {
            c = jgetc(json);
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
                json->error = bad_string;
                break;
                }
            }
        else if (c == '\n' || iscntrl(c))
            json->error = bad_string;
        else if (c == '"')
            break;

        if (json->error)
            break;
        putc_work_buffer(json, c);
        }

    if (c != '"')
        json->error = bad_string;
    else
        putc_work_buffer(json, 0);
    return json->error ? -1 : 0;
    }

static int parse_number(int c, JSON *json)
    {
    bool is_dotted = false;
    if (c == '-' || isdigit(c))
        {
        while ((c = jgetc(json)) != '\0')
            {
            if (c == '.')
                {
                if (is_dotted)
                    {
                    json->error = bad_number;
                    return -1;
                    }
                else
                    is_dotted = true;
                }
            else if (!isdigit(c))
                {
                --json->p;
                break;
                }
            }
        terminate_token(json);
        return 0;
        }
    json->error = bad_number;
    return -1;
    }

static int parse_boolean(JSON *json)
    {
    char c;
    if ((c =jgetc(json)) == 'r')
        if (jgetc(json) == 'u')
            if (jgetc(json) == 'e')
                {
                terminate_token(json);
                return 0;
                }
    if (c == 'a')
        if (jgetc(json) == 'l')
            if (jgetc(json) == 's')
                if (jgetc(json) == 'e')
                    {
                    terminate_token(json);
                    return 0;
                    }
    json->error = bad_boolean;
    return -1;
    }

static int parse_null(JSON *json)
    {
    if (jgetc(json) == 'u')
        if (jgetc(json) == 'l')
            if (jgetc(json) == 'l')
                {
                terminate_token(json);
                return 0;
                }
    json->error = bad_null;
    return -1;
    }


static int parse_into_map(JSON_DATA *map, JSON *json);

static int parse_into_array(ARRAY *array, JSON *json)
    {
    char c = skip_whitespace(json);
    init_work_buffer(json);
    JSON_DATA *data;
    switch (c)
        {
    case ']':
        if (array->next > 0)
            {
            json->error = bad_array;
            return -1;
            }
        return 0;
    case '{':
        data = create_data_map(json);
        parse_into_map(data, json);
        break;
    case '[':
        data = create_data_array(json);
        parse_into_array(data->data.array, json);
        break;
    case '"':
        if (parse_string(json) == 0)
            data = create_data_string(json);
        break;
    case 't':
    case 'f':
        if (parse_boolean(json) == 0)
            data = create_data_boolean(json);
        break;
    case 'n':
        if (parse_null(json) == 0)
            data = create_data_null(json);
        break;
    default:
        if (parse_number(c, json) == 0)
            data = create_data_number(json);
        }
    if (!json->error)
        {
        put_data_array(array, data);
        c = skip_whitespace(json);
        if (c == ',')
            parse_into_array(array, json); // recursion
        else if (c != ']')
            json->error = bad_array;
        }
    return json->error ? -1 : 0;
    }

static int parse_into_map(JSON_DATA *map, JSON *json)
    {
    char c = skip_whitespace(json);
    init_work_buffer(json);
    if (c == '}')
        {
        if (map->data.map)
            {
            json->error = bad_map;
            return -1;
            }
        return 0;
        }
    else if (c != '"')
        json->error = bad_map; // key/string not found

    char *key = json->token;
    JSON_DATA *data = NULL;
    if (!json->error && parse_string(json) == 0)
        {
        c = skip_whitespace(json);
        if (c != ':')
            json->error = bad_map;
        else
            {
            c = skip_whitespace(json);
            init_work_buffer(json);

            switch (c)
                {
            case '{':
                data = create_data_map(json);
                parse_into_map(data, json);
                break;
            case '[':
                data = create_data_array(json);
                parse_into_array(data->data.array, json);
                break;
            case '"':
                if (parse_string(json) == 0)
                    data = create_data_string(json);
                break;
            case 't':
            case 'f':
                if (parse_boolean(json) == 0)
                    data = create_data_boolean(json);
                break;
            case 'n':
                if (parse_null(json) == 0)
                    data = create_data_null(json);
                break;
            default:
                if (parse_number(c, json) == 0)
                    data = create_data_number(json);
                }
            }
        }
    if (!json->error)
        {
        put_data_map(json, map, key, data);
        c = skip_whitespace(json);
        if (c == ',')
            parse_into_map(map, json); // recursion
        else if (c != '}')
            json->error = bad_map;
        }

    return json->error ? -1 : 0;
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

JSON *json_parse_string(char *s, bool should_free)
    {
    JSON *json = create_json();
    json->p = s;
    if (should_free)
        json->buffer = s;
    else
        json->buffer = NULL;
    
    char c = skip_whitespace(json);
    init_work_buffer(json);
    switch (c)
        {
    case '{':
        json->data = create_data_map(json);
        parse_into_map(json->data, json);
        break;
    case '[':
        json->data = create_data_array(json);
        parse_into_array(json->data->data.array, json);
        break;
    case '"':
        if (parse_string(json) == 0)
            json->data = create_data_string(json);
        break;
    case 't':
    case 'f':
        if (parse_boolean(json) == 0)
            json->data = create_data_boolean(json);
        break;
    case 'n':
        if (parse_null(json) == 0)
            json->data = create_data_null(json);
        break;
    default:
        if (parse_number(c, json) == 0)
            json->data = create_data_number(json);
        }

    if (skip_whitespace(json) != '\0' || json->error)
        {
        json_destroy(json);
        json = NULL;
        }

    return json;
    }

JSON *json_parse_file(FILE *f)
    {
    char *buffer = NULL;
    size_t sz = BUF_INC;
    size_t read_so_far = 0;
    while ((buffer = realloc(buffer, sz)))
        {
        read_so_far += fread(buffer + read_so_far, 1, BUF_INC, f); 
        if (feof(f))
            {
            *(buffer + read_so_far) = '\0';
            break;
            }
        if (ferror(f))
            {
            free(buffer);
            return NULL;
            }
        sz += BUF_INC;
        }
    if (buffer)
        return json_parse_string(buffer, true);
    return NULL;
    }

void json_destroy(JSON *doomed)
    {
    recurse_and_destroy_data(doomed, doomed->data);
    PoolDestroy(doomed->data_pool);
    PoolDestroy(doomed->map_pool);
    PoolDestroy(doomed->array_pool);
    if (doomed->buffer)
        free(doomed->buffer);
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
