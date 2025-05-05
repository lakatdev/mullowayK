#include <userlib.h>
#include <interface.h>
#include <memory.h>

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
    draw_rect(0, 0, 10 * APP_RUNNER_WIDTH, 20 * APP_RUNNER_HEIGHT, 0, 0, 0);
    for (int i = 0; i < APP_RUNNER_HEIGHT; i++) {
        char line[APP_RUNNER_WIDTH + 1] = {0};
        memcpy(line, &app_runner_video[i * APP_RUNNER_WIDTH], APP_RUNNER_WIDTH);
        draw_text(0, 15 + i * 20, line, 20, 255, 255, 255);
    }
    draw_line(app_runner_x * 10, (app_runner_y + 1) * 20 - 5, (app_runner_x + 1) * 10, (app_runner_y + 1) * 20 - 5, 3, 200, 200, 200);
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
