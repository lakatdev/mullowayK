#ifndef INTERRUPTS_H
#define INTERRUPTS_H

typedef struct {
    unsigned short offset_lowerbits;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short offset_higherbits;
} __attribute__((packed)) InterruptDescriptor;

void init_idt();
void init_pit(unsigned int frequency);
void sleep(unsigned long long int ticks);
unsigned long long int system_uptime();
unsigned long long int get_tick_count();

#endif
