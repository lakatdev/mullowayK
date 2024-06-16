#ifndef MEMORY_H
#define MEMORY_H

typedef struct Block Block;
struct Block {
    Block* next;
    Block* prev;
    unsigned int size;
    char allocated;
};

void init_memory(unsigned int, unsigned int);
void* malloc(unsigned int);
void free(void*);
void defragment();
unsigned int get_memory_usage();
void *memcpy(void *dest, const void *src, unsigned int size);
extern void* memcpy_sse(void* dest, void* src, unsigned int size);
extern void enable_sse();
void* memset(void *dest, char val, int count);
void* realloc(void* ptr, unsigned int size);

#endif
