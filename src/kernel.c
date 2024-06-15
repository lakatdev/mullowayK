#include <port.h>
#include <gdt.h>
#include <memory.h>
#include <interrupts.h>
#include <interface.h>
#include <graphics.h>
#include <mouse.h>
#include <desktop.h>

char boot_messages[80 * 25] = {0};
int boot_messages_len = 0;
char* get_boot_messages()
{
    return boot_messages;
}

void printf(const char* str)
{
    for (int i = 0; str[i] != 0; i++) {
        boot_messages[boot_messages_len] = str[i];
        boot_messages_len++;
    }
}

void print_hex(unsigned int n)
{
    char* foo = "0x00000000";
    char* hex = "0123456789ABCDEF";
    foo[2] = hex[(n >> 28) & 0xF];
    foo[3] = hex[(n >> 24) & 0xF];
    foo[4] = hex[(n >> 20) & 0xF];
    foo[5] = hex[(n >> 16) & 0xF];
    foo[6] = hex[(n >> 12) & 0xF];
    foo[7] = hex[(n >> 8) & 0xF];
    foo[8] = hex[(n >> 4) & 0xF];
    foo[9] = hex[n & 0xF];
    printf(foo);
}

void set_timer_freq(unsigned int divisor)
{
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void kernel_main(const void* multiboot_struct)
{
    printf("lasang please\n");
    init_gdt();
    enable_sse();

    // init memory manager with start position received from multiboot header
    unsigned int* header = (unsigned int*)(((unsigned int)multiboot_struct) + 8);
    // 10MB as begin heap address
    unsigned int heap = 10 * 1024 * 1024;

    init_memory(heap, (*header) * 1024 - heap - 10 * 1024);

    // memory info
    printf("Heap start:");
    print_hex(heap);
    printf("\n");
    printf("Available memory:");
    print_hex((*header) * 1024 - heap - 10 * 1024);
    printf("\n");

    // interrupts (idt table) and programmable interval timer
    set_timer_freq(1193);
    init_idt();

    // user interface
    unsigned char* buffer = (unsigned char*)header[20];
    init_graphics(buffer);
    init_mouse();
    draw_screen(0, 0, 0);
    invalidate();

    init_desktop();
}



