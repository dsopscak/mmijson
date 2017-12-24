/* test
 */

#include "string.h"

int main(int argc, char **argv)
    {
    char *s = get_json_string(1024);
    ret_json_string(s);

    return 0;
    }
