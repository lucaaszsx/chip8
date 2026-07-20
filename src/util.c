#include <ctype.h>
#include "util.h"

char *str_to_lower(char *str) {
    char *p = str;
    for (; *p; ++p) *p = tolower((unsigned char)*p);

    return str;
}
