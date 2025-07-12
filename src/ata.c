#include <port.h>

void ata_lba_write(unsigned int lba, unsigned char sector_count, const unsigned short *data)
{
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, sector_count);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F7, 0x30);

    while (!(inb(0x1F7) & 0x08));

    for (unsigned short s = 0; s < sector_count; ++s) {
        for (unsigned short i = 0; i < 256; ++i) {
            unsigned short word = data[s * 256 + i];
            outb(0x1F0, word & 0xFF);
            outb(0x1F0, (word >> 8) & 0xFF);
        }

        if (s + 1 < sector_count) {
            while (!(inb(0x1F7) & 0x08));
        }
    }
}
