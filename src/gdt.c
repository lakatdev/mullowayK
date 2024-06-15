#include <gdt.h>
#include <interface.h>

unsigned int gdt_pointer = 0;
unsigned int gdt_size = 0;
unsigned int gdtr_loc = 0;

unsigned int highpart = 0;
unsigned int lowpart = 0;

extern void _set_gdtr();
extern void _reload_segments();

void init_gdt()
{
    gdt_pointer = 0x806; // start GDT data at 4MB
    gdtr_loc = 0x800;
    gdt_add_descriptor(0, 0);
    gdt_add_descriptor(1, 0x00CF9A000000FFFF);
    gdt_add_descriptor(2, 0x00CF92000000FFFF);
    gdt_add_descriptor(3, 0x008FFA000000FFFF); // 16bit code pl3
    gdt_set_descriptor(4, 0x008FF2000000FFFF); // 16bit data pl3
    printf("Global Descriptor Table is alive.\n");
}

int gdt_set_descriptor()
{
    /* GDTR
     * 0-1 = SIZE - 1
     * 2-5 = OFFSET
     */
    *(unsigned short*)gdtr_loc = (gdt_size - 1) & 0x0000FFFF;
    gdtr_loc += 2;
    *(unsigned int*)gdtr_loc = gdt_pointer;
    _set_gdtr();
    printf("GDTR was set\n");
    _reload_segments();
    printf("Segments reloaded\n");
    return 0;
}

int gdt_add_descriptor(unsigned char id, unsigned long long int desc)
{
    unsigned int loc = gdt_pointer + sizeof(unsigned long long int)*id;
    *(unsigned long long int*)loc = desc;
    printf("Added GDT entry\n");
    gdt_size += sizeof(desc);
    return 0;
}

unsigned long long int gdt_create_descriptor(unsigned int base, unsigned int limit, unsigned short flag)
{
    unsigned long long int desc = 0;
    highpart = 0;
    lowpart = 0;
    desc = limit & 0x000F0000;
    desc |= (flag << 8) & 0x00F0FF00;
    desc |= (base >> 16) & 0x000000FF;
    desc |= base & 0xFF000000;

    highpart = desc;
    desc <<= 32;

    desc |= base << 16;
    desc |= limit & 0x0000FFFF;
    lowpart = (unsigned int)desc;
    return desc;
}
