#ifndef GDT_H
#define GDT_H

#define GDT_ACCESSED	0x01
#define GDT_READWRITE	0x02
#define GDT_DIRECTION	0x04
#define GDT_EXEC	    0x08
#define GDT_STATIC	    0x10
#define GDT_PRESENT	    0x20

extern void init_gdt();
extern int gdt_set_descriptor();
extern int gdt_add_descriptor(unsigned char id, unsigned long long int desc);
extern unsigned long long int gdt_create_descriptor(unsigned int base, unsigned int limit, unsigned short flag);

#endif
