#include <memory.h>

// beginning of the linked list
Block* first;

// amount of allocated memory
unsigned int total_allocated = 0;
unsigned int total_available = 0;

// right now only works up to 4 gigabytes because block size is stored in an unsigned int

/**
 * @brief inits the first memory block of the linked list with
 * its size as all of the available system memory
 * 
 * @param first_addr beginning of managed memory
 * @param size available memory for the memory manager
 */
void init_memory(unsigned int first_addr, unsigned int size)
{
    first = (Block*)first_addr;
    first->next = (void*)0;
    first->prev = (void*)0;
    first->allocated = 0;
    first->size = size - sizeof(Block);
    total_available = size;
}

void* malloc(unsigned int size)
{
    Block* best = (void*)0;
    Block* curr = first;

    // find next best fitting block by iterating through the list and finding the smallest still fitting one
    while (curr != (void*)0) {
        if (!curr->allocated && curr->size >= size) {
            if (best == (void*)0 || curr->size < best->size) {
                best = curr;
            }
        }

        curr = curr->next;
    }

    // no suitable block was found
    if (best == (void*)0) {
        return (void*)0;
    }

    // split the block so new ones will be available later
    if (best->size >= size + sizeof(Block) + 1) {
        Block* new_block = (Block*)((unsigned int)best + sizeof(Block) + size);
        new_block->next = best->next;
        new_block->prev = best;
        new_block->allocated = 0;
        new_block->size = best->size - sizeof(Block) - size;

        if (best->next != (void*)0) {
            best->next->prev = new_block;
        }

        best->next = new_block;
        best->size = size;
    }

    best->allocated = 1;
    total_allocated += size;
    return (void*)((unsigned int)best + sizeof(Block));
}

void defragment()
{
    Block *curr = first;
    Block *prev = (void*)0;

    while (curr != (void*)0) {
        if (!curr->allocated) {
            if (prev != (void*)0 && !prev->allocated) {
                prev->size += sizeof(Block) + curr->size;
                prev->next = curr->next;
                if (curr->next != (void*)0) {
                    curr->next->prev = prev;
                }
            }
            else {
                prev = curr;
            }
        }
        else {
            prev = curr;
        }
        curr = curr->next;
    }
}

void free(void* ptr)
{
    if (ptr == (void*)0) {
        return;
    }
    Block* freed_block = (Block*)((unsigned int)ptr - sizeof(Block));
    freed_block->allocated = 0;
    total_allocated -= freed_block->size;
    // could be displaced later because it gives overhead
    defragment();
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

unsigned int get_memory_usage()
{
    return (unsigned int)((float)total_allocated / total_available * 10000.0f);
}

void* realloc(void* ptr, unsigned int size)
{
    if (ptr == (void*)0) {
        return malloc(size);
    }

    Block* current = (Block*)((unsigned int)ptr - sizeof(Block));

    if (current->size >= size && current->size <= size + sizeof(Block) + 1) {
        return ptr;
    }

    void* new_ptr = malloc(size);
    if (new_ptr == (void*)0) {
        return (void*)0;
    }
    memcpy(new_ptr, ptr, current->size);
    free(ptr);

    return new_ptr;
}
