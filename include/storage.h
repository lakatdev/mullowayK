#ifndef STORAGE_H
#define STORAGE_H

#define STORAGE_KEY_SIZE 256
#define STORAGE_RECORD_SIZE 32000000

typedef struct {
    char key[STORAGE_KEY_SIZE];
    char valid;
    unsigned int size;
    char unassigned[512 - STORAGE_KEY_SIZE - 1 - sizeof(unsigned int)];
} StorageRecordHeader;

void init_storage(unsigned int start_address);
void write_to_storage(const char* key, const char* data, unsigned int size);
void read_from_storage(const char* key, char* buffer, unsigned int* size);
void delete_from_storage(const char* key);
int files_exists(const char* key);
int get_record_count();
void write_magic_number(unsigned int lba);
int get_record_key(unsigned int index, char* key_out);
char is_storage_initialized();

#endif
