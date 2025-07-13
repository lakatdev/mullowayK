#include <ata.h>
#include <storage.h>
#include <memory.h>

#define MAX_DATA_LBAS ((STORAGE_RECORD_SIZE + 511) / 512)

unsigned int first_lba = 0;
char storage_initialized = 0;

void init_storage(unsigned int start_address)
{
    storage_initialized = 0;
    first_lba = start_address;

    char magic_sector[512] = {0};
    ata_lba_read(first_lba, 1, (unsigned short*)magic_sector);

    if (magic_sector[0] == 'G' && magic_sector[1] == 'I' && magic_sector[2] == 'P' && magic_sector[3] == '!') {
        unsigned int record_size = (magic_sector[4] << 24) | (magic_sector[5] << 16) | (magic_sector[6] << 8) | magic_sector[7];
        unsigned int key_size = (magic_sector[8] << 24) | (magic_sector[9] << 16) | (magic_sector[10] << 8) | magic_sector[11];
        unsigned int record_count = (magic_sector[12] << 24) | (magic_sector[13] << 16) | (magic_sector[14] << 8) | magic_sector[15];

        if (record_size == STORAGE_RECORD_SIZE && key_size == STORAGE_KEY_SIZE) {
            storage_initialized = 1;
        }
    }
}

int get_record_count()
{
    if (!storage_initialized) {
        return 0;
    }
    char magic[512];
    ata_lba_read(first_lba, 1, (unsigned short*)magic);
    return (magic[12] << 24) | (magic[13] << 16) | (magic[14] <<  8) |  magic[15];
}

void set_record_count(unsigned int count)
{
    if (!storage_initialized) {
        return;
    }
    char magic[512];
    ata_lba_read(first_lba, 1, (unsigned short*)magic);
    magic[12] = (count >> 24) & 0xFF;
    magic[13] = (count >> 16) & 0xFF;
    magic[14] = (count >>  8) & 0xFF;
    magic[15] =  count        & 0xFF;
    ata_lba_write(first_lba, 1, (unsigned short*)magic);
}

void write_to_storage(const char* key, const char* data, unsigned int size)
{
    if (!storage_initialized || size > STORAGE_RECORD_SIZE)
    {
        return;
    }

    unsigned int records = get_record_count();
    unsigned int slot = 0;

    while (1) {
        unsigned int hdr_lba = first_lba + 1 + slot * (1 + MAX_DATA_LBAS);
        StorageRecordHeader hdr;
        ata_lba_read(hdr_lba, 1, (unsigned short*)&hdr);
        if (!hdr.valid) {
            break;
        }
        slot++;
    }

    unsigned int hdr_lba = first_lba + 1 + slot * (1 + MAX_DATA_LBAS);
    StorageRecordHeader out = {0};
    strncpy(out.key, key, STORAGE_KEY_SIZE);
    out.valid = 1;
    out.size  = size;
    ata_lba_write(hdr_lba, 1, (unsigned short*)&out);

    unsigned int sectors = (size + 511) / 512;
    for (unsigned int i = 0; i < MAX_DATA_LBAS; i++) {
        char buf[512] = {0};
        if (i < sectors) {
            unsigned int chunk = (i + 1 < sectors) ? 512 : (size - i * 512);
            memcpy(buf, data + i * 512, chunk);
        }
        ata_lba_write(hdr_lba + 1 + i, 1, (unsigned short*)buf);
    }

    set_record_count(records + 1);
}

void read_from_storage(const char* key, char* buffer, unsigned int* size)
{
    *size = 0;
    if (!storage_initialized) {
        return;
    }

    unsigned int rec_count = get_record_count();
    unsigned int found = 0;
    unsigned int slot  = 0;

    while (found < rec_count) {
        unsigned int hdr_lba = first_lba + 1 + slot * (1 + MAX_DATA_LBAS);
        StorageRecordHeader hdr;
        ata_lba_read(hdr_lba, 1, (unsigned short*)&hdr);

        if (hdr.valid) {
            found++;
            if (strcmp(hdr.key, key) == 0) {
                unsigned int sz      = hdr.size;
                unsigned int sectors = (sz + 511) / 512;

                for (unsigned int s = 0; s < sectors; s++) {
                    char chunk[512];
                    ata_lba_read(hdr_lba + 1 + s, 1, (unsigned short*)chunk);
                    unsigned int copy_sz = (s + 1 < sectors) ? 512 : (sz - s * 512);
                    memcpy(buffer + s * 512, chunk, copy_sz);
                }

                *size = sz;
                return;
            }
        }
        slot++;
    }
}

void delete_from_storage(const char* key)
{
    if (!storage_initialized) {
        return;
    }

    unsigned int records = get_record_count();
    unsigned int found = 0, slot = 0;

    while (found < records) {
        unsigned int hdr_lba = first_lba + 1 + slot * (1 + MAX_DATA_LBAS);
        StorageRecordHeader hdr;
        ata_lba_read(hdr_lba, 1, (unsigned short*)&hdr);
        if (hdr.valid) {
            found++;
            if (strcmp(hdr.key, key) == 0) {
                hdr.valid = 0;
                ata_lba_write(hdr_lba, 1, (unsigned short*)&hdr);
                set_record_count(records - 1);
                break;
            }
        }
        slot++;
    }
}

int get_record_key(unsigned int index, char* key_out)
{
    if (!storage_initialized) {
        return 0;
    }
    unsigned int rec_count = get_record_count();
    if (index >= rec_count) {
        return 0;
    }

    unsigned int found = 0, slot = 0;
    while (found <= index) {
        unsigned int hdr_lba = first_lba + 1 + slot * (1 + MAX_DATA_LBAS);
        StorageRecordHeader hdr;
        ata_lba_read(hdr_lba, 1, (unsigned short*)&hdr);
        if (hdr.valid) {
            if (found == index) {
                memcpy(key_out, hdr.key, STORAGE_KEY_SIZE);
                key_out[STORAGE_KEY_SIZE - 1] = '\0';
                return 1;
            }
            found++;
        }
        slot++;
    }
    return 0;
}

int files_exists(const char* key)
{
    if (!storage_initialized) {
        return 0;
    }

    unsigned int records = get_record_count();
    unsigned int found = 0, slot = 0;

    while (found < records) {
        unsigned int hdr_lba = first_lba + 1 + slot * (1 + MAX_DATA_LBAS);
        StorageRecordHeader hdr;
        ata_lba_read(hdr_lba, 1, (unsigned short*)&hdr);
        if (hdr.valid) {
            found++;
            if (strcmp(hdr.key, key) == 0) {
                return 1;
            }
        }
        slot++;
    }
    return 0;
}

void write_magic_number(unsigned int lba)
{
    char magic_sector[512] = {0};

    // magic number
    magic_sector[0] = 'G';
    magic_sector[1] = 'I';
    magic_sector[2] = 'P';
    magic_sector[3] = '!';

    // record size
    magic_sector[4] = (STORAGE_RECORD_SIZE >> 24) & 0xFF;
    magic_sector[5] = (STORAGE_RECORD_SIZE >> 16) & 0xFF;
    magic_sector[6] = (STORAGE_RECORD_SIZE >> 8) & 0xFF;
    magic_sector[7] = STORAGE_RECORD_SIZE & 0xFF;

    // key size
    magic_sector[8] = (STORAGE_KEY_SIZE >> 24) & 0xFF;
    magic_sector[9] = (STORAGE_KEY_SIZE >> 16) & 0xFF;
    magic_sector[10] = (STORAGE_KEY_SIZE >> 8) & 0xFF;
    magic_sector[11] = STORAGE_KEY_SIZE & 0xFF;

    // number of records
    magic_sector[12] = (0 >> 24) & 0xFF;
    magic_sector[13] = (0 >> 16) & 0xFF;
    magic_sector[14] = (0 >> 8) & 0xFF;
    magic_sector[15] = 0 & 0xFF;

    ata_lba_write(lba, 1, (const short unsigned int*)magic_sector);
}
