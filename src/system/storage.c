#include <drivers/ata.h>
#include <system/storage.h>
#include <system/memory.h>
#include <system/interrupts.h>
#include <system/interface.h>
#include <system/port.h>
#include <drivers/pci.h>

// filesystem state
unsigned int first_lba = 0;
char storage_initialized = 0;
unsigned int fat_start_lba = 0;
unsigned int fat_size = 0;
unsigned int data_start_lba = 0;
unsigned int root_dir_cluster = 0;
unsigned int next_free_cluster = 2;
FAT32_DirEntry root_dir_cache[128];
int root_dir_entry_count = 0;

#define FAT_CACHE_SECTORS 16
#define FAT_ENTRIES_PER_SECTOR 128

unsigned int fat_cache[FAT_CACHE_SECTORS * FAT_ENTRIES_PER_SECTOR];
unsigned int fat_cache_start_sector = 0xFFFFFFFF;
char fat_cache_dirty = 0;

void flush_fat_cache()
{
    if (!fat_cache_dirty || fat_cache_start_sector == 0xFFFFFFFF) {
        return;
    }
    
    for (int i = 0; i < FAT_CACHE_SECTORS; i++) {
        ata_lba_write_safe(fat_start_lba + fat_cache_start_sector + i, 1, 
                          (unsigned short*)(&fat_cache[i * FAT_ENTRIES_PER_SECTOR]));
    }
    
    for (int i = 0; i < FAT_CACHE_SECTORS; i++) {
        ata_lba_write_safe(fat_start_lba + fat_size + fat_cache_start_sector + i, 1, 
                          (unsigned short*)(&fat_cache[i * FAT_ENTRIES_PER_SECTOR]));
    }
    
    fat_cache_dirty = 0;
}

void load_fat_cache(unsigned int sector)
{
    sector = (sector / FAT_CACHE_SECTORS) * FAT_CACHE_SECTORS;
    
    if (fat_cache_start_sector == sector) {
        return;
    }
    
    flush_fat_cache();
    
    for (int i = 0; i < FAT_CACHE_SECTORS; i++) {
        ata_lba_read(fat_start_lba + sector + i, 1, 
                    (unsigned short*)(&fat_cache[i * FAT_ENTRIES_PER_SECTOR]));
    }
    
    fat_cache_start_sector = sector;
    fat_cache_dirty = 0;
}

unsigned int cluster_to_lba(unsigned int cluster)
{
    if (cluster < 2) {
        return 0;
    }
    return data_start_lba + (cluster - 2) * SECTORS_PER_CLUSTER;
}

unsigned int read_fat_entry(unsigned int cluster)
{
    unsigned int fat_offset = cluster * 4;
    unsigned int fat_sector = fat_offset / 512;
    unsigned int entry_offset = (fat_offset % 512) / 4;
    
    load_fat_cache(fat_sector);
    
    unsigned int cache_offset = (fat_sector - fat_cache_start_sector) * FAT_ENTRIES_PER_SECTOR + entry_offset;
    return fat_cache[cache_offset] & 0x0FFFFFFF;
}

void write_fat_entry(unsigned int cluster, unsigned int value)
{
    unsigned int fat_offset = cluster * 4;
    unsigned int fat_sector = fat_offset / 512;
    unsigned int entry_offset = (fat_offset % 512) / 4;
    
    load_fat_cache(fat_sector);
    
    unsigned int cache_offset = (fat_sector - fat_cache_start_sector) * FAT_ENTRIES_PER_SECTOR + entry_offset;
    fat_cache[cache_offset] = (fat_cache[cache_offset] & 0xF0000000) | (value & 0x0FFFFFFF);
    fat_cache_dirty = 1;
}

unsigned int allocate_cluster_chain(unsigned int num_clusters)
{
    if (num_clusters == 0) return 0;
    
    unsigned int first_cluster = next_free_cluster;
    unsigned int prev_cluster = 0;
    unsigned int allocated = 0;
    
    for (unsigned int i = 0; i < num_clusters && next_free_cluster < 0x0FFFFFF0; i++) {
        if (read_fat_entry(next_free_cluster) == FAT32_FREE_CLUSTER) {
            if (prev_cluster != 0) {
                write_fat_entry(prev_cluster, next_free_cluster);
            }
            prev_cluster = next_free_cluster;
            allocated++;
            next_free_cluster++;
        }
        else {
            next_free_cluster++;
        }
    }
    
    if (allocated == num_clusters && prev_cluster != 0) {
        write_fat_entry(prev_cluster, FAT32_EOC);
        flush_fat_cache();
        return first_cluster;
    }
    
    return 0;
}

void free_cluster_chain(unsigned int start_cluster)
{
    unsigned int cluster = start_cluster;
    while (cluster >= 2 && cluster < FAT32_EOC) {
        unsigned int next = read_fat_entry(cluster);
        write_fat_entry(cluster, FAT32_FREE_CLUSTER);
        cluster = next;
    }
    flush_fat_cache();
}

unsigned char lfn_checksum(const unsigned char* short_name)
{
    unsigned char sum = 0;
    for (int i = 0; i < 11; i++) {
        sum = ((sum & 1) << 7) + (sum >> 1) + short_name[i];
    }
    return sum;
}

void create_short_name(const char* long_name, char* short_name)
{
    memset(short_name, ' ', 11);
    
    int len = strlen(long_name);
    int dot_pos = -1;
    for (int i = len - 1; i >= 0; i--) {
        if (long_name[i] == '.') {
            dot_pos = i;
            break;
        }
    }
    
    int name_len = (dot_pos > 0) ? dot_pos : len;
    if (name_len > 8) name_len = 8;
    
    for (int i = 0; i < name_len; i++) {
        char c = long_name[i];
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        short_name[i] = c;
    }
    
    if (dot_pos > 0 && dot_pos < len - 1) {
        int ext_len = len - dot_pos - 1;
        if (ext_len > 3) ext_len = 3;
        for (int i = 0; i < ext_len; i++) {
            char c = long_name[dot_pos + 1 + i];
            if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
            short_name[8 + i] = c;
        }
    }
}

void extract_filename(FAT32_DirEntry* entry, char* filename)
{
    int name_len = 0;
    for (int j = 0; j < 8 && entry->name[j] != ' '; j++) {
        filename[name_len++] = entry->name[j];
    }
    if (entry->name[8] != ' ') {
        filename[name_len++] = '.';
        for (int j = 8; j < 11 && entry->name[j] != ' '; j++) {
            filename[name_len++] = entry->name[j];
        }
    }
    filename[name_len] = '\0';
}

int filename_matches(const char* name1, const char* name2)
{
    int i = 0;
    while (name1[i] && name2[i]) {
        char c1 = name1[i];
        char c2 = name2[i];
        if (c1 >= 'a' && c1 <= 'z') c1 = c1 - 'a' + 'A';
        if (c2 >= 'a' && c2 <= 'z') c2 = c2 - 'a' + 'A';
        if (c1 != c2) return 0;
        i++;
    }
    return name1[i] == name2[i];
}

void load_root_directory()
{
    root_dir_entry_count = 0;
    unsigned int cluster = root_dir_cluster;
    int entry_idx = 0;
    
    while (cluster >= 2 && cluster < FAT32_EOC && entry_idx < 128) {
        unsigned int lba = cluster_to_lba(cluster);
        unsigned char sector[512];
        
        for (int s = 0; s < SECTORS_PER_CLUSTER && entry_idx < 128; s++) {
            ata_lba_read(lba + s, 1, (unsigned short*)sector);
            
            FAT32_DirEntry* entries = (FAT32_DirEntry*)sector;
            for (int e = 0; e < 16 && entry_idx < 128; e++) {
                if (entries[e].name[0] == 0x00) {
                    root_dir_entry_count = entry_idx;
                    return;
                }
                if (entries[e].name[0] == 0xE5) {
                    continue;
                }
                if (entries[e].attr == FAT32_ATTR_LONG_NAME) {
                    continue;
                }
                if (entries[e].attr & FAT32_ATTR_VOLUME_ID) {
                    continue;
                }
                if (entries[e].attr & FAT32_ATTR_DIRECTORY) {
                    continue;
                }
                
                memcpy(&root_dir_cache[entry_idx], &entries[e], sizeof(FAT32_DirEntry));
                entry_idx++;
            }
        }
        
        cluster = read_fat_entry(cluster);
    }
    
    root_dir_entry_count = entry_idx;
}

void save_root_directory()
{
    flush_fat_cache();
    
    unsigned int cluster = root_dir_cluster;
    int entry_idx = 0;
    
    while (cluster >= 2 && cluster < FAT32_EOC) {
        unsigned int lba = cluster_to_lba(cluster);
        unsigned char sector[512];
        
        for (int s = 0; s < SECTORS_PER_CLUSTER; s++) {
            memset(sector, 0, 512);
            FAT32_DirEntry* entries = (FAT32_DirEntry*)sector;
            
            for (int e = 0; e < 16; e++) {
                if (entry_idx < root_dir_entry_count) {
                    memcpy(&entries[e], &root_dir_cache[entry_idx], sizeof(FAT32_DirEntry));
                    entry_idx++;
                }
            }
            
            ata_lba_write_safe(lba + s, 1, (unsigned short*)sector);
        }
        break;
    }
}

int ata_controller_present()
{
    printf("ATA: status = ");
    print_hex(ata_get_status());
    printf(", altstatus = ");
    print_hex(ata_get_altstatus());
    printf("\n");

    unsigned char status = ata_get_status();
    if (status == 0xFF) {
        printf("ATA: Status register - no controller\n");
        return 0;
    }

    unsigned short identify_data[256];
    if (ata_identify_drive(0xA0, identify_data) == 0) {
        return 1;
    }
    
    if (ata_identify_drive(0xB0, identify_data) == 0) {
        ata_select_drive(0xB0);
        return 1;
    }

    printf("ATA: No drives found on this controller\n");
    return 0;
}

void init_storage(unsigned int start_address)
{
    storage_initialized = 0;
    first_lba = start_address;

    scan_bus(0);

    unsigned short io = 0, ctrl = 0;
    int controller_found = 0;

    if (pci_get_ide_selected_ports(&io, &ctrl)) {
        ata_set_bases(io, ctrl);
        ata_reset_channel();

        if (ata_controller_present()) {
            controller_found = 1;
        }
    }

    if (!controller_found) {
        while (pci_next_ide_controller(&io, &ctrl)) {
            ata_set_bases(io, ctrl);
            ata_reset_channel();
            
            if (ata_controller_present()) {
                controller_found = 1;
                break;
            }
        }
    }

    if (!controller_found) {
        printf("ATA: No working controller found. Storage disabled.\n");
        return;
    }
    
    printf("ATA: Controller found. Initializing FAT32 storage.\n");

    unsigned char boot_sector[512];
    if (ata_lba_read_safe(first_lba, 1, (unsigned short*)boot_sector) != 0) {
        printf("ATA: Failed to read boot sector. Storage disabled.\n");
        return;
    }

    FAT32_BootSector* bs = (FAT32_BootSector*)boot_sector;
    
    if (bs->bytes_per_sector == 512 && 
        bs->sectors_per_cluster == SECTORS_PER_CLUSTER &&
        bs->num_fats > 0 &&
        bs->fat_size_32 > 0 &&
        boot_sector[510] == 0x55 && boot_sector[511] == 0xAA) {
        
        printf("ATA: Valid FAT32 filesystem detected.\n");
        
        fat_start_lba = first_lba + bs->reserved_sectors;
        fat_size = bs->fat_size_32;
        data_start_lba = fat_start_lba + (bs->num_fats * fat_size);
        root_dir_cluster = bs->root_cluster;
        
        if (bs->fs_info != 0 && bs->fs_info != 0xFFFF) {
            unsigned char fsinfo_sector[512];
            if (ata_lba_read_safe(first_lba + bs->fs_info, 1, (unsigned short*)fsinfo_sector) == 0) {
                FAT32_FSInfo* fsinfo = (FAT32_FSInfo*)fsinfo_sector;
                if (fsinfo->lead_signature == 0x41615252 && 
                    fsinfo->struct_signature == 0x61417272 &&
                    fsinfo->next_free != 0xFFFFFFFF) {
                    next_free_cluster = fsinfo->next_free;
                }
            }
        }
        
        if (next_free_cluster < 2) {
            next_free_cluster = 2;
        }
        
        fat_cache_start_sector = 0xFFFFFFFF;
        fat_cache_dirty = 0;
        
        load_root_directory();
        storage_initialized = 1;
        
        printf("ATA: FAT32 initialized. Files: ");
        print_hex(root_dir_entry_count);
        printf("\n");
    }
    else {
        printf("ATA: No valid FAT32 filesystem. Disk can be formatted.\n");
    }
}

char is_storage_initialized()
{
    return storage_initialized;
}

int get_record_count()
{
    if (!storage_initialized) {
        return 0;
    }
    return root_dir_entry_count;
}

void write_to_storage(const char* key, const char* data, unsigned int size)
{
    if (!storage_initialized || size > STORAGE_RECORD_SIZE) {
        return;
    }

    int found_idx = -1;
    for (int i = 0; i < root_dir_entry_count; i++) {
        char entry_name[256];
        extract_filename(&root_dir_cache[i], entry_name);
        
        if (filename_matches(entry_name, key)) {
            found_idx = i;
            break;
        }
    }

    unsigned int first_cluster;
    unsigned int old_cluster = 0;
    
    if (found_idx >= 0) {
        old_cluster = (root_dir_cache[found_idx].first_cluster_high << 16) | root_dir_cache[found_idx].first_cluster_low;
    }
    else {
        if (root_dir_entry_count >= 128) {
            printf("ATA: Directory full\n");
            return;
        }
        found_idx = root_dir_entry_count;
    }

    first_cluster = allocate_cluster_chain(CLUSTERS_PER_FILE);
    if (first_cluster == 0) {
        printf("ATA: Failed to allocate clusters\n");
        return;
    }

    unsigned int cluster = first_cluster;
    unsigned int bytes_written = 0;
    
    while (cluster >= 2 && cluster < FAT32_EOC && bytes_written < size) {
        unsigned int lba = cluster_to_lba(cluster);
        
        for (int s = 0; s < SECTORS_PER_CLUSTER && bytes_written < size; s++) {
            unsigned char sector[512];
            memset(sector, 0, 512);
            
            unsigned int bytes_to_copy = 512;
            if (bytes_written + bytes_to_copy > size) {
                bytes_to_copy = size - bytes_written;
            }
            
            memcpy(sector, data + bytes_written, bytes_to_copy);
            
            if (ata_lba_write_safe(lba + s, 1, (unsigned short*)sector) != 0) {
                printf("ATA: Failed to write file sector\n");
                free_cluster_chain(first_cluster);
                return;
            }
            
            bytes_written += bytes_to_copy;
        }
        
        cluster = read_fat_entry(cluster);
    }

    FAT32_DirEntry* entry = &root_dir_cache[found_idx];
    memset(entry, 0, sizeof(FAT32_DirEntry));
    create_short_name(key, entry->name);
    entry->attr = FAT32_ATTR_ARCHIVE;
    entry->first_cluster_high = (first_cluster >> 16) & 0xFFFF;
    entry->first_cluster_low = first_cluster & 0xFFFF;
    entry->file_size = size;
    
    if (old_cluster == 0) {
        root_dir_entry_count++;
    }

    save_root_directory();
    
    if (old_cluster >= 2) {
        free_cluster_chain(old_cluster);
    }
}

void read_from_storage(const char* key, char* buffer, unsigned int* size)
{
    *size = 0;
    memset(buffer, 0, STORAGE_RECORD_SIZE);
    
    if (!storage_initialized) {
        return;
    }

    int found_idx = -1;
    for (int i = 0; i < root_dir_entry_count; i++) {
        char entry_name[256];
        extract_filename(&root_dir_cache[i], entry_name);
        
        if (filename_matches(entry_name, key)) {
            found_idx = i;
            break;
        }
    }

    if (found_idx < 0) {
        return;
    }

    FAT32_DirEntry* entry = &root_dir_cache[found_idx];
    unsigned int file_size = entry->file_size;
    unsigned int first_cluster = (entry->first_cluster_high << 16) | entry->first_cluster_low;

    if (first_cluster < 2) {
        return;
    }
    
    if (file_size > STORAGE_RECORD_SIZE) {
        printf("ATA: File size too large\n");
        return;
    }

    unsigned int cluster = first_cluster;
    unsigned int bytes_read = 0;
    unsigned int cluster_count = 0;
    
    while (cluster >= 2 && cluster < FAT32_EOC && bytes_read < file_size) {
        unsigned int lba = cluster_to_lba(cluster);
        
        cluster_count++;
        if (cluster_count > CLUSTERS_PER_FILE + 10) {
            printf("ATA: Cluster chain too long, possible corruption\n");
            *size = 0;
            memset(buffer, 0, STORAGE_RECORD_SIZE);
            return;
        }
        
        for (int s = 0; s < SECTORS_PER_CLUSTER && bytes_read < file_size; s++) {
            unsigned char sector[512];
            memset(sector, 0, 512);
            
            ata_lba_read(lba + s, 1, (unsigned short*)sector);
            
            unsigned int bytes_to_copy = 512;
            if (bytes_read + bytes_to_copy > file_size) {
                bytes_to_copy = file_size - bytes_read;
            }
            
            memcpy(buffer + bytes_read, sector, bytes_to_copy);
            bytes_read += bytes_to_copy;
        }
        
        unsigned int next_cluster = read_fat_entry(cluster);
        if (next_cluster >= 2 && next_cluster < FAT32_EOC && next_cluster == cluster) {
            printf("ATA: Circular cluster chain detected\n");
            break;
        }
        
        cluster = next_cluster;
    }

    *size = bytes_read;
    if (bytes_read < STORAGE_RECORD_SIZE) {
        buffer[bytes_read] = '\0';
    }
}

void delete_from_storage(const char* key)
{
    if (!storage_initialized) {
        return;
    }

    int found_idx = -1;
    for (int i = 0; i < root_dir_entry_count; i++) {
        char entry_name[256];
        extract_filename(&root_dir_cache[i], entry_name);
        
        if (filename_matches(entry_name, key)) {
            found_idx = i;
            break;
        }
    }

    if (found_idx < 0) {
        return;
    }

    unsigned int first_cluster = (root_dir_cache[found_idx].first_cluster_high << 16) | root_dir_cache[found_idx].first_cluster_low;
    if (first_cluster >= 2) {
        free_cluster_chain(first_cluster);
    }

    for (int i = found_idx; i < root_dir_entry_count - 1; i++) {
        memcpy(&root_dir_cache[i], &root_dir_cache[i + 1], sizeof(FAT32_DirEntry));
    }
    root_dir_entry_count--;
    save_root_directory();
}

int get_record_key(unsigned int index, char* key_out)
{
    if (!storage_initialized) {
        return 0;
    }
    if (index >= root_dir_entry_count) {
        return 0;
    }

    extract_filename(&root_dir_cache[index], key_out);
    return 1;
}

int files_exists(const char* key)
{
    if (!storage_initialized) {
        return 0;
    }
    
    for (int i = 0; i < root_dir_entry_count; i++) {
        char entry_name[256];
        extract_filename(&root_dir_cache[i], entry_name);
        
        if (filename_matches(entry_name, key)) {
            return 1;
        }
    }
    
    return 0;
}

int write_magic_number(unsigned int lba)
{
    printf("ATA: Formatting disk as FAT32...\n");
    
    unsigned int reserved_sectors = 32;
    unsigned int sectors_per_fat = 2048;
    unsigned int num_fats = 2;
    
    unsigned char boot_sector[512];
    memset(boot_sector, 0, 512);
    
    FAT32_BootSector* bs = (FAT32_BootSector*)boot_sector;
    
    bs->jmp_boot[0] = 0xEB;
    bs->jmp_boot[1] = 0x58;
    bs->jmp_boot[2] = 0x90;
    
    memcpy(bs->oem_name, "MULLOWAY", 8);
    bs->bytes_per_sector = 512;
    bs->sectors_per_cluster = SECTORS_PER_CLUSTER;
    bs->reserved_sectors = reserved_sectors;
    bs->num_fats = num_fats;
    bs->root_entry_count = 0;
    bs->total_sectors_16 = 0;
    bs->media_type = 0xF8;
    bs->fat_size_16 = 0;
    bs->sectors_per_track = 63;
    bs->num_heads = 255;
    bs->hidden_sectors = 0;
    bs->total_sectors_32 = 1000000;
    bs->fat_size_32 = sectors_per_fat;
    bs->ext_flags = 0;
    bs->fs_version = 0;
    bs->root_cluster = 2;
    bs->fs_info = 1;
    bs->backup_boot_sector = 6;
    bs->drive_number = 0x80;
    bs->boot_signature = 0x29;
    bs->volume_id = 0x12345678;
    memcpy(bs->volume_label, "MWK-GIP!   ", 11);
    memcpy(bs->fs_type, "FAT32   ", 8);
    
    boot_sector[510] = 0x55;
    boot_sector[511] = 0xAA;
    
    if (ata_lba_write_safe(lba, 1, (unsigned short*)boot_sector) != 0) {
        printf("ATA: Failed to write boot sector\n");
        return -1;
    }
    
    unsigned char fsinfo_sector[512];
    memset(fsinfo_sector, 0, 512);
    
    FAT32_FSInfo* fsinfo = (FAT32_FSInfo*)fsinfo_sector;
    fsinfo->lead_signature = 0x41615252;
    fsinfo->struct_signature = 0x61417272;
    fsinfo->free_count = 0xFFFFFFFF;
    fsinfo->next_free = 3;
    fsinfo->trail_signature = 0xAA550000;
    
    if (ata_lba_write_safe(lba + 1, 1, (unsigned short*)fsinfo_sector) != 0) {
        printf("ATA: Failed to write FSInfo sector\n");
        return -1;
    }
    
    if (ata_lba_write_safe(lba + 6, 1, (unsigned short*)boot_sector) != 0) {
        printf("ATA: Failed to write backup boot sector\n");
        return -1;
    }
    
    unsigned int fat1_lba = lba + reserved_sectors;
    unsigned int fat2_lba = fat1_lba + sectors_per_fat;
    
    unsigned char zero_sector[512];
    memset(zero_sector, 0, 512);
    
    for (unsigned int i = 0; i < sectors_per_fat; i++) {
        ata_lba_write_safe(fat1_lba + i, 1, (unsigned short*)zero_sector);
        ata_lba_write_safe(fat2_lba + i, 1, (unsigned short*)zero_sector);
    }
    
    unsigned char fat_start[512];
    memset(fat_start, 0, 512);
    unsigned int* fat_entries = (unsigned int*)fat_start;
    fat_entries[0] = 0x0FFFFFF8;
    fat_entries[1] = 0x0FFFFFFF;
    fat_entries[2] = 0x0FFFFFFF;
    
    ata_lba_write_safe(fat1_lba, 1, (unsigned short*)fat_start);
    ata_lba_write_safe(fat2_lba, 1, (unsigned short*)fat_start);
    
    unsigned int root_lba = fat2_lba + sectors_per_fat;
    for (int i = 0; i < SECTORS_PER_CLUSTER; i++) {
        ata_lba_write_safe(root_lba + i, 1, (unsigned short*)zero_sector);
    }
    
    printf("ATA: FAT32 format complete\n");
    return 0;
}
