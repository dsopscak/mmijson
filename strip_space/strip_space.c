//  strip_space
//
//  Strip un-quoted whitespaces and return number of them

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

int main(void)
    {
    int space_count = 0;
    bool in_quote = false;
    int c;
    while ((c = getchar()) != EOF)
        {
        if (c == '"')
            in_quote = !in_quote;
        if (in_quote && c == '\\')
            {
            putchar(c);
            c = getchar();
            }
        if (!in_quote && isspace(c))
            ++space_count;
        else
            putchar(c);
        }

    if (in_quote)
        {
        fprintf(stderr, "ERROR: eof before end of quote\n");
        return -1;
        }
    fprintf(stderr, "%d\n", space_count);
    return 0;
    }
