#include <graphics.h>
#include <mouse.h>
#include <rtc.h>
#include <interrupts.h>
#include <memory.h>
#include <interface.h>

typedef struct  {
    void (*mouse_click)(int x, int y);
    void (*key_press)(char key);
    void (*draw)();
    void (*init)();
    unsigned char* icon;
    char name[32];
} Application_t;

unsigned char update_required = 1;
Application_t* applications = (void*)0;
int application_count = 0;
int selected_application = 0;

void update_info_bar()
{
    char* template = "MEM ~000.00%\nyy-mm-dd hh:mm:ss";
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

    draw_text(WIDTH - 220, HEIGHT - 68, template, 24, 0, 0, 0);
}

void invalidate()
{
    update_required = 1;
}

void key_press(char key)
{
    if (application_count > 0) {
        applications[selected_application].key_press(key);
    }
}

void add_application(Application_t app)
{
    Application_t* new_apps = (Application_t*)malloc((application_count + 1) * sizeof(Application_t));
    memcpy(new_apps, applications, application_count * sizeof(Application_t));
    new_apps[application_count] = app;
    free(applications);
    applications = new_apps;
    application_count++;
    app.init();
}

void mouse_click(int x, int y)
{
    if (y > HEIGHT - 90 && y < HEIGHT - 30) {
        int app = (x - 250) / 70;
        if (app >= 0 && app < application_count) {
            selected_application = app;
            invalidate();
        }
    }
    else {
        if (application_count > 0) {
            applications[selected_application].mouse_click(x, y);
        }
    }
}

void draw_panel()
{
    draw_rect(0, HEIGHT - 95, WIDTH, 70, 249, 249, 224);
    draw_rect(240, HEIGHT - 90, WIDTH - 480, 60, 255, 208, 208);
    draw_image(45, HEIGHT - 90, 150, 60, mlogo_60, 0, 0, 0);

    for (int i = 0; i < application_count; i++) {
        if (i == selected_application) {
            draw_rect(250 + i * 70, HEIGHT - 90, 60, 60, 249, 249, 224);
        }
        draw_image(250 + i * 70, HEIGHT - 90, 60, 60, applications[i].icon, 28, 136, 155);
    }

    update_info_bar();
}

void draw_desktop()
{
    draw_screen(58, 166, 185);
    draw_panel();
    if (application_count > 0) {
        applications[selected_application].draw();
    }
    draw_cursor();
    invalidate_buffer();
    sleep(10);
}

#include <apps/desktop.h>
#include <apps/info.h>
#include <apps/files.h>
#include <apps/editor.h>
#include <apps/runner.h>

void init_desktop()
{
    add_application((Application_t) {
        .init = app_desktop_init,
        .mouse_click = app_desktop_mouse,
        .key_press = app_desktop_key,
        .draw = app_desktop_draw,
        .icon = app_desktop_icon_60,
        .name = "Desktop"
    });

    add_application((Application_t) {
        .init = app_info_init,
        .mouse_click = app_info_mouse,
        .key_press = app_info_key,
        .draw = app_info_draw,
        .icon = app_info_icon_60,
        .name = "Info"
    });

    add_application((Application_t) {
        .init = app_files_init,
        .mouse_click = app_files_mouse,
        .key_press = app_files_key,
        .draw = app_files_draw,
        .icon = app_files_icon_60,
        .name = "Files"
    });

    add_application((Application_t) {
        .init = app_editor_init,
        .mouse_click = app_editor_mouse,
        .key_press = app_editor_key,
        .draw = app_editor_draw,
        .icon = app_editor_icon_60,
        .name = "Editor"
    });

    add_application((Application_t) {
        .init = app_runner_init,
        .mouse_click = app_runner_mouse,
        .key_press = app_runner_key,
        .draw = app_runner_draw,
        .icon = app_runner_icon_60,
        .name = "Runner"
    });

    while (1) {
        if (update_required) {
            draw_desktop();
            update_required = 0;
        }
    }
}

