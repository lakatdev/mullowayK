#include <serial.h>
#include <port.h>

void init_serial()
{
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

void serial_write(const uint8_t* data, unsigned int size)
{
    for (uint32_t i = 0; i < len; ++i) {
        while (!(inb(COM1 + 5) & 0x20));
        outb(COM1, data[i]);
    }
}
