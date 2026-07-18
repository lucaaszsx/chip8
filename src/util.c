#include <ctype.h>
#include "util.h"

char *str_to_lower(char *str) {
    char *final = str;
    for (; *final; ++final) *final = tolower((unsigned char)*final);

    return final;
}
