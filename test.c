// test.c

#include "string.h"
#include "json.h"

int main(int argc, char **argv)
    {
    char *s = get_json_string(1024);
    ret_json_string(s);

    JSON *json = parse_json_file(stdin);
    dump_json(json, stdout);
    destroy_json(json);

    return 0;
    }
