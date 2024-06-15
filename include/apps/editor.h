#ifndef APP_EDITOR_H
#define APP_EDITOR_H

#include <graphics.h>

char app_editor_text[1024] = {0};
int app_editor_text_len = 0;

void app_editor_draw()
{
    draw_rect(30, 30, WIDTH - 60, HEIGHT - 150, 255, 255, 255);
    draw_text(40, 70, "Editor", 32, 0, 0, 0);
    draw_text(40, 120, app_editor_text, 20, 0, 0, 0);
}

void app_editor_key(char key)
{
    if (key == '\b') {
        if (app_editor_text_len > 0) {
            app_editor_text_len--;
            app_editor_text[app_editor_text_len] = 0;
        }
        return;
    }

    if (app_editor_text_len >= 1023) {
        return;
    }

    app_editor_text[app_editor_text_len] = key;
    app_editor_text_len++;
}

void app_editor_mouse(int x, int y)
{

}

void app_editor_init()
{

}

unsigned char app_editor_icon_60[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xdf, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1f, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x3e, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x7c, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80,
    0x0f, 0x80, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0xf8, 0x00, 0x00, 0x00,
    0x00, 0x3e, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x03, 0xe0,
    0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x1f,
    0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x01, 0xf0, 0x00, 0x00,
    0x00, 0x00, 0x7c, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x07,
    0xc0, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00,
    0x3e, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x03, 0xe0, 0x00,
    0x00, 0x00, 0x00, 0xf8, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00,
    0x0f, 0x80, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x01, 0xf0, 0x00, 0x00, 0x00,
    0x00, 0x3f, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x07, 0xc0,
    0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x03,
    0xbe, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79, 0xf3, 0xe0, 0x00, 0x00,
    0x00, 0x00, 0x07, 0x8f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x7f,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0x27, 0xf0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0xc0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00,
    0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#endif
