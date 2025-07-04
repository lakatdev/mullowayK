#include <serial.h>
#include <port.h>
#include <interface.h>
#include <memory.h>
#include <interrupts.h>

#define COM1 0x3F8
#define TICKS_PER_MS 1
#define TIMEOUT 1000

void init_serial()
{
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
    printf("COM1 port initialized.\n");
}

void com1_write(const unsigned char* data, unsigned int size)
{
    for (unsigned int i = 0; i < size; ++i) {
        while (!(inb(COM1 + 5) & 0x20));
        outb(COM1, data[i]);
    }
}

void com1_read(unsigned char* data, unsigned int size)
{
    for (unsigned int i = 0; i < size; ++i) {
        while (!(inb(COM1 + 5) & 0x01));
        data[i] = inb(COM1);
    }
}
