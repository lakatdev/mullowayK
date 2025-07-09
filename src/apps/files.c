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
    add_app_menu_item((MenuItem) {
        .name = "Refresh",
        .action = (void*)0
    });

    add_app_menu_item((MenuItem) {
        .name = "Edit",
        .action = (void*)0
    });

    add_app_menu_item((MenuItem) {
        .name = "Run",
        .action = (void*)0
    });

    add_app_menu_item((MenuItem) {
        .name = "Delete",
        .action = (void*)0
    });
}
