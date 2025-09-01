#include <port.h>
#include <interface.h>

#define ATA_CMD_READ     0x20
#define ATA_CMD_WRITE    0x30
#define ATA_CMD_IDENTIFY 0xEC
#define ATA_TIMEOUT      500000

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
    unsigned int t = ATA_TIMEOUT * 10;
    while (t--) {
        unsigned char status = inb(ATA_IO + 7);
        if ((status & 0x80) == 0) {
            return 0;
        }
    }
    return -1;
}

void ata_reset_channel()
{
    outb(ATA_CTL, 0x04 | 0x02);
    ata_delay_400ns();
    outb(ATA_CTL, 0x02);
    ata_delay_400ns();
    if (ata_wait_bsy_clear() < 0) {
        printf("ATA: WARNING - BSY did not clear after reset\n");
    }
    outb(ATA_IO + 6, 0xA0);
    ata_delay_400ns();
}

int ata_identify_drive(unsigned char drive_select, unsigned short* identify_buffer)
{
    printf("ATA: Identifying drive 0x");
    print_hex(drive_select);
    printf("\n");
    outb(ATA_IO + 6, drive_select);
    ata_delay_400ns();
    outb(ATA_IO + 2, 0);
    outb(ATA_IO + 3, 0);
    outb(ATA_IO + 4, 0);
    outb(ATA_IO + 5, 0);
    outb(ATA_IO + 7, ATA_CMD_IDENTIFY);
    ata_delay_400ns();

    unsigned char status = inb(ATA_IO + 7);
    if (status == 0) {
        printf("ATA: No drive detected (status = 0)\n");
        return -1;
    }

    if (ata_wait_bsy_clear() < 0) {
        printf("ATA: BSY timeout during IDENTIFY\n");
        return -1;
    }

    unsigned char lbamid = inb(ATA_IO + 4);
    unsigned char lbahi = inb(ATA_IO + 5);
    if (lbamid != 0 || lbahi != 0) {
        printf("ATA: Not an ATA device (LBA mid=");
        print_hex(lbamid);
        printf(" high=");
        print_hex(lbahi);
        printf(")\n");
        return -1;
    }

    status = inb(ATA_IO + 7);
    while (!(status & 0x08) && !(status & 0x01) && (status & 0x80)) {
        status = inb(ATA_IO + 7);
    }

    if (status & 0x01) {
        printf("ATA: IDENTIFY command failed (ERR bit set)\n");
        return -1;
    }

    if (!(status & 0x08)) {
        printf("ATA: IDENTIFY - DRQ not set\n");
        return -1;
    }

    for (int i = 0; i < 256; i++) {
        identify_buffer[i] = inw(ATA_IO + 0);
    }

    printf("ATA: Drive identified successfully\n");
    return 0;
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

        if (ata_wait(0, 0x80) < 0) {
            return; 
        }
        if (ata_wait(0x08, 0) < 0) {
            return; 
        }

        unsigned char status = ata_status();
        if (status & 0x01) {
            printf("ATA: Error during write setup, status = ");
            print_hex(status);
            printf("\n");
            return;
        }

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
            printf("ATA: read timeout on sector ");
            print_hex(s);
            printf("\n");
            return;
        }

        unsigned char status = ata_status();
        if (status & 0x01) {
            printf("ATA: Error during read, status = ");
            print_hex(status);
            printf("\n");
            return;
        }

        unsigned short *p = data + (s * 256);
        for (unsigned short i = 0; i < 256; i++) {
            p[i] = inw(ATA_IO + 0);
        }
    }
}

int ata_lba_write_safe(unsigned int lba, unsigned char sector_count, const unsigned short *data)
{
    unsigned char status = ata_status();
    if (status == 0xFF) {
        printf("ATA: Controller not responding (0xFF)\n");
        return -1;
    }

    if (status & 0x01) {
        printf("ATA: Error bit set before write, resetting\n");
        ata_reset_channel();
        status = ata_status();
        if (status == 0xFF) {
            printf("ATA: Controller lost after reset\n");
            return -1;
        }
    }

    if (ata_wait(0, 0x80) < 0) {
        printf("ATA: Timeout waiting for BSY clear before write\n");
        return -1;
    }

    ata_lba_write(lba, sector_count, data);

    if (ata_wait(0, 0x88) < 0) {
        printf("ATA: Timeout waiting for write complete\n");
        return -1;
    }

    status = ata_status();
    if (status & 0x01) {
        printf("ATA: Error bit set after write, status = ");
        print_hex(status);
        printf("\n");
        return -1;
    }
    return 0;
}

int ata_lba_read_safe(unsigned int lba, unsigned char sector_count, unsigned short *data)
{
    unsigned char status = ata_status();
    if (status == 0xFF) {
        printf("ATA: Controller not responding (0xFF)\n");
        return -1;
    }

    if (status == 0x7F) {
        printf("ATA: Drive status 0x7F - possible drive not ready\n");
    }

    if (status & 0x01) {
        printf("ATA: Error bit set before read, resetting\n");
        ata_reset_channel();
        status = ata_status();
        if (status == 0xFF) {
            printf("ATA: Controller lost after reset\n");
            return -1;
        }
    }

    if (ata_wait(0, 0x80) < 0) {
        printf("ATA: Timeout waiting for BSY clear before read\n");
        return -1;
    }

    ata_lba_read(lba, sector_count, data);

    if (ata_wait(0, 0x80) < 0) {
        printf("ATA: Timeout waiting for operation complete\n");
        return -1;
    }

    status = ata_status();
    if (status & 0x01) {
        printf("ATA: Error bit set after read, status = ");
        print_hex(status);
        printf("\n");
        return -1;
    }

    return 0;
}
