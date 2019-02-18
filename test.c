//  test.c
//
//  (c) 2019 Skip Sopscak
//  This code is licensed under MIT license (see LICENSE for details)

#include "test.h"
#include "string.h"
#include "json.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
    {
    const char *tstrings[] = { 
        "  27.312  ",
        "true",
        "false",
        "null",
        "[]",
        "{}",
        "[1]",
        "[1,2]",
        "[1,2,\"foo\"]",
        "{\"foo\": \"bar\", \"baz\": \"blah\"}",
        "{\"a\": {\"foo\": \"bar\", \"baz\": \"blah\"}, \"b\": {\"foo\": \"bar\", \"baz\": \"blah\"}}",
        "  \" foobar \"  "
    };

    for (int i = 0; i < sizeof(tstrings)/sizeof(tstrings[0]); ++i)
        {
        JSON *json = json_parse_string(strdup(tstrings[i]), true);
        if (!json)
            {
            printf("Parsing error\n");
            exit(-1);
            }
        JSON_DATA *data = json_get_root(json);
        if (json_is_string(data))
            printf("\nstring[%s]\n", json_string(data));
        else if (json_is_null(data))
            printf("\nIt's null\n");
        else if (json_is_number(data))
            printf("\nnumber[%f]\n", json_number(data));
        else if (json_is_boolean(data))
            printf("\nboolean[%d]\n", json_boolean(data));
        else if (json_is_object(data))
            printf("\nIt's an object\n");
        else if (json_is_array(data))
            printf("\nIt's an array\n");
        else
            {
            printf("\nUnknown data type\n");
            return -1;
            }
        json_dump(json, stdout);
        printf("\n");
        json_destroy(json);
        }
    for (int i = 0; i < 1024; ++i)
        {
        JSON *json = json_parse_string(strdup(big_test), true);
        json_destroy(json);
        printf(".");
        }
    printf("\n");

    FILE *f = fopen("test.json", "r");
    JSON *json = json_parse_file(f);
    fclose(f);
    JSON_DATA *root = json_get_root(json);
    JSON_DATA *d = json_get_data(root, "bands,beatles,lead");
    printf("%s was lead guitar\n", json_string(d));
    d = json_get_data(root, "genres,2");
    printf("%s is the 3rd 'P'\n", json_string(d));
    d = json_get_data(root, "genres");
    JSON_DATA **da = json_array(d);
    printf("The 4 P's:\n");
    while (*da)
        {
        printf("  %s\n", json_string(*da));
        ++da;
        }
    d = json_get_data(root, "bands");
    assert(d);
    d = json_get_data(root, "big_string2");
    assert(d);
    d = json_get_data(root, "bands,devo,bass");
    assert(!d);

    json_destroy(json);

    json = json_parse_file(stdin);
    json_dump(json, stdout);
    printf("\n");

    return 0;
    }
