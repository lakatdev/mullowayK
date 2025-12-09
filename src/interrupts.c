#include <interrupts.h>
#include <interface.h>
#include <port.h>
#include <graphics.h>
#include <mouse.h>
#include <desktop.h>
#include <keyboard.h>
#include <exceptions.h>
#include <apps/runtime.h>

InterruptDescriptor idt[256];

void init_idt()
{
    printf("Initializing interrupt descriptor table\n");

    extern int exception0();
    extern int exception1();
    extern int exception2();
    extern int exception3();
    extern int exception4();
    extern int exception5();
    extern int exception6();
    extern int exception7();
    extern int exception8();
    extern int exception10();
    extern int exception11();
    extern int exception12();
    extern int exception13();
    extern int exception14();
    extern int exception16();
    extern int exception17();
    extern int exception18();
    extern int exception19();

    extern int load_idt();
    extern int irq0();
    extern int irq1();
    extern int irq2();
    extern int irq3();
    extern int irq4();
    extern int irq5();
    extern int irq6();
    extern int irq7();
    extern int irq8();
    extern int irq9();
    extern int irq10();
    extern int irq11();
    extern int irq12();
    extern int irq13();
    extern int irq14();
    extern int irq15();

    unsigned int exception_address;

    exception_address = (unsigned int)exception0;
    idt[0].offset_lowerbits = exception_address & 0xffff;
    idt[0].selector = 0x08;
    idt[0].zero = 0;
    idt[0].type_attr = 0x8e;
    idt[0].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception1;
    idt[1].offset_lowerbits = exception_address & 0xffff;
    idt[1].selector = 0x08;
    idt[1].zero = 0;
    idt[1].type_attr = 0x8e;
    idt[1].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception2;
    idt[2].offset_lowerbits = exception_address & 0xffff;
    idt[2].selector = 0x08;
    idt[2].zero = 0;
    idt[2].type_attr = 0x8e;
    idt[2].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception3;
    idt[3].offset_lowerbits = exception_address & 0xffff;
    idt[3].selector = 0x08;
    idt[3].zero = 0;
    idt[3].type_attr = 0x8e;
    idt[3].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception4;
    idt[4].offset_lowerbits = exception_address & 0xffff;
    idt[4].selector = 0x08;
    idt[4].zero = 0;
    idt[4].type_attr = 0x8e;
    idt[4].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception5;
    idt[5].offset_lowerbits = exception_address & 0xffff;
    idt[5].selector = 0x08;
    idt[5].zero = 0;
    idt[5].type_attr = 0x8e;
    idt[5].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception6;
    idt[6].offset_lowerbits = exception_address & 0xffff;
    idt[6].selector = 0x08;
    idt[6].zero = 0;
    idt[6].type_attr = 0x8e;
    idt[6].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception7;
    idt[7].offset_lowerbits = exception_address & 0xffff;
    idt[7].selector = 0x08;
    idt[7].zero = 0;
    idt[7].type_attr = 0x8e;
    idt[7].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception8;
    idt[8].offset_lowerbits = exception_address & 0xffff;
    idt[8].selector = 0x08;
    idt[8].zero = 0;
    idt[8].type_attr = 0x8e;
    idt[8].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception10;
    idt[10].offset_lowerbits = exception_address & 0xffff;
    idt[10].selector = 0x08;
    idt[10].zero = 0;
    idt[10].type_attr = 0x8e;
    idt[10].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception11;
    idt[11].offset_lowerbits = exception_address & 0xffff;
    idt[11].selector = 0x08;
    idt[11].zero = 0;
    idt[11].type_attr = 0x8e;
    idt[11].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception12;
    idt[12].offset_lowerbits = exception_address & 0xffff;
    idt[12].selector = 0x08;
    idt[12].zero = 0;
    idt[12].type_attr = 0x8e;
    idt[12].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception13;
    idt[13].offset_lowerbits = exception_address & 0xffff;
    idt[13].selector = 0x08;
    idt[13].zero = 0;
    idt[13].type_attr = 0x8e;
    idt[13].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception14;
    idt[14].offset_lowerbits = exception_address & 0xffff;
    idt[14].selector = 0x08;
    idt[14].zero = 0;
    idt[14].type_attr = 0x8e;
    idt[14].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception16;
    idt[16].offset_lowerbits = exception_address & 0xffff;
    idt[16].selector = 0x08;
    idt[16].zero = 0;
    idt[16].type_attr = 0x8e;
    idt[16].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception17;
    idt[17].offset_lowerbits = exception_address & 0xffff;
    idt[17].selector = 0x08;
    idt[17].zero = 0;
    idt[17].type_attr = 0x8e;
    idt[17].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception18;
    idt[18].offset_lowerbits = exception_address & 0xffff;
    idt[18].selector = 0x08;
    idt[18].zero = 0;
    idt[18].type_attr = 0x8e;
    idt[18].offset_higherbits = (exception_address & 0xffff0000) >> 16;
    
    exception_address = (unsigned int)exception19;
    idt[19].offset_lowerbits = exception_address & 0xffff;
    idt[19].selector = 0x08;
    idt[19].zero = 0;
    idt[19].type_attr = 0x8e;
    idt[19].offset_higherbits = (exception_address & 0xffff0000) >> 16;

    unsigned int irq0_address;
    unsigned int irq1_address;
    unsigned int irq2_address;
    unsigned int irq3_address;
    unsigned int irq4_address;
    unsigned int irq5_address;
    unsigned int irq6_address;
    unsigned int irq7_address;
    unsigned int irq8_address;
    unsigned int irq9_address;
    unsigned int irq10_address;
    unsigned int irq11_address;
    unsigned int irq12_address;
    unsigned int irq13_address;
    unsigned int irq14_address;
    unsigned int irq15_address;
    unsigned int idt_address;
    unsigned int idt_ptr[2];

    printf("Remapping PIC\n");
    /* remapping the PIC */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 40);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    irq0_address = (unsigned int)irq0;
    idt[32].offset_lowerbits = irq0_address & 0xffff;
    idt[32].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[32].zero = 0;
    idt[32].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[32].offset_higherbits = (irq0_address & 0xffff0000) >> 16;

    irq1_address = (unsigned int)irq1;
    idt[33].offset_lowerbits = irq1_address & 0xffff;
    idt[33].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[33].zero = 0;
    idt[33].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[33].offset_higherbits = (irq1_address & 0xffff0000) >> 16;

    irq2_address = (unsigned int)irq2;
    idt[34].offset_lowerbits = irq2_address & 0xffff;
    idt[34].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[34].zero = 0;
    idt[34].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[34].offset_higherbits = (irq2_address & 0xffff0000) >> 16;

    irq3_address = (unsigned int)irq3;
    idt[35].offset_lowerbits = irq3_address & 0xffff;
    idt[35].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[35].zero = 0;
    idt[35].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[35].offset_higherbits = (irq3_address & 0xffff0000) >> 16;

    irq4_address = (unsigned int)irq4;
    idt[36].offset_lowerbits = irq4_address & 0xffff;
    idt[36].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[36].zero = 0;
    idt[36].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[36].offset_higherbits = (irq4_address & 0xffff0000) >> 16;

    irq5_address = (unsigned int)irq5;
    idt[37].offset_lowerbits = irq5_address & 0xffff;
    idt[37].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[37].zero = 0;
    idt[37].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[37].offset_higherbits = (irq5_address & 0xffff0000) >> 16;

    irq6_address = (unsigned int)irq6;
    idt[38].offset_lowerbits = irq6_address & 0xffff;
    idt[38].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[38].zero = 0;
    idt[38].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[38].offset_higherbits = (irq6_address & 0xffff0000) >> 16;

    irq7_address = (unsigned int)irq7;
    idt[39].offset_lowerbits = irq7_address & 0xffff;
    idt[39].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[39].zero = 0;
    idt[39].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[39].offset_higherbits = (irq7_address & 0xffff0000) >> 16;

    irq8_address = (unsigned int)irq8;
    idt[40].offset_lowerbits = irq8_address & 0xffff;
    idt[40].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[40].zero = 0;
    idt[40].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[40].offset_higherbits = (irq8_address & 0xffff0000) >> 16;

    irq9_address = (unsigned int)irq9;
    idt[41].offset_lowerbits = irq9_address & 0xffff;
    idt[41].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[41].zero = 0;
    idt[41].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[41].offset_higherbits = (irq9_address & 0xffff0000) >> 16;

    irq10_address = (unsigned int)irq10;
    idt[42].offset_lowerbits = irq10_address & 0xffff;
    idt[42].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[42].zero = 0;
    idt[42].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[42].offset_higherbits = (irq10_address & 0xffff0000) >> 16;

    irq11_address = (unsigned int)irq11;
    idt[43].offset_lowerbits = irq11_address & 0xffff;
    idt[43].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[43].zero = 0;
    idt[43].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[43].offset_higherbits = (irq11_address & 0xffff0000) >> 16;

    irq12_address = (unsigned int)irq12;
    idt[44].offset_lowerbits = irq12_address & 0xffff;
    idt[44].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[44].zero = 0;
    idt[44].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[44].offset_higherbits = (irq12_address & 0xffff0000) >> 16;

    irq13_address = (unsigned int)irq13;
    idt[45].offset_lowerbits = irq13_address & 0xffff;
    idt[45].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[45].zero = 0;
    idt[45].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[45].offset_higherbits = (irq13_address & 0xffff0000) >> 16;

    irq14_address = (unsigned int)irq14;
    idt[46].offset_lowerbits = irq14_address & 0xffff;
    idt[46].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[46].zero = 0;
    idt[46].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[46].offset_higherbits = (irq14_address & 0xffff0000) >> 16;

    irq15_address = (unsigned int)irq15;
    idt[47].offset_lowerbits = irq15_address & 0xffff;
    idt[47].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    idt[47].zero = 0;
    idt[47].type_attr = 0x8e; /* INTERRUPT_GATE */
    idt[47].offset_higherbits = (irq15_address & 0xffff0000) >> 16;

    /* fill the idt descriptor */
    idt_address = (unsigned int)idt ;
    idt_ptr[0] = (sizeof (InterruptDescriptor) * 256) + ((idt_address & 0xffff) << 16);
    idt_ptr[1] = idt_address >> 16;

    printf("Loading IDT");
    load_idt(idt_ptr);
    printf (" [DONE]\n");
}

unsigned long long int uptime = 0;
unsigned long long int timer_ticks = 0;
void sleep(unsigned long long int ticks)
{
    timer_ticks = ticks;
    while (timer_ticks > 0);
}

unsigned long long int system_uptime()
{
    return uptime;
}

void irq0_handler(void)
{
    uptime++;
    
    if ((unsigned int)uptime % 200 == 0) {
        invalidate();
    }

    if ((unsigned int)uptime % 15 == 0) {
        update_video();
    }



    if (timer_ticks > 0) {
        timer_ticks--;
    }
    outb(0x20, 0x20); //EOI
}

void irq1_handler(void)
{
    handle_keyboard(inb(0x60));
    outb(0x20, 0x20); //EOI
}

void irq2_handler(void)
{
    outb(0x20, 0x20); //EOI
}

void irq3_handler(void)
{
    outb(0x20, 0x20); //EOI
}

void irq4_handler(void)
{
    outb(0x20, 0x20); //EOI
}

void irq5_handler(void)
{
    outb(0x20, 0x20); //EOI
}

void irq6_handler(void)
{
    outb(0x20, 0x20); //EOI
}

void irq7_handler(void)
{
    outb(0x20, 0x20); //EOI
}

void irq8_handler(void)
{
    outb(0xA0, 0x20);
    outb(0x20, 0x20); //EOI
}

void irq9_handler(void)
{
    outb(0xA0, 0x20);
    outb(0x20, 0x20); //EOI
}

void irq10_handler(void)
{
    outb(0xA0, 0x20);
    outb(0x20, 0x20); //EOI
}

void irq11_handler(void)
{
    outb(0xA0, 0x20);
    outb(0x20, 0x20); //EOI
}

void irq12_handler(void)
{
    handle_mouse();
    outb(0xA0, 0x20);
    outb(0x20, 0x20); //EOI
}

void irq13_handler(void)
{
    outb(0xA0, 0x20);
    outb(0x20, 0x20); //EOI
}

void irq14_handler(void)
{
    outb(0xA0, 0x20);
    outb(0x20, 0x20); //EOI
}

void irq15_handler(void)
{
    outb(0xA0, 0x20);
    outb(0x20, 0x20); //EOI
}

