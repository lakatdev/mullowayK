#include <graphics.h>
#include <mouse.h>
#include <rtc.h>
#include <interrupts.h>
#include <memory.h>
#include <interface.h>
#include <userlib.h>

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
    MenuItem* items;
    int item_count;
} Menu;

typedef struct {
    void (*mouse_click)(int x, int y);
    void (*key_press)(char key);
    void (*draw)();
    void (*init)();
    unsigned char* icon;
    char name[32];
    Menu menu;
} Application;

unsigned char update_required = 1;
Application* applications = (void*)0;
int application_count = 0;
int selected_application = 0;
Menu* menus = (void*)0;
int menu_count = 0;
int selected_menu = -1;
int last_clicked_menu_item = 0;
int initialized_app = 0;
unsigned char desktop_running = 1;

void update_info_bar()
{
    char* template = "MEM ~000.00% yy-mm-dd hh:mm:ss";
    char* hex = "0123456789ABCDEF";

    char seconds = get_second();
    char minutes = get_minute();
    char hours = get_hour();
    char year = get_year();
    char month = get_month();
    char day = get_day();
    unsigned int memory = get_memory_usage();

    template[5] = '0' + (memory / 10000);
    memory %= 10000;
    template[6] = '0' + (memory / 1000);
    memory %= 1000;
    template[7] = '0' + (memory / 100);
    memory %= 100;
    template[9] = '0' + (memory / 10);
    memory %= 10;
    template[10] = '0' + memory;

    template[13] = hex[(year >> 4) & 0xF];
    template[14] = hex[year & 0xF];
    template[16] = hex[(month >> 4) & 0xF];
    template[17] = hex[month & 0xF];
    template[19] = hex[(day >> 4) & 0xF];
    template[20] = hex[day & 0xF];
    template[22] = hex[(hours >> 4) & 0xF];
    template[23] = hex[hours & 0xF];
    template[25] = hex[(minutes >> 4) & 0xF];
    template[26] = hex[minutes & 0xF];
    template[28] = hex[(seconds >> 4) & 0xF];
    template[29] = hex[seconds & 0xF];

    system_draw_text(WIDTH - 370, 22, template, 24, THEME_TEXT_COLOR);
}

void invalidate()
{
    update_required = 1;
}

void key_press(char key)
{
    if (application_count > 0) {
        applications[selected_application].key_press(key);
        invalidate();
    }
}

void add_application(Application app)
{
    memcpy(app.menu.name, app.name, 32);
    app.menu.item_count = 0;

    Application* new_apps = (Application*)malloc((application_count + 1) * sizeof(Application));
    memcpy(new_apps, applications, application_count * sizeof(Application));
    new_apps[application_count] = app;
    if (application_count > 0) {
        free(applications);
    }
    applications = new_apps;
    application_count++;

    initialized_app = application_count - 1;
    app.init();
}

void add_menu(Menu menu)
{
    Menu* new_menus = (Menu*)malloc((menu_count + 1) * sizeof(Menu));
    memcpy(new_menus, menus, menu_count * sizeof(Menu));
    new_menus[menu_count] = menu;
    if (menu_count > 0) {
        free(menus);
    }
    menus = new_menus;
    menu_count++;
}

void add_menu_item(Menu* menu, MenuItem menu_item)
{
    MenuItem* new_items = (MenuItem*)malloc((menu->item_count + 1) * sizeof(MenuItem));
    memcpy(new_items, menu->items, menu->item_count * sizeof(MenuItem));
    new_items[menu->item_count] = menu_item;
    if (menu->item_count > 0) {
        free(menu->items);
    }
    menu->items = new_items;
    menu->item_count++;
}

void add_app_menu_item(MenuItem menu_item)
{
    add_menu_item(&applications[initialized_app].menu, menu_item);
}

void mouse_click(int x, int y)
{
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
    else if (y >= USER_WINDOW_Y && y < USER_WINDOW_Y + USER_WINDOW_HEIGHT &&
        x >= USER_WINDOW_X && x < USER_WINDOW_X + USER_WINDOW_WIDTH) {
        if (application_count > 0) {
            applications[selected_application].mouse_click(x - USER_WINDOW_X, y - USER_WINDOW_Y);
            invalidate();
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
    if (application_count > 0) {
        applications[selected_application].draw();
    }
    draw_panel();
    draw_cursor();

    invalidate_buffer();
    sleep(10);
}

void select_application()
{
    selected_application = last_clicked_menu_item;
    memcpy(&menus[2], &applications[selected_application].menu, sizeof(Menu));
    invalidate();
}

void terminate_desktop()
{
    desktop_running = 0;
}

#include <apps/desktop.h>
#include <apps/info.h>
#include <apps/files.h>
#include <apps/editor.h>
#include <apps/runner.h>

void init_desktop()
{
    add_application((Application) {
        .init = app_desktop_init,
        .mouse_click = app_desktop_mouse,
        .key_press = app_desktop_key,
        .draw = app_desktop_draw,
        .icon = app_desktop_icon_60,
        .name = "Desktop"
    });

    add_application((Application) {
        .init = app_info_init,
        .mouse_click = app_info_mouse,
        .key_press = app_info_key,
        .draw = app_info_draw,
        .icon = app_info_icon_60,
        .name = "Info"
    });

    add_application((Application) {
        .init = app_files_init,
        .mouse_click = app_files_mouse,
        .key_press = app_files_key,
        .draw = app_files_draw,
        .icon = app_files_icon_60,
        .name = "Files"
    });

    add_application((Application) {
        .init = app_editor_init,
        .mouse_click = app_editor_mouse,
        .key_press = app_editor_key,
        .draw = app_editor_draw,
        .icon = app_editor_icon_60,
        .name = "Editor"
    });

    add_application((Application) {
        .init = app_runner_init,
        .mouse_click = app_runner_mouse,
        .key_press = app_runner_key,
        .draw = app_runner_draw,
        .icon = app_runner_icon_60,
        .name = "Runner"
    });

    add_menu((Menu) { .name = "System" });
    add_menu_item(&menus[0], (MenuItem) {
        .name = "Shutdown",
        .action = terminate_desktop
    });

    add_menu((Menu) { .name = "Apps" });
    for (int i = 0; i < application_count; i++) {
        add_menu_item(&menus[1], (MenuItem) {
            .name = "",
            .action = select_application
        });
        memcpy(menus[1].items[i].name, applications[i].name, 32);
    }

    add_menu((Menu) { .name = "---" });
    memcpy(&menus[2], &applications[selected_application].menu, sizeof(Menu));

    while (desktop_running) {
        if (update_required) {
            draw_desktop();
            update_required = 0;
        }
    }
}

