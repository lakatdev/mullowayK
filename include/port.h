#ifndef PORT_H
#define PORT_H

static inline unsigned char inb(unsigned short port)
{
    unsigned char result;
    asm volatile("inb %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

static inline void outb(unsigned short port, unsigned char data)
{
    asm volatile("outb %0, %1" : : "a" (data), "Nd" (port));
}

static unsigned short inw(unsigned short port)
{
    unsigned short result;
    asm volatile("inw %1, %0" : "=a"(result) : "d"(port));
    return result;
}

#endif

