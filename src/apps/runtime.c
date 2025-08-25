#include <userlib.h>
#include <interpreter/interpreter.h>
#include <rtc.h>
#include <storage.h>
#include <interface.h>
#include <desktop.h>
#include <memory.h>

Interpreter_Instance app_runtime_instances[INTERPRETER_MAX_INSTANCES];
int app_runtime_instance_stack_top = -1;

int app_runtime_loaded = 0;
char app_runtime_load_year = 0;
char app_runtime_load_month = 0;
char app_runtime_load_day = 0;
char app_runtime_load_hour = 0;
char app_runtime_load_minute = 0;
char app_runtime_load_second = 0;

int app_runtime_error_code = 0;
int app_runtime_execution_requested = 0;
int app_runtime_executing = 0;
int app_runtime_input_requested = 0;
char app_runtime_input_buffer[INTERPRETER_MAX_INPUT_LENGTH] = {0};
int app_runtime_input_buffer_length = 0;
#define APP_RUNTIME_WIDTH 80
#define APP_RUNTIME_HEIGHT 25

char app_runtime_code[INTERPRETER_MAX_CODE] = {0};
char app_runtime_video[APP_RUNTIME_WIDTH * APP_RUNTIME_HEIGHT] = {0};
int app_runtime_cursor_x = 0;

Interpreter_Instance* app_runtime_get_current_instance()
{
    if (app_runtime_instance_stack_top >= 0) {
        return &app_runtime_instances[app_runtime_instance_stack_top];
    }
    return (void*)0;
}

void app_runtime_request_input()
{
    app_runtime_input_requested = 1;
    app_runtime_input_buffer[0] = '\0';
    app_runtime_input_buffer_length = 0;
}

void app_runtime_scroll_video()
{
    for (int i = 0; i < (APP_RUNTIME_HEIGHT - 1) * APP_RUNTIME_WIDTH; i++) {
        app_runtime_video[i] = app_runtime_video[i + APP_RUNTIME_WIDTH];
    }
    for (int i = (APP_RUNTIME_HEIGHT - 1) * APP_RUNTIME_WIDTH; i < APP_RUNTIME_HEIGHT * APP_RUNTIME_WIDTH; i++) {
        app_runtime_video[i] = 0;
    }
}

void app_runtime_print_char(char c)
{
    switch (c) {
        case '\n': {
            app_runtime_cursor_x = 0;
            app_runtime_scroll_video();
            break;
        }
        case '\t': {
            app_runtime_cursor_x = (app_runtime_cursor_x + 4) & ~3; 
            break;
        }
        default: {
            if (c >= 32 && c <= 126) { 
                if (app_runtime_cursor_x < APP_RUNTIME_WIDTH) {
                    app_runtime_video[(APP_RUNTIME_HEIGHT - 1) * APP_RUNTIME_WIDTH + app_runtime_cursor_x] = c;
                    app_runtime_cursor_x++;
                }
                else {
                    app_runtime_cursor_x = 0;
                    app_runtime_scroll_video();
                    app_runtime_video[(APP_RUNTIME_HEIGHT - 1) * APP_RUNTIME_WIDTH + app_runtime_cursor_x] = c;
                }
            }
            break;
        }
    }
    invalidate();
}

void app_runtime_print(const char* str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        app_runtime_print_char(str[i]);
    }
    invalidate();
}

void app_runtime_print_int(int value)
{
    char buffer[16]; 
    int i = 0;
    int is_negative = 0;
    
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }
    
    if (value == 0) {
        app_runtime_print_char('0');
        return;
    }
    
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    if (is_negative) {
        app_runtime_print_char('-');
    }
    
    for (int j = i - 1; j >= 0; j--) {
        app_runtime_print_char(buffer[j]);
    }
    invalidate();
}

void app_runtime_print_float(float value)
{
    if (value != value) {
        app_runtime_print_char('N');
        app_runtime_print_char('a');
        app_runtime_print_char('N');
        return;
    }
    
    if (value < 0) {
        app_runtime_print_char('-');
        value = -value;
    }
    
    if (value > 3.4028235e+38f) {
        app_runtime_print_char('I');
        app_runtime_print_char('n');
        app_runtime_print_char('f');
        return;
    }
    
    int int_part = (int)value;
    float frac_part = value - int_part;
    app_runtime_print_int(int_part);
    app_runtime_print_char('.');
    
    for (int i = 0; i < 6; i++) {
        frac_part *= 10;
        int digit = (int)frac_part;
        app_runtime_print_char('0' + digit);
        frac_part -= digit;
        
        if (frac_part < 0.0000001f) {
            break;
        }
    }
    invalidate();
}

void app_runtime_draw()
{
    draw_screen(THEME_BACKGROUND_COLOR);

    if (app_runtime_input_requested) {
        draw_rect(0, 6, get_window_width(), 22, THEME_HIGHLIGHT_COLOR);
        draw_text(10, 22, "Input: ", 20, THEME_TEXT_COLOR);
        draw_text(70, 22, app_runtime_input_buffer, 20, THEME_TEXT_COLOR);
    }
    else if (app_runtime_loaded) {
        char template[] = "Loaded at: XX-XX-XX XX:XX:XX";
        char hex[] = "0123456789ABCDEF";
        template[11] = hex[(app_runtime_load_year >> 4) & 0xF];
        template[12] = hex[app_runtime_load_year & 0xF];
        template[14] = hex[(app_runtime_load_month >> 4) & 0xF];
        template[15] = hex[app_runtime_load_month & 0xF];
        template[17] = hex[(app_runtime_load_day >> 4) & 0xF];
        template[18] = hex[app_runtime_load_day & 0xF];
        template[20] = hex[(app_runtime_load_hour >> 4) & 0xF];
        template[21] = hex[app_runtime_load_hour & 0xF];
        template[23] = hex[(app_runtime_load_minute >> 4) & 0xF];
        template[24] = hex[app_runtime_load_minute & 0xF];
        template[26] = hex[(app_runtime_load_second >> 4) & 0xF];
        template[27] = hex[app_runtime_load_second & 0xF];

        if (app_runtime_error_code == 0) {
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

    for (int y = 0; y < visible_height && y < APP_RUNTIME_HEIGHT; y++) {
        int buffer_row = APP_RUNTIME_HEIGHT - visible_height + y;
        if (buffer_row >= 0) {
            for (int x = 0; x < visible_width && x < APP_RUNTIME_WIDTH; x++) {
                char c[2] = {0};
                c[0] = app_runtime_video[buffer_row * APP_RUNTIME_WIDTH + x];
                if (c[0] != 0) {
                    draw_text(10 + x * 10, 48 + y * 24, c, 20, THEME_TEXT_COLOR);
                }
            }
        }
    }
}

void app_runtime_clear_buffer()
{
    memset(app_runtime_video, 0, sizeof(app_runtime_video));
    app_runtime_cursor_x = 0;
}

void app_runtime_load_code(const char* code)
{
    app_runtime_loaded = 0;
    strcpy(app_runtime_code, code);
    app_runtime_loaded = 1;
    app_runtime_load_year = get_year();
    app_runtime_load_month = get_month();
    app_runtime_load_day = get_day();
    app_runtime_load_hour = get_hour();
    app_runtime_load_minute = get_minute();
    app_runtime_load_second = get_second();
}

void app_runtime_clear_code()
{
    app_runtime_loaded = 0;
    app_runtime_error_code = 0;
    app_runtime_execution_requested = 0;
    app_runtime_executing = 0;
    memset(app_runtime_video, 0, sizeof(app_runtime_video));
    app_runtime_cursor_x = 0;
}

void app_runtime_execute()
{
    app_runtime_error_code = 0;
    memset(app_runtime_video, 0, sizeof(app_runtime_video));
    app_runtime_cursor_x = 0;

    if (!app_runtime_loaded) {
        printf("Runtime: No code loaded to execute.\n");
        return;
    }
    
    app_runtime_instance_stack_top = 0;
    interpreter_instance_init(&app_runtime_instances[0]);
    
    if (interpreter_load_code(&app_runtime_instances[0], app_runtime_code) != 0) {
        printf("Runtime: Failed to load code.\n");
        app_runtime_error_code = 1;
        app_runtime_executing = 0;
        app_runtime_instance_stack_top = -1;
        return;
    }
    if (interpreter_parse_functions(&app_runtime_instances[0]) != 0) {
        printf("Runtime: Failed to parse functions.\n");
        app_runtime_error_code = 1;
        app_runtime_executing = 0;
        app_runtime_instance_stack_top = -1;
        return;
    }
    
    int result = interpreter_execute(&app_runtime_instances[0]);
    if (result == 0) {
        app_runtime_executing = 0;
        app_runtime_instance_stack_top = -1;
    }
    else if (result < 0) {
        printf("Runtime: Execution failed.\n");
        app_runtime_error_code = 1;
        app_runtime_executing = 0;
        app_runtime_instance_stack_top = -1;
    }
}

void app_runtime_continue_execution()
{
    Interpreter_Instance* current = app_runtime_get_current_instance();
    if (app_runtime_executing && current && current->is_running) {
        int result = interpreter_execute_chunk(current, 32);
        if (result == 0) {
            app_runtime_instance_stack_top--;
            if (app_runtime_instance_stack_top < 0) {
                app_runtime_executing = 0;
            }
        }
        else if (result < 0) {
            printf("Runtime: Execution failed.\n");
            app_runtime_error_code = 1;
            app_runtime_executing = 0;
            app_runtime_instance_stack_top = -1;
        }
    }
}

void app_runtime_stop_execute()
{
    app_runtime_execution_requested = 0;
    Interpreter_Instance* current = app_runtime_get_current_instance();
    if (app_runtime_executing && current && current->is_running) {
        interpreter_stop(current);
    }
    app_runtime_executing = 0;
    app_runtime_instance_stack_top = -1;
    app_runtime_input_requested = 0;
    app_runtime_input_buffer[0] = '\0';
    app_runtime_input_buffer_length = 0;
}

void app_runtime_request_execute()
{
    if (!app_runtime_executing) {
        app_runtime_execution_requested = 1;
    }
}

void app_runtime_process_deferred()
{
    if (app_runtime_execution_requested && !app_runtime_executing) {
        app_runtime_execution_requested = 0;
        app_runtime_executing = 1;
        app_runtime_execute();
    }
    else if (app_runtime_executing) {
        app_runtime_continue_execution();
    }
}

int app_runtime_push_instance_from_file(const char* filename)
{
    if (app_runtime_instance_stack_top >= INTERPRETER_MAX_INSTANCES - 1) {
        printf("Error: EXEC: Maximum instance depth reached.\n");
        return -1;
    }
    
    if (!files_exists(filename)) {
        printf("Error: EXEC: File not found.\n");
        return -1;
    }
    
    static char file_content[INTERPRETER_MAX_CODE];
    unsigned int file_size = 0;
    
    read_from_storage(filename, file_content, &file_size);
    
    if (file_size == 0) {
        printf("Error: EXEC: File is empty.\n");
        return -1;
    }
    
    file_content[file_size] = '\0';
    
    app_runtime_instance_stack_top++;
    Interpreter_Instance* new_instance = &app_runtime_instances[app_runtime_instance_stack_top];
    
    interpreter_instance_init(new_instance);
    
    if (interpreter_load_code(new_instance, file_content) != 0) {
        printf("Error: EXEC: Failed to load code.\n");
        app_runtime_instance_stack_top--;
        return -1;
    }
    
    if (interpreter_parse_functions(new_instance) != 0) {
        printf("Error: EXEC: Failed to parse functions.\n");
        app_runtime_instance_stack_top--;
        return -1;
    }
    
    int result = interpreter_execute(new_instance);
    if (result < 0) {
        printf("Error: EXEC: Execution failed.\n");
        app_runtime_instance_stack_top--;
        return -1;
    }
    
    if (result == 0) {
        app_runtime_instance_stack_top--;
    }
    
    return 0;
}

void app_runtime_send_io()
{
    if (app_runtime_input_requested && app_runtime_instance_stack_top >= 0) {
        Interpreter_Instance* current = app_runtime_get_current_instance();
        if (current && current->waiting_for_input) {
            strncpy(current->input_buffer, app_runtime_input_buffer, sizeof(current->input_buffer) - 1);
            current->input_buffer[sizeof(current->input_buffer) - 1] = '\0';
            current->input_ready = 1;
            
            app_runtime_input_requested = 0;
            app_runtime_input_buffer[0] = '\0';
            app_runtime_input_buffer_length = 0;
        }
    }
}

void app_runtime_key(char key)
{
    if (app_runtime_input_requested) {
        switch (key) {
            case '\b': {
                if (app_runtime_input_buffer_length > 0) {
                    app_runtime_input_buffer_length--;
                    app_runtime_input_buffer[app_runtime_input_buffer_length] = '\0';
                }
                break;
            }
            case '\n': {
                app_runtime_send_io();
                break;
            }
            default: {
                if (key >= 32 && key <= 126) {
                    if (app_runtime_input_buffer_length < sizeof(app_runtime_input_buffer) - 1) {
                        app_runtime_input_buffer[app_runtime_input_buffer_length] = key;
                        app_runtime_input_buffer_length++;
                        app_runtime_input_buffer[app_runtime_input_buffer_length] = '\0';
                    }
                }
                break;
            }
        }
        invalidate();
    }
}

void app_runtime_mouse(int x, int y)
{

}

void app_runtime_on_close()
{
    app_runtime_stop_execute();
}

void app_runtime_init()
{
    add_app_menu_item((MenuItem) {
        .name = "Run",
        .action = app_runtime_request_execute
    });

    add_app_menu_item((MenuItem) {
        .name = "Clear",
        .action = app_runtime_clear_code
    });

    add_app_menu_item((MenuItem) {
        .name = "Send",
        .action = app_runtime_send_io
    });

    add_app_menu_item((MenuItem) {
        .name = "Stop",
        .action = app_runtime_stop_execute
    });

    app_runtime_stop_execute();
    memset(app_runtime_video, 0, sizeof(app_runtime_video));
    app_runtime_cursor_x = 0;
}
