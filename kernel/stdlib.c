
#include <kernel.h>
#include "../include/kernel.h"

int k_strlen(const char* str) {
    const char* string = str;

    for(string; *string; string++);

    /* difference in contiguous memory addresses */
    return string - str;
}

void* k_memcpy(void* dst, const void* src, int len) {
    char* destination = (char*) dst;
    char* source = (char*) src;

    if(destination == source) {
        return dst;
    }

    while(len > 0) {
        *destination = *source;
        destination++;
        source++;

        len--;
    }

    return dst;
}

int k_memcmp(const void* b1, const void* b2, int len) {
    const char* block1 = (char*) b1;
    const char* block2 = (char*) b2;

    if(block1 == block2) {
        return 0;
    }

    while(len > 0) {
        if(*block1 != *block2) {
            return *block1 - *block2;
        }

        block1++;
        block2++;

        len--;
    }

    return 0;
}

BOOL k_strcmp(const char* str1, const char* str2) {
    int length1 = k_strlen(str1);
    int length2 = k_strlen(str2);

    if(length1 != length2) {
        return FALSE;
    } else {
        return !(k_memcmp((void*) str1, (void*) str2, length1));
    }
}
