#include <userlib.h>

void app_files_draw()
{
    draw_screen(255, 255, 255);
    draw_text(10, 40, "Files", 32, 0, 0, 0);
}

void app_files_key(char key)
{

}

void app_files_mouse(int x, int y)
{

}

void app_files_init()
{

}

unsigned char app_files_icon_60[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xc0, 0x00,
    0x00, 0x00, 0x01, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff,
    0xff, 0xe0, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00,
    0x00, 0xfc, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x1e,
    0x00, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x7f,
    0xc0, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x0f, 0xbc, 0x00, 0x01, 0xe0, 0x00,
    0x00, 0x01, 0xf3, 0xc0, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00,
    0x01, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xc0, 0x00, 0x1e, 0x00, 0x00, 0x00,
    0x7f, 0xfc, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x1e,
    0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x07, 0x83,
    0xff, 0xf0, 0x1e, 0x00, 0x00, 0x00, 0x78, 0x7f, 0xff, 0x81, 0xe0, 0x00,
    0x00, 0x07, 0x87, 0xff, 0xf8, 0x1e, 0x00, 0x00, 0x00, 0x78, 0x3f, 0xff,
    0x01, 0xe0, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
    0x78, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x1e,
    0x00, 0x00, 0x00, 0x78, 0x3f, 0xff, 0x01, 0xe0, 0x00, 0x00, 0x07, 0x87,
    0xff, 0xf8, 0x1e, 0x00, 0x00, 0x00, 0x78, 0x7f, 0xff, 0x81, 0xe0, 0x00,
    0x00, 0x07, 0x83, 0xff, 0xf0, 0x1e, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00,
    0x01, 0xe0, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
    0x78, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x07, 0x83, 0xff, 0xf0, 0x1e,
    0x00, 0x00, 0x00, 0x78, 0x7f, 0xff, 0x81, 0xe0, 0x00, 0x00, 0x07, 0x87,
    0xff, 0xf8, 0x1e, 0x00, 0x00, 0x00, 0x78, 0x3f, 0xff, 0x01, 0xe0, 0x00,
    0x00, 0x07, 0x80, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00,
    0x01, 0xe0, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
    0x78, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x1e,
    0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x07, 0x80,
    0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x01, 0xe0, 0x00,
    0x00, 0x07, 0x80, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff,
    0xff, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00,
    0x7f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xfc,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
