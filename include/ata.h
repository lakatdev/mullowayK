#ifndef ATA_H
#define ATA_H

void ata_lba_write(unsigned int lba, unsigned char sector_count, const unsigned short *data);
void ata_lba_read(unsigned int lba, unsigned char sector_count, unsigned short *data);

#endif
