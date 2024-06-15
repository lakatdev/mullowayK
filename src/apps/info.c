#include <graphics.h>

void app_info_draw()
{
    draw_rect(30, 30, WIDTH - 60, HEIGHT - 150, 255, 255, 255);
    draw_text(40, 70, "Info", 32, 0, 0, 0);
}

void app_info_key(char key)
{

}

void app_info_mouse(int x, int y)
{

}

void app_info_init()
{

}

unsigned char app_info_icon_60[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xf0, 0x00, 0x00,
    0x00, 0x03, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
    0xff, 0xe0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x03,
    0xe0, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x03,
    0xc0, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0xe0, 0x00,
    0x00, 0x00, 0x00, 0xe0, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00,
    0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x1c, 0x00, 0x00, 0xf0,
    0x00, 0x07, 0x00, 0x01, 0xc0, 0x00, 0x1f, 0x80, 0x00, 0x70, 0x00, 0x1c,
    0x00, 0x01, 0xf8, 0x00, 0x07, 0x00, 0x03, 0xc0, 0x00, 0x1f, 0x80, 0x00,
    0x78, 0x00, 0x3c, 0x00, 0x01, 0xf8, 0x00, 0x07, 0x80, 0x03, 0xc0, 0x00,
    0x1f, 0x80, 0x00, 0x78, 0x00, 0x3c, 0x00, 0x00, 0x70, 0x00, 0x07, 0x80,
    0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x07, 0x80, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x3c,
    0x00, 0x0f, 0xf8, 0x00, 0x07, 0x80, 0x03, 0xc0, 0x00, 0xff, 0x80, 0x00,
    0x78, 0x00, 0x3c, 0x00, 0x03, 0xf8, 0x00, 0x07, 0x80, 0x03, 0xc0, 0x00,
    0x1f, 0x80, 0x00, 0x78, 0x00, 0x3c, 0x00, 0x01, 0xf8, 0x00, 0x07, 0x80,
    0x03, 0xc0, 0x00, 0x1f, 0x80, 0x00, 0x78, 0x00, 0x3c, 0x00, 0x01, 0xf8,
    0x00, 0x07, 0x80, 0x03, 0xc0, 0x00, 0x1f, 0x80, 0x00, 0x78, 0x00, 0x3c,
    0x00, 0x01, 0xf8, 0x00, 0x07, 0x80, 0x03, 0xc0, 0x00, 0x1f, 0x80, 0x00,
    0x78, 0x00, 0x3c, 0x00, 0x01, 0xf8, 0x00, 0x07, 0x80, 0x03, 0xc0, 0x00,
    0x1f, 0x80, 0x00, 0x78, 0x00, 0x3c, 0x00, 0x01, 0xf8, 0x00, 0x07, 0x80,
    0x03, 0xc0, 0x00, 0x1f, 0x80, 0x00, 0x78, 0x00, 0x1c, 0x00, 0x01, 0xf8,
    0x00, 0x07, 0x00, 0x01, 0xc0, 0x00, 0x1f, 0xc0, 0x00, 0x70, 0x00, 0x1c,
    0x00, 0x0f, 0xff, 0x00, 0x07, 0x00, 0x01, 0xc0, 0x00, 0xff, 0xf0, 0x00,
    0x70, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0xe0, 0x00,
    0x00, 0x00, 0x00, 0xe0, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x00, 0x78, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xe0, 0x00, 0x00,
    0x00, 0xf8, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xf8,
    0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
