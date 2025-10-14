#include "revert_string.h"
#include <string.h>  // для strlen

void RevertString(char *str)
{
    if (!str) return;                // защита от NULL
    size_t len = strlen(str);
    if (len < 2) return;             // пустая или из 1 символа — нечего разворачивать

    for (size_t i = 0, j = len - 1; i < j; ++i, --j) {
        char tmp = str[i];
        str[i] = str[j];
        str[j] = tmp;
    }
}
