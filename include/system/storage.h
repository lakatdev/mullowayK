#ifndef STORAGE_H
#define STORAGE_H

#define STORAGE_KEY_SIZE 256
#define STORAGE_RECORD_SIZE 16000000

// boot
typedef struct __attribute__((packed)) {
    unsigned char jmp_boot[3];
    char oem_name[8];
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char num_fats;
    unsigned short root_entry_count; // 0
    unsigned short total_sectors_16; // 0
    unsigned char media_type;
    unsigned short fat_size_16; // 0
    unsigned short sectors_per_track;
    unsigned short num_heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_32;
    // extended fields
    unsigned int fat_size_32;
    unsigned short ext_flags;
    unsigned short fs_version;
    unsigned int root_cluster;
    unsigned short fs_info;
    unsigned short backup_boot_sector;
    unsigned char reserved[12];
    unsigned char drive_number;
    unsigned char reserved1;
    unsigned char boot_signature;
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8];
} FAT32_BootSector;

// fsinfo
typedef struct __attribute__((packed)) {
    unsigned int lead_signature; // 0x41615252
    unsigned char reserved1[480];
    unsigned int struct_signature; // 0x61417272
    unsigned int free_count;
    unsigned int next_free;
    unsigned char reserved2[12];
    unsigned int trail_signature; // 0xAA550000
} FAT32_FSInfo;

// directory entry
typedef struct __attribute__((packed)) {
    char name[11]; // 8.3 format
    unsigned char attr;
    unsigned char nt_reserved;
    unsigned char created_tenth;
    unsigned short created_time;
    unsigned short created_date;
    unsigned short accessed_date;
    unsigned short first_cluster_high;
    unsigned short modified_time;
    unsigned short modified_date;
    unsigned short first_cluster_low;
    unsigned int file_size;
} FAT32_DirEntry;

// long filename entry
typedef struct __attribute__((packed)) {
    unsigned char order;
    unsigned short name1[5];
    unsigned char attr; // 0x0F
    unsigned char type;
    unsigned char checksum;
    unsigned short name2[6];
    unsigned short first_cluster_low; // 0
    unsigned short name3[2];
} FAT32_LFNEntry;

#define FAT32_ATTR_READ_ONLY 0x01
#define FAT32_ATTR_HIDDEN 0x02
#define FAT32_ATTR_SYSTEM 0x04
#define FAT32_ATTR_VOLUME_ID 0x08
#define FAT32_ATTR_DIRECTORY 0x10
#define FAT32_ATTR_ARCHIVE 0x20
#define FAT32_ATTR_LONG_NAME 0x0F
#define FAT32_FREE_CLUSTER 0x00000000
#define FAT32_EOC 0x0FFFFFFF
#define FAT32_BAD_CLUSTER 0x0FFFFFF7

// fixed 16 mb file size 4000 clusters
#define SECTORS_PER_CLUSTER 8
#define BYTES_PER_CLUSTER (512 * SECTORS_PER_CLUSTER)
#define CLUSTERS_PER_FILE (STORAGE_RECORD_SIZE / BYTES_PER_CLUSTER)

void init_storage(unsigned int start_address);
void write_to_storage(const char* key, const char* data, unsigned int size);
void read_from_storage(const char* key, char* buffer, unsigned int* size);
void delete_from_storage(const char* key);
int files_exists(const char* key);
int get_record_count();
int write_magic_number(unsigned int lba);
int get_record_key(unsigned int index, char* key_out);
char is_storage_initialized();

#endif
