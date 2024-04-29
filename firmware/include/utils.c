#include <string.h>

size_t strlen_s(const char *str, size_t max_len) { // Replacement for the somehow missing strlen_s function
    const char * end = (const char *)memchr(str, '\0', max_len);
    if (end == NULL)
        return max_len;
    else
        return end - str;
}
