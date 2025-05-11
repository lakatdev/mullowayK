#include <memory.h>

void* memcpy(void *dest, const void *src, unsigned int size)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    for(; size != 0; size--) *dp++ = *sp++;
    return dest;
}

void* memset(void *dest, char val, int count)
{
    char *temp = (char *)dest;
    for(; count != 0; count--) *temp++ = val;
    return dest;
}

unsigned int strlen(const char *str)
{
    unsigned int len = 0;
    while (*str++) len++;
    return len;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

int memcmp(const void *str1, const void *str2, unsigned int size)
{
    const unsigned char *s1 = (const unsigned char *)str1;
    const unsigned char *s2 = (const unsigned char *)str2;
    for (; size != 0; size--) {
        if (*s1 != *s2) {
            return (*s1 - *s2);
        }
        s1++;
        s2++;
    }
    return 0;
}
