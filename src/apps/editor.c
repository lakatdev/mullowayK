#include <userlib.h>

void app_editor_draw()
{
    draw_screen(0, 0, 0);
}

void app_editor_key(char key)
{
    
}

void app_editor_mouse(int x, int y)
{

}

void app_editor_init()
{
    add_app_menu_item((MenuItem) {
        .name = "Run",
        .action = (void*)0
    });
}
