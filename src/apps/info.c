#include <userlib.h>

void app_info_draw()
{
    draw_screen(THEME_BACKGROUND_COLOR);
    draw_text(10, 30, "MullowayK 1.0.2 build 2025-09-02", 24, THEME_TEXT_COLOR);
    draw_text(10, 54, "Using Keszeg 4 interpreter.", 24, THEME_TEXT_COLOR);
}

void app_info_key(char key)
{

}

void app_info_on_close()
{

}

void app_info_mouse(int x, int y)
{

}

void app_info_init()
{

}
