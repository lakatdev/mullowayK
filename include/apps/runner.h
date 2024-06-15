#ifndef APP_RUNNER_H
#define APP_RUNNER_H

#include <graphics.h>
#include <interface.h>

#define APP_RUNNER_WIDTH 80
#define APP_RUNNER_HEIGHT 25

int app_runner_x = 0;
int app_runner_y = 0;
char app_runner_video[APP_RUNNER_WIDTH * APP_RUNNER_HEIGHT] = {0};

void app_runner_scroll_video()
{
    for (int count = APP_RUNNER_WIDTH; count < APP_RUNNER_WIDTH * APP_RUNNER_HEIGHT; count++) {
        app_runner_video[count - APP_RUNNER_WIDTH] = app_runner_video[count];
        app_runner_video[count] = 0;
    }
}

void app_runner_clear_video_memory()
{
    for (int count = 0; count < APP_RUNNER_WIDTH * APP_RUNNER_HEIGHT; count++) {
        app_runner_video[count] = 0;
    }
}

void app_runner_printc(char c)
{
    switch (c) {
        case '\n': {
            app_runner_x = 0;
            app_runner_y++;
            break;
        }
        case 0x3: {
            app_runner_x = 0;
            app_runner_y = 0;
            app_runner_clear_video_memory();
            break;
        }   
        case '\b': {
            if (app_runner_x > 0) {
                app_runner_x--;
                app_runner_video[app_runner_y * APP_RUNNER_WIDTH + app_runner_x] = 0;
            }
            break;
        }
        default: {
            app_runner_video[app_runner_y * APP_RUNNER_WIDTH + app_runner_x] = c;
            app_runner_x++;
            break;
        }
    }

    if (app_runner_x >= 80) {
        app_runner_x = 0;
        app_runner_y++;
    }

    if (app_runner_y >= 25) {
        app_runner_scroll_video();
        app_runner_x = 0;
        app_runner_y = 24;
    }
}

void app_runner_printf(const char* str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        app_runner_printc(str[i]);
    }
}

void app_runner_draw()
{
    draw_rect(30, 30, 10 * APP_RUNNER_WIDTH, 20 * APP_RUNNER_HEIGHT, 0, 0, 0);
    for (int i = 0; i < APP_RUNNER_HEIGHT; i++) {
        char line[APP_RUNNER_WIDTH + 1] = {0};
        memcpy(line, &app_runner_video[i * APP_RUNNER_WIDTH], APP_RUNNER_WIDTH);
        draw_text(30, 50 + i * 20, line, 20, 255, 255, 255);
    }
    draw_line(30 + app_runner_x * 10, 30 + (app_runner_y + 1) * 20, 30 + (app_runner_x + 1) * 10, 30 + (app_runner_y + 1) * 20, 3, 200, 200, 200);
}

void app_runner_key(char key)
{
    app_runner_printc(key);
}

void app_runner_mouse(int x, int y)
{

}

void app_runner_init()
{
    app_runner_clear_video_memory();
    app_runner_printf(boot_messages);
}

unsigned char app_runner_icon_60[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
    0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3c, 0x7f, 0xff, 0xff,
    0xff, 0xff, 0xc0, 0x03, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3c,
    0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xe1, 0xff, 0xff, 0xff, 0xff,
    0xfc, 0x00, 0x3f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xf8, 0x7f,
    0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xc0,
    0x03, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xc3, 0xff, 0xff,
    0xff, 0xff, 0xc0, 0x03, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f,
    0x07, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xe0, 0xff, 0xff, 0xff, 0xff,
    0xfc, 0x00, 0x3c, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xc3, 0xff,
    0xff, 0xff, 0xff, 0xfc, 0x00, 0x3c, 0x7f, 0x80, 0x7f, 0xff, 0xff, 0xc0,
    0x03, 0xcf, 0xf8, 0x03, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0x80, 0x7f,
    0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
    0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#endif
