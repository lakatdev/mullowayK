#include <memory.h>

unsigned int total_available = 0;

void init_memory(unsigned int size)
{
    total_available = size;
}

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

unsigned int get_memory_size()
{
    return total_available;
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
