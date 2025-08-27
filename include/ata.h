#ifndef ATA_H
#define ATA_H

void ata_set_bases(unsigned short io, unsigned short ctl);
unsigned char ata_get_status();
unsigned char ata_get_altstatus();
void ata_select_drive(unsigned char v);
void ata_reset_channel();

void ata_lba_write(unsigned int lba, unsigned char sector_count, const unsigned short *data);
void ata_lba_read(unsigned int lba, unsigned char sector_count, unsigned short *data);
int ata_lba_read_safe(unsigned int lba, unsigned char sector_count, unsigned short *data);

#endif
