#include <graphics.h>

void crash(const char* message, int code)
{
    __asm__ volatile("cli");
    system_draw_screen(0, 0, 180);
    system_draw_text(50, 150, "Exception:", 24, 255, 255, 255);
    system_draw_text(200, 150, message, 24, 255, 255, 0);
    system_draw_text(50, 180, "System halted. Please restart the system.", 24, 255, 255, 255);
    force_draw();
    while(1) {
        __asm__ volatile("hlt");
    }
}

void exception_divide_error()
{
    crash("Division by Zero", 0);
}

void exception_debug()
{
    crash("Debug Exception", 1);
}

void exception_nmi()
{
    crash("Non-Maskable Interrupt", 2);
}

void exception_breakpoint()
{
    crash("Breakpoint", 3);
}

void exception_overflow()
{
    crash("Overflow", 4);
}

void exception_bound_range()
{
    crash("Bound Range Exceeded", 5);
}

void exception_invalid_opcode()
{
    crash("Invalid Opcode", 6);
}

void exception_device_not_available()
{
    crash("Device Not Available", 7);
}

void exception_double_fault()
{
    crash("Double Fault", 8);
}

void exception_invalid_tss()
{
    crash("Invalid TSS", 10);
}

void exception_segment_not_present()
{
    crash("Segment Not Present", 11);
}

void exception_stack_segment_fault()
{
    crash("Stack Segment Fault", 12);
}

void exception_general_protection()
{
    crash("General Protection Fault", 13);
}

void exception_page_fault()
{
    crash("Page Fault", 14);
}

void exception_fpu_error()
{
    crash("FPU Error", 16);
}

void exception_alignment_check()
{
    crash("Alignment Check", 17);
}

void exception_machine_check()
{
    crash("Machine Check", 18);
}

void exception_simd_exception()
{
    crash("SIMD Exception", 19);
}
