// test.c

#include "string.h"
#include "json.h"

int main(int argc, char **argv)
    {
    char *s = get_json_string(1024);
    ret_json_string(s);

    JSON *json = init_json_file(stdin);

    return 0;
    }
