#ifndef MEMORY_H
#define MEMORY_H

void *memcpy(void *dest, const void *src, unsigned int size);
extern void* memcpy_sse(void* dest, void* src, unsigned int size);
extern void enable_sse();
void* memset(void *dest, char val, int count);
unsigned int strlen(const char *str);
int strcmp(const char *str1, const char *str2);
int memcmp(const void *str1, const void *str2, unsigned int size);

#endif
