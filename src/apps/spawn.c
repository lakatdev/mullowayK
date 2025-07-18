#include <userlib.h>

void app_spawn_draw()
{
    draw_screen(THEME_BACKGROUND_COLOR);
    draw_text(10, 30, "this will manage interpreter io and have the 5 buttons and a stop start thing", 20, THEME_TEXT_COLOR);
}

void app_spawn_key(char key)
{

}

void app_spawn_mouse(int x, int y)
{

}

void app_spawn_init()
{

}
