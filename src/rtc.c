#include <port.h>

unsigned char get_year() {
    outb(0x70, 0x09);
    return inb(0x71);
}

unsigned char get_month() {
    outb(0x70, 0x08);
    return inb(0x71);
}

unsigned char get_day() {
    outb(0x70, 0x07);
    return inb(0x71);
}

unsigned char get_weekday() {
    outb(0x70, 0x06);
    return inb(0x71);
}

unsigned char get_hour() {
    outb(0x70, 0x04);
    return inb(0x71);
}

unsigned char get_minute() {
    outb(0x70, 0x02);
    return inb(0x71);
}

unsigned char get_second() {
    outb(0x70, 0x00);
    return inb(0x71);
}
