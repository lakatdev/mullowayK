#include <graphics.h>
#include <mouse.h>
#include <rtc.h>
#include <interrupts.h>
#include <memory.h>
#include <interface.h>
#include <userlib.h>
#include <storage.h>

unsigned char mlogo_26[] = {
    0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7f, 0xfc, 0x00, 0x00, 0x00, 0x04, 0x00, 0x07, 0xff, 0xf0, 0x00, 0x00,
    0x01, 0x80, 0x00, 0x3f, 0xff, 0xc0, 0x00, 0x00, 0x78, 0x00, 0x03, 0xff,
    0xff, 0x00, 0x00, 0x1f, 0x00, 0x0c, 0x3f, 0xff, 0xfc, 0x00, 0x07, 0xe0,
    0x03, 0xe7, 0xff, 0xff, 0xf0, 0x00, 0xfc, 0x00, 0x7f, 0xff, 0xff, 0xff,
    0xe0, 0x3f, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x8f, 0xf8, 0x03, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x8f,
    0xfc, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xe0, 0x7f, 0xff, 0xff,
    0xff, 0xfc, 0x00, 0xfe, 0x01, 0xdf, 0xbf, 0xdb, 0xff, 0x00, 0x03, 0xf0,
    0x04, 0xf3, 0xfd, 0xbf, 0xc0, 0x00, 0x0f, 0x80, 0x0f, 0x3f, 0xdb, 0xf0,
    0x00, 0x00, 0x1c, 0x01, 0xeb, 0x6d, 0xb5, 0x6c, 0x29, 0x20, 0x60, 0x0b,
    0xb6, 0xda, 0x12, 0x40, 0x92, 0x01, 0x00, 0x9b, 0x6d, 0xb1, 0x24, 0x08,
    0x80, 0x00, 0x04, 0x36, 0xd9, 0x13, 0x64, 0x8c, 0x00, 0x02, 0x45, 0x6c,
    0x49, 0x12, 0x48, 0xc0, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x30, 0x00
};

typedef struct {
    char name[32];
    MenuItem items[9];
    int item_count;
} Menu;

typedef struct {
    void (*mouse_click)(int x, int y);
    void (*key_press)(char key);
    void (*draw)();
    void (*init)();
    void (*on_close)();
    char name[32];
    int x;
    int y;
    int width;
    int height;
    char visible;
    Menu menu;
} Application;

unsigned char update_required = 1;
Application applications[9];
int application_count = 0;
int selected_application = -1;
Menu menus[3];
int menu_count = 0;
int selected_menu = -1;
int last_clicked_menu_item = 0;
unsigned char desktop_running = 1;
int moving_application = -1;
int move_offset_x = 0;
int move_offset_y = 0;
int resizing_application = -1;

void update_info_bar()
{
    char* template = "yy-mm-dd hh:mm:ss";
    char* hex = "0123456789ABCDEF";

    char seconds = get_second();
    char minutes = get_minute();
    char hours = get_hour();
    char year = get_year();
    char month = get_month();
    char day = get_day();

    template[0] = hex[(year >> 4) & 0xF];
    template[1] = hex[year & 0xF];
    template[3] = hex[(month >> 4) & 0xF];
    template[4] = hex[month & 0xF];
    template[6] = hex[(day >> 4) & 0xF];
    template[7] = hex[day & 0xF];
    template[9] = hex[(hours >> 4) & 0xF];
    template[10] = hex[hours & 0xF];
    template[12] = hex[(minutes >> 4) & 0xF];
    template[13] = hex[minutes & 0xF];
    template[15] = hex[(seconds >> 4) & 0xF];
    template[16] = hex[seconds & 0xF];

    system_draw_text(WIDTH - 220, 22, template, 24, THEME_TEXT_COLOR);
}

void invalidate()
{
    update_required = 1;
}

void key_press(char key)
{
    if (application_count > 0 && selected_application != -1) {
        applications[selected_application].key_press(key);
        invalidate();
    }
}

void add_application(Application app)
{
    if (application_count >= 9) {
        return;
    }

    applications[application_count] = app;
    applications[application_count].x = 100;
    applications[application_count].y = 100;
    applications[application_count].width = 400;
    applications[application_count].height = 300;
    applications[application_count].visible = 0;
    memcpy(applications[application_count].menu.name, applications[application_count].name, 32);
    applications[application_count].menu.item_count = 0;

    application_count++;
}

void add_menu(Menu menu)
{
    if (menu_count >= 3) {
        return;
    }
    menu.item_count = 0;
    menus[menu_count] = menu;
    menu_count++;
}

void add_menu_item(Menu* menu, MenuItem menu_item)
{
    if (menu->item_count >= 9) {
        return;
    }

    menu->items[menu->item_count] = menu_item;
    menu->item_count++;
}

void mouse_click(int x, int y)
{
    if (moving_application != -1) {
        moving_application = -1;
        return;
    }

    if (resizing_application != -1) {
        resizing_application = -1;
        return;
    }

    if (selected_menu != -1) {
        if (y >= 30 && y < 30 + menus[selected_menu].item_count * 30 &&
            x >= (selected_menu + 1) * 100 && x < (selected_menu + 1) * 100 + 200) {
            int item = (y - 30) / 30;
            if (menus[selected_menu].items[item].action != (void*)0) {
                last_clicked_menu_item = item;
                menus[selected_menu].items[item].action();
                invalidate();
                selected_menu = -1;
                return;
            }
        }
    }

    int previous_selected_menu = selected_menu;
    selected_menu = -1;
    if (y <= 30 && x >= 100 && x < 100 + menu_count * 100) {
        int menu = (x - 100) / 100;
        if (menu >= 0 && menu < menu_count) {
            if (menu != previous_selected_menu) {
                selected_menu = menu;
            }
            invalidate();
        }
    }
    else {
        if (application_count > 0) {
            if (x >= applications[selected_application].x && x < applications[selected_application].x + applications[selected_application].width &&
                y >= applications[selected_application].y && y < applications[selected_application].y + applications[selected_application].height) {
                system_set_clip_region(applications[selected_application].x, applications[selected_application].y, applications[selected_application].width, applications[selected_application].height);
                applications[selected_application].mouse_click(x - applications[selected_application].x, y - applications[selected_application].y);
                system_reset_clip_region();
                invalidate();
                return;
            }
            else if (x >= applications[selected_application].x && x < applications[selected_application].x + 30 &&
                y >= applications[selected_application].y - 30 && y < applications[selected_application].y) {
                applications[selected_application].on_close();
                applications[selected_application].visible = 0;
                selected_application = -1;
                memset(menus[2].name, 0, 32);
                memcpy(menus[2].name, "Desktop", 7);
                menus[2].item_count = 0;
                invalidate();
                return;
            }
            else if (x >= applications[selected_application].x + 30 && x < applications[selected_application].x + 60 &&
                y >= applications[selected_application].y - 30 && y < applications[selected_application].y) {
                moving_application = selected_application;
                move_offset_x = x - applications[selected_application].x;
                move_offset_y = y - applications[selected_application].y;
                return;
            }
            else if (x >= applications[selected_application].x + 60 && x < applications[selected_application].x + 90 &&
                y >= applications[selected_application].y - 30 && y < applications[selected_application].y) {
                resizing_application = selected_application;
                set_mouse_pos(applications[selected_application].x + applications[selected_application].width, applications[selected_application].y + applications[selected_application].height);
            }
        }
    }
}

void draw_panel()
{
    system_draw_rect(0, 0, WIDTH, 30, THEME_BACKGROUND_COLOR);
    system_draw_image(20, 2, 60, 26, mlogo_26, THEME_TEXT_COLOR);

    for (int i = 0; i < menu_count; i++) {
        if (i == selected_menu) {
            system_draw_rect((i + 1) * 100, 0, 100, 30, THEME_HIGHLIGHT_COLOR);
            system_draw_rect((i + 1) * 100 - 3, 27, 206, menus[i].item_count * 30 + 6, THEME_TEXT_COLOR);
            for (int j = 0; j < menus[i].item_count; j++) {
                system_draw_rect((i + 1) * 100, 30 + j * 30, 200, 30, THEME_BACKGROUND_COLOR);
                system_draw_text((i + 1) * 100 + 5, 30 + j * 30 + 22, menus[i].items[j].name, 24, THEME_TEXT_COLOR);
            }
        }
        system_draw_text((i + 1) * 100 + 5, 22, menus[i].name, 24, THEME_TEXT_COLOR);
    }
    update_info_bar();
}

void draw_desktop()
{
    system_draw_screen(THEME_ACCENT_COLOR);

    if (moving_application != -1) {
        applications[moving_application].x = get_mouse_x() - move_offset_x;
        applications[moving_application].y = get_mouse_y() - move_offset_y;

        if (applications[moving_application].x < 0) {
            applications[moving_application].x = 0;
        }
        if (applications[moving_application].y < 60) {
            applications[moving_application].y = 60;
        }
    }

    if (resizing_application != -1) {
        int new_width = get_mouse_x() - applications[resizing_application].x;
        int new_height = get_mouse_y() - applications[resizing_application].y;

        if (new_width > 200) {
            applications[resizing_application].width = new_width;
        }
        if (new_height > 100) {
            applications[resizing_application].height = new_height;
        }
    }

    for (int i = 0; i < application_count; i++) {
        if (applications[i].visible == 1 && i != selected_application) {
            system_draw_rect(applications[i].x - 3, applications[i].y - 33, applications[i].width + 6, applications[i].height + 36, THEME_BACKGROUND_COLOR);
            system_draw_text(applications[i].x + 5, applications[i].y - 10, applications[i].name, 24, THEME_TEXT_COLOR);

            system_set_clip_region(applications[i].x, applications[i].y, applications[i].width, applications[i].height);
            applications[i].draw();
            system_reset_clip_region();
        }
    }
    if (application_count > 0 && selected_application != -1) {
        if (applications[selected_application].visible == 1) {
            system_draw_rect(applications[selected_application].x - 3, applications[selected_application].y - 33, applications[selected_application].width + 6, applications[selected_application].height + 36, THEME_HIGHLIGHT_COLOR);
            system_draw_text(applications[selected_application].x + 95, applications[selected_application].y - 10, applications[selected_application].name, 24, THEME_TEXT_COLOR);

            system_draw_rect(applications[selected_application].x, applications[selected_application].y - 30, 30, 30, 255, 0, 0);
            system_draw_rect(applications[selected_application].x + 30, applications[selected_application].y - 30, 30, 30, 0, 255, 0);
            system_draw_rect(applications[selected_application].x + 60, applications[selected_application].y - 30, 30, 30, 0, 0, 255);

            system_set_clip_region(applications[selected_application].x, applications[selected_application].y, applications[selected_application].width, applications[selected_application].height);
            applications[selected_application].draw();
            system_reset_clip_region();
        }
    }

    draw_panel();
    draw_cursor();

    invalidate_buffer();
    sleep(15);
}

void add_app_menu_item(MenuItem menu_item)
{
    add_menu_item(&applications[selected_application].menu, menu_item);
    menus[2] = applications[selected_application].menu;
}

void select_application()
{
    selected_application = last_clicked_menu_item;
    menus[2] = applications[selected_application].menu;
    if (applications[selected_application].visible == 0) {
        applications[selected_application].menu.item_count = 0;
        applications[selected_application].init();
    }
    applications[selected_application].visible = 1;
    invalidate();
}

void terminate_desktop()
{
    desktop_running = 0;
}

void format_disk()
{
    if (!is_storage_initialized()) {
        if (write_magic_number(32768) != 0) {
            printf("System: Disk format failed!\n");
            return;
        }
        printf("System: Disk formatted successfully.\n");
        init_storage(32768);
        if (is_storage_initialized()) {
            printf("System: Storage reinitialized successfully!\n");
        }
        else {
            printf("System: Storage reinitialization failed!\n");
        }
    }
    else {
        printf("System: Storage already initialized\n");
    }
}

#include <apps/info.h>
#include <apps/files.h>
#include <apps/editor.h>
#include <apps/debug.h>
#include <apps/runtime.h>

void init_desktop()
{
    add_application((Application) {
        .init = app_info_init,
        .mouse_click = app_info_mouse,
        .key_press = app_info_key,
        .draw = app_info_draw,
        .on_close = app_info_on_close,
        .name = "Info"
    });

    add_application((Application) {
        .init = app_files_init,
        .mouse_click = app_files_mouse,
        .key_press = app_files_key,
        .draw = app_files_draw,
        .on_close = app_files_on_close,
        .name = "Files"
    });

    add_application((Application) {
        .init = app_editor_init,
        .mouse_click = app_editor_mouse,
        .key_press = app_editor_key,
        .draw = app_editor_draw,
        .on_close = app_editor_on_close,
        .name = "Editor"
    });

    add_application((Application) {
        .init = app_debug_init,
        .mouse_click = app_debug_mouse,
        .key_press = app_debug_key,
        .draw = app_debug_draw,
        .on_close = app_debug_on_close,
        .name = "Debug"
    });

    add_application((Application) {
        .init = app_runtime_init,
        .mouse_click = app_runtime_mouse,
        .key_press = app_runtime_key,
        .draw = app_runtime_draw,
        .on_close = app_runtime_on_close,
        .name = "Runtime"
    });

    add_menu((Menu) { .name = "System" });
    add_menu_item(&menus[0], (MenuItem) {
        .name = "Shutdown",
        .action = terminate_desktop
    });
    
    add_menu_item(&menus[0], (MenuItem) {
        .name = "Format disk",
        .action = format_disk
    });

    add_menu((Menu) { .name = "Tools" });
    for (int i = 0; i < application_count; i++) {
        add_menu_item(&menus[1], (MenuItem) {
            .name = "",
            .action = select_application
        });
        memcpy(menus[1].items[i].name, applications[i].name, 32);
    }

    add_menu((Menu) { .name = "Desktop" });

    while (desktop_running) {
        if (update_required) {
            draw_desktop();
            update_required = 0;
        }
    }
}
