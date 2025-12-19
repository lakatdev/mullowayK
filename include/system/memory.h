#ifndef MEMORY_H
#define MEMORY_H

void *memcpy(void *dest, const void *src, unsigned int size);
extern void* memcpy_sse(void* dest, void* src, unsigned int size);
extern void enable_sse();
void* memset(void *dest, char val, int count);
unsigned int strlen(const char *str);
int strcmp(const char *str1, const char *str2);
int memcmp(const void *str1, const void *str2, unsigned int size);
char* strtok_r(char* str, const char* delim, char** saveptr);
char* strchr(const char* s, int c);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, unsigned int n);
int isspace(int c);
long strtol(const char* nptr, char** endptr, int base);
float strtof(const char* nptr, char** endptr);
void* memmove(void* dest, const void* src, int n);

#endif
