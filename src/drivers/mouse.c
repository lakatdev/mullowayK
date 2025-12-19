#include <system/port.h>
#include <tools/graphics.h>
#include <system/desktop.h>

unsigned char mouse_cycle;
char mouse_byte[3];
char mouse_x, mouse_y;
int abs_x = WIDTH / 2;
int abs_y = HEIGHT / 2;
char mouse_left = 0;

unsigned char cursor_outline[63] = {
    0xc0, 0x00, 0x38, 0x00, 0x0f, 0x00, 0x03, 0xe0, 0x00, 0xdc, 0x00, 0x33,
    0x80, 0x0c, 0x70, 0x03, 0x0e, 0x00, 0xc1, 0xc0, 0x30, 0x38, 0x0c, 0x07,
    0x03, 0x00, 0xe0, 0xc0, 0x1c, 0x30, 0x03, 0x8c, 0x00, 0x73, 0x00, 0x1c,
    0xc0, 0x3f, 0x31, 0x9f, 0x8c, 0xe6, 0x03, 0x78, 0xc0, 0xfb, 0x30, 0x3c,
    0xc6, 0x0c, 0x19, 0xc0, 0x06, 0x30, 0x00, 0x8c, 0x00, 0x37, 0x00, 0x0f,
    0x80, 0x01, 0x80
};

unsigned char cursor_back[63] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x0c,
    0x00, 0x03, 0x80, 0x00, 0xf0, 0x00, 0x3e, 0x00, 0x0f, 0xc0, 0x03, 0xf8,
    0x00, 0xff, 0x00, 0x3f, 0xe0, 0x0f, 0xfc, 0x03, 0xff, 0x80, 0xff, 0xe0,
    0x3f, 0xc0, 0x0e, 0x60, 0x03, 0x18, 0x00, 0x87, 0x00, 0x00, 0xc0, 0x00,
    0x38, 0x00, 0x06, 0x00, 0x01, 0xc0, 0x00, 0x70, 0x00, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00
};

void handle_mouse()
{
    switch (mouse_cycle) {
        case (0): {
            mouse_byte[0] = inb(0x60);
            mouse_cycle++;
            break;
        }
        case (1): {
            mouse_byte[1] = inb(0x60);
            mouse_cycle++;
            break;
        }
        case (2): {
            mouse_byte[2] = inb(0x60);
            mouse_x = mouse_byte[1];
            mouse_y = mouse_byte[2];
            mouse_cycle = 0;
            break;
        }
    }
    abs_x += mouse_x;
    abs_y -= mouse_y;

    if (abs_x < 0) {
        abs_x = 0;
    }

    if (abs_x >= WIDTH) {
        abs_x = WIDTH - 1;
    }

    if (abs_y < 0) {
        abs_y = 0;
    }

    if (abs_y >= HEIGHT) {
        abs_y = HEIGHT - 1;
    }

    if (mouse_byte[0] & 0x1) {
        if (!mouse_left) {
            mouse_click(abs_x, abs_y);
        }
        mouse_left = 1;
    }
    else {
        mouse_left = 0;
    }

    invalidate();
}

void draw_cursor()
{
    system_draw_image(abs_x, abs_y, 18, 28, cursor_outline, 0, 0, 0);
    system_draw_image(abs_x, abs_y, 18, 28, cursor_back, 255, 255, 255);
}

void mouse_wait(unsigned char type)
{
    unsigned int time_out = 100000;
    if (type == 0) {
        while (time_out--) {
            if ((inb(0x64) & 1) == 1) {
                return;
            }
        }
        return;
    }
    else {
        while (time_out--) {
            if ((inb(0x64) & 2) == 0) {
                return;
            }
        }
        return;
    }
}

int get_mouse_x()
{
    return abs_x;
}

int get_mouse_y()
{
    return abs_y;
}

void set_mouse_pos(int x, int y)
{
    if (x < 0) {
        x = 0;
    }
    if (x >= WIDTH) {
        x = WIDTH - 1;
    }
    if (y < 0) {
        y = 0;
    }
    if (y >= HEIGHT) {
        y = HEIGHT - 1;
    }
    abs_x = x;
    abs_y = y;
}

void mouse_write(unsigned char write)
{
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, write);
}

unsigned char mouse_read()
{
    mouse_wait(0);
    return inb(0x60);
}

void init_mouse()
{
    unsigned char status;

    mouse_wait(1);
    outb(0x64, 0xA8);
    mouse_write(0xF6);
    mouse_read();
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);
    mouse_write(0xF4);
    mouse_read();
    
    mouse_cycle = 0;
}
