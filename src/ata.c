#include <port.h>
#include <interface.h>

#define ATA_CMD_READ   0x20
#define ATA_CMD_WRITE  0x30
#define ATA_TIMEOUT    500000

unsigned short ATA_IO  = 0x1F0;
unsigned short ATA_CTL = 0x3F6;

void ata_set_bases(unsigned short io, unsigned short ctl)
{
    if (io) {
        ATA_IO  = io;
    }
    if (ctl) {
        ATA_CTL = ctl;
    } 
}

unsigned char ata_get_status()
{
    return inb(ATA_IO + 7);
}

unsigned char ata_get_altstatus()
{
    return inb(ATA_CTL);
}

void ata_select_drive(unsigned char v)
{
    outb(ATA_IO + 6, v);
}

static inline unsigned char ata_status()
{
    return inb(ATA_IO + 7);
}

static inline void ata_delay_400ns(void)
{
    (void)inb(ATA_CTL); (void)inb(ATA_CTL); (void)inb(ATA_CTL); (void)inb(ATA_CTL);
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

static int ata_wait_bsy_clear()
{
    unsigned int t = ATA_TIMEOUT * 2;
    while (t--) {
        if ((inb(ATA_IO + 7) & 0x80) == 0) return 0;
    }
    return -1;
}

void ata_reset_channel()
{
    outb(ATA_CTL, 0x04 | 0x02);
    ata_delay_400ns();
    outb(ATA_CTL, 0x02);
    ata_delay_400ns();

    ata_wait_bsy_clear();

    outb(ATA_IO + 6, 0xA0);
    ata_delay_400ns();
}

void ata_lba_write(unsigned int lba, unsigned char sector_count, const unsigned short *data)
{
    outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay_400ns();

    outb(ATA_IO + 2, sector_count);
    outb(ATA_IO + 3, (unsigned char)( lba        & 0xFF));
    outb(ATA_IO + 4, (unsigned char)((lba >>  8) & 0xFF));
    outb(ATA_IO + 5, (unsigned char)((lba >> 16) & 0xFF));

    outb(ATA_IO + 7, ATA_CMD_WRITE);

    for (unsigned char s = 0; s < sector_count; s++) {
        if (ata_wait(0, 0x80) < 0) { printf("ATA: BSY-clear timeout\n"); return; }
        if (ata_wait(0x08, 0) < 0) { printf("ATA: DRQ-set  timeout\n"); return; }

        const unsigned short *p = data + (s * 256);
        for (unsigned short i = 0; i < 256; i++) {
            outw(ATA_IO + 0, p[i]);
        }
    }

    if (ata_wait(0, 0x88) < 0) {
        printf("ATA: write-complete timeout\n");
    }
}

void ata_lba_read(unsigned int lba, unsigned char sector_count, unsigned short *data)
{
    outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO + 2, sector_count);
    outb(ATA_IO + 3, (unsigned char)( lba        & 0xFF));
    outb(ATA_IO + 4, (unsigned char)((lba >>  8) & 0xFF));
    outb(ATA_IO + 5, (unsigned char)((lba >> 16) & 0xFF));
    outb(ATA_IO + 7, ATA_CMD_READ);

    for (unsigned char s = 0; s < sector_count; s++) {
        if (ata_wait(0x08, 0x80) < 0) {
            printf("ATA: read timeout\n");
            return;
        }

        unsigned short *p = data + (s * 256);
        for (unsigned short i = 0; i < 256; i++) {
            p[i] = inw(ATA_IO + 0);
        }
    }
}

int ata_lba_read_safe(unsigned int lba, unsigned char sector_count, unsigned short *data)
{
    unsigned char status = ata_status();
    if (status == 0xFF) {
        return -1;
    }

    if (status & 0x01) {
        ata_reset_channel();
        status = ata_status();
        if (status == 0xFF) return -1;
    }

    if (ata_wait(0, 0x80) < 0) {
        return -1;
    }

    ata_lba_read(lba, sector_count, data);

    if (ata_wait(0, 0x80) < 0) {
        return -1;
    }

    return 0;
}
