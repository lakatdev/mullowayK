#include <userlib.h>
#include <interface.h>
#include <memory.h>

#define APP_DEBUG_WIDTH 80
#define APP_DEBUG_HEIGHT 25

int app_debug_x = 0;
int app_debug_y = 0;
char app_debug_video[APP_DEBUG_WIDTH * APP_DEBUG_HEIGHT] = {0};

void app_debug_scroll_video()
{
    for (int count = APP_DEBUG_WIDTH; count < APP_DEBUG_WIDTH * APP_DEBUG_HEIGHT; count++) {
        app_debug_video[count - APP_DEBUG_WIDTH] = app_debug_video[count];
        app_debug_video[count] = 0;
    }
}

void app_debug_clear_video_memory()
{
    for (int count = 0; count < APP_DEBUG_WIDTH * APP_DEBUG_HEIGHT; count++) {
        app_debug_video[count] = 0;
    }
}

void app_debug_printc(char c)
{
    switch (c) {
        case '\n': {
            app_debug_x = 0;
            app_debug_y++;
            break;
        }
        case 0x3: {
            app_debug_x = 0;
            app_debug_y = 0;
            app_debug_clear_video_memory();
            break;
        }   
        case '\b': {
            if (app_debug_x > 0) {
                app_debug_x--;
                app_debug_video[app_debug_y * APP_DEBUG_WIDTH + app_debug_x] = 0;
            }
            break;
        }
        default: {
            app_debug_video[app_debug_y * APP_DEBUG_WIDTH + app_debug_x] = c;
            app_debug_x++;
            break;
        }
    }

    if (app_debug_x >= 80) {
        app_debug_x = 0;
        app_debug_y++;
    }

    if (app_debug_y >= 25) {
        app_debug_scroll_video();
        app_debug_x = 0;
        app_debug_y = 24;
    }
}

void app_debug_printf(const char* str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        app_debug_printc(str[i]);
    }
}

void app_debug_draw()
{
    draw_rect(0, 0, 10 * APP_DEBUG_WIDTH, 20 * APP_DEBUG_HEIGHT, 0, 0, 0);
    for (int i = 0; i < APP_DEBUG_HEIGHT; i++) {
        char line[APP_DEBUG_WIDTH + 1] = {0};
        memcpy(line, &app_debug_video[i * APP_DEBUG_WIDTH], APP_DEBUG_WIDTH);
        draw_text(0, 15 + i * 20, line, 20, 255, 255, 255);
    }
    draw_line(app_debug_x * 10, (app_debug_y + 1) * 20 - 5, (app_debug_x + 1) * 10, (app_debug_y + 1) * 20 - 5, 3, 200, 200, 200);
}

void app_debug_key(char key)
{
    app_debug_printc(key);
}

void app_debug_mouse(int x, int y)
{

}

void app_debug_print_boot_messages()
{
    app_debug_clear_video_memory();
    app_debug_printf(boot_messages);
}

void app_debug_init()
{
    app_debug_print_boot_messages();
    add_app_menu_item((MenuItem){
        .name = "Boot Messages",
        .action = app_debug_print_boot_messages
    });
}
