#include <stdio.h>
#include <stdlib.h>

void *bf_safe_calloc(size_t n, size_t size) {
    void *dest = calloc(n, size);
    if (dest == NULL)
        exit(1);
    return dest;
}