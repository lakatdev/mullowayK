#include <userlib.h>
#include <interpreter/interpreter.h>
#include <rtc.h>
#include <interface.h>
#include <memory.h>

Interpreter_Instance app_spawn_main_instance;
int app_spawn_loaded = 0;
char app_spawn_load_year = 0;
char app_spawn_load_month = 0;
char app_spawn_load_day = 0;
char app_spawn_load_hour = 0;
char app_spawn_load_minute = 0;
char app_spawn_load_second = 0;

int app_spawn_error_code = 0;
#define APP_SPAWN_WIDTH 80
#define APP_SPAWN_HEIGHT 25

char app_spawn_video[APP_SPAWN_WIDTH * APP_SPAWN_HEIGHT] = {0};
int app_spawn_cursor_x = 0;

void app_spawn_scroll_video()
{
    for (int i = 0; i < (APP_SPAWN_HEIGHT - 1) * APP_SPAWN_WIDTH; i++) {
        app_spawn_video[i] = app_spawn_video[i + APP_SPAWN_WIDTH];
    }
    for (int i = (APP_SPAWN_HEIGHT - 1) * APP_SPAWN_WIDTH; i < APP_SPAWN_HEIGHT * APP_SPAWN_WIDTH; i++) {
        app_spawn_video[i] = 0;
    }
}

void app_spawn_print_char(char c)
{
    switch (c) {
        case '\n': {
            app_spawn_cursor_x = 0;
            app_spawn_scroll_video();
            break;
        }
        case '\t': {
            app_spawn_cursor_x = (app_spawn_cursor_x + 4) & ~3; 
            break;
        }
        default: {
            if (c >= 32 && c <= 126) { 
                if (app_spawn_cursor_x < APP_SPAWN_WIDTH) {
                    app_spawn_video[(APP_SPAWN_HEIGHT - 1) * APP_SPAWN_WIDTH + app_spawn_cursor_x] = c;
                    app_spawn_cursor_x++;
                }
                else {
                    app_spawn_cursor_x = 0;
                    app_spawn_scroll_video();
                    app_spawn_video[(APP_SPAWN_HEIGHT - 1) * APP_SPAWN_WIDTH + app_spawn_cursor_x] = c;
                }
            }
            break;
        }
    }
}

void app_spawn_print(const char* str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        app_spawn_print_char(str[i]);
    }
}

void app_spawn_print_int(int value)
{
    char buffer[16]; 
    int i = 0;
    int is_negative = 0;
    
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }
    
    if (value == 0) {
        app_spawn_print_char('0');
        return;
    }
    
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    if (is_negative) {
        app_spawn_print_char('-');
    }
    
    for (int j = i - 1; j >= 0; j--) {
        app_spawn_print_char(buffer[j]);
    }
}

void app_spawn_print_float(float value)
{
    if (value != value) {
        app_spawn_print_char('N');
        app_spawn_print_char('a');
        app_spawn_print_char('N');
        return;
    }
    
    if (value < 0) {
        app_spawn_print_char('-');
        value = -value;
    }
    
    if (value > 3.4028235e+38f) {
        app_spawn_print_char('I');
        app_spawn_print_char('n');
        app_spawn_print_char('f');
        return;
    }
    
    int int_part = (int)value;
    float frac_part = value - int_part;
    app_spawn_print_int(int_part);
    app_spawn_print_char('.');
    
    for (int i = 0; i < 6; i++) {
        frac_part *= 10;
        int digit = (int)frac_part;
        app_spawn_print_char('0' + digit);
        frac_part -= digit;
        
        if (frac_part < 0.0000001f) {
            break;
        }
    }
}

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
            draw_text(10, 22, template, 20, THEME_TEXT_COLOR);
        }
        else {
            draw_text(10, 22, template, 20, 255, 0, 0);
        }
    }
    else {
        draw_text(10, 22, "No code loaded.", 20, THEME_TEXT_COLOR);
    }

    draw_line(0, 32, get_window_width(), 32, 3, THEME_TEXT_COLOR);

    int visible_width = (get_window_width() - 20) / 10;
    int visible_height = (get_window_height() - 48) / 24 + 1;

    for (int y = 0; y < visible_height && y < APP_SPAWN_HEIGHT; y++) {
        int buffer_row = APP_SPAWN_HEIGHT - visible_height + y;
        if (buffer_row >= 0) {
            for (int x = 0; x < visible_width && x < APP_SPAWN_WIDTH; x++) {
                char c[2] = {0};
                c[0] = app_spawn_video[buffer_row * APP_SPAWN_WIDTH + x];
                if (c[0] != 0) {
                    draw_text(10 + x * 10, 48 + y * 24, c, 20, THEME_TEXT_COLOR);
                }
            }
        }
    }
}

void app_spawn_clear_buffer()
{
    memset(app_spawn_video, 0, sizeof(app_spawn_video));
    app_spawn_cursor_x = 0;
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
    app_spawn_error_code = 0;
    memset(app_spawn_video, 0, sizeof(app_spawn_video));
    app_spawn_cursor_x = 0;
    
    if (!app_spawn_loaded) {
        printf("Spawn: No code loaded to execute.\n");
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
    memset(app_spawn_video, 0, sizeof(app_spawn_video));
    app_spawn_cursor_x = 0;
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

    memset(app_spawn_video, 0, sizeof(app_spawn_video));
    app_spawn_cursor_x = 0;
}
