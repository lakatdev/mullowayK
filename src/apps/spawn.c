#include <userlib.h>
#include <interpreter/interpreter.h>
#include <rtc.h>
#include <interface.h>

Interpreter_Instance app_spawn_main_instance;
int app_spawn_loaded = 0;
char app_spawn_load_year = 0;
char app_spawn_load_month = 0;
char app_spawn_load_day = 0;
char app_spawn_load_hour = 0;
char app_spawn_load_minute = 0;
char app_spawn_load_second = 0;

int app_spawn_error_code = 0;

void app_spawn_draw()
{
    draw_screen(THEME_BACKGROUND_COLOR);

    if (app_spawn_loaded) {
        char template[] = "Loaded at: XX-XX-XX XX:XX:XX";
        char hex[] = "0123456789ABCDEF";
        template[11] = hex[(app_spawn_load_year >> 4) & 0xF];
        template[12] = hex[app_spawn_load_year & 0xF];
        template[14] = hex[(app_spawn_load_month >> 4) & 0xF];
        template[15] = hex[app_spawn_load_month & 0xF];
        template[17] = hex[(app_spawn_load_day >> 4) & 0xF];
        template[18] = hex[app_spawn_load_day & 0xF];
        template[20] = hex[(app_spawn_load_hour >> 4) & 0xF];
        template[21] = hex[app_spawn_load_hour & 0xF];
        template[23] = hex[(app_spawn_load_minute >> 4) & 0xF];
        template[24] = hex[app_spawn_load_minute & 0xF];
        template[26] = hex[(app_spawn_load_second >> 4) & 0xF];
        template[27] = hex[app_spawn_load_second & 0xF];
        if (app_spawn_error_code == 0) {
            draw_text(10, 24, template, 20, THEME_TEXT_COLOR);
        }
        else {
            draw_text(10, 24, template, 20, 255, 0, 0);
        }
    }
    else {
        draw_text(10, 24, "No code loaded.", 20, THEME_TEXT_COLOR);
    }
}

void app_spawn_load_code(const char* code)
{
    app_spawn_loaded = 0;
    if (interpreter_load_code(&app_spawn_main_instance, code) != 0) {
        printf("Spawn: Failed to load code.\n");
        return;
    }
    if (interpreter_parse_functions(&app_spawn_main_instance) != 0) {
        printf("Spawn: Failed to parse functions.\n");
        return;
    }
    app_spawn_loaded = 1;
    app_spawn_load_year = get_year();
    app_spawn_load_month = get_month();
    app_spawn_load_day = get_day();
    app_spawn_load_hour = get_hour();
    app_spawn_load_minute = get_minute();
    app_spawn_load_second = get_second();
}

void app_spawn_execute()
{
    // clear buffer
    app_spawn_error_code = 0;
    

    if (!app_spawn_loaded) {
        printf("Spawn: No code loaded to execute.\n");
        // say this in the UI
        return;
    }
    if (interpreter_execute(&app_spawn_main_instance) != 0) {
        printf("Spawn: Execution failed.\n");
        app_spawn_error_code = 1;
        return;
    }
}

void app_spawn_clear_code()
{
    app_spawn_loaded = 0;
    app_spawn_main_instance.parsed_line_count = 0;
    app_spawn_main_instance.func_count = 0;
    app_spawn_main_instance.execution_position = 0;
    app_spawn_main_instance.stack_pointer = 0;
    app_spawn_error_code = 0;
}

void app_spawn_send_io()
{

}

void app_spawn_key(char key)
{

}

void app_spawn_mouse(int x, int y)
{

}

void app_spawn_init()
{
    add_app_menu_item((MenuItem) {
        .name = "Run/Stop",
        .action = app_spawn_execute
    });

    add_app_menu_item((MenuItem) {
        .name = "Clear",
        .action = app_spawn_clear_code
    });

    add_app_menu_item((MenuItem) {
        .name = "Send",
        .action = app_spawn_send_io
    });
}
