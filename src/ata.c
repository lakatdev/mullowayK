#include <port.h>
#include <interface.h>

#define ATA_CMD_WRITE 0x30
#define ATA_TIMEOUT 500000

static inline unsigned char ata_status(void)
{
    return inb(0x1F7);
}

static inline void ata_delay_400ns(void)
{
    inb(0x1F7);
    inb(0x1F7);
    inb(0x1F7);
    inb(0x1F7);
}

static int ata_wait(unsigned char mask_set, unsigned char mask_clear)
{
    unsigned int t = ATA_TIMEOUT;
    unsigned char st;
    while (t--) {
        st = ata_status();
        if (st & 0x01) {
            return -1;
        }
        if ((st & mask_set) == mask_set && (st & mask_clear) == 0) {
            return 0;
        }
    }
    return -1;
}

void ata_lba_write(unsigned int lba, unsigned char sector_count, const unsigned short *data)
{
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay_400ns();

    outb(0x1F2, sector_count);
    outb(0x1F3, (unsigned char)( lba        & 0xFF));
    outb(0x1F4, (unsigned char)((lba >>  8) & 0xFF));
    outb(0x1F5, (unsigned char)((lba >> 16) & 0xFF));

    outb(0x1F7, ATA_CMD_WRITE);

    for (unsigned char s = 0; s < sector_count; s++) {
        if (ata_wait(0, 0x80) < 0) { printf("BSY-clear timeout\n"); return; }
        if (ata_wait(0x08, 0) < 0) { printf("DRQ-set  timeout\n"); return; }
        const unsigned short *p = data + (s * 256);
        for (unsigned short i = 0; i < 256; i++) {
            outb(0x1F0, (unsigned char)( p[i]        & 0xFF ));
            outb(0x1F0, (unsigned char)((p[i] >> 8) & 0xFF ));
        }
    }

    if (ata_wait(0, 0x88) < 0) {
        printf("ATA write-complete timeout\n");
        return;
    }
}

void ata_lba_read(unsigned int lba, unsigned char sector_count, unsigned short *data)
{
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, sector_count);
    outb(0x1F3, (unsigned char)( lba        & 0xFF));
    outb(0x1F4, (unsigned char)((lba >>  8) & 0xFF));
    outb(0x1F5, (unsigned char)((lba >> 16) & 0xFF));
    outb(0x1F7, 0x20);

    for (unsigned char s = 0; s < sector_count; s++) {
        while (ata_status() & 0x80);
        if (ata_status() & 0x01) {
            return;
        }

        while (!(ata_status() & 0x08));
        unsigned short *p = data + (s * 256);
        for (unsigned short i = 0; i < 256; i++) {
            p[i] = inw(0x1F0);
        }
    }
}
