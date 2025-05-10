#ifndef MEMORY_H
#define MEMORY_H

typedef struct Block Block;
struct Block {
    Block* next;
    Block* prev;
    unsigned int size;
    char allocated;
};

void init_memory(unsigned int);
unsigned int get_memory_size();
void *memcpy(void *dest, const void *src, unsigned int size);
extern void* memcpy_sse(void* dest, void* src, unsigned int size);
extern void enable_sse();
void* memset(void *dest, char val, int count);
unsigned int strlen(const char *str);
int strcmp(const char *str1, const char *str2);

#endif
