#include <tools/userlib.h>
#include <interpreter/interpreter.h>
#include <system/rtc.h>
#include <system/storage.h>
#include <system/interface.h>
#include <system/desktop.h>
#include <system/memory.h>
#include <apps/runtime_session.h>

static int app_runtime_current_window_id = -1;

RuntimeSession* app_runtime_get_current_session()
{
    if (app_runtime_current_window_id < 0) {
        return (void*)0;
    }
    return runtime_session_get_by_window(app_runtime_current_window_id);
}

void app_runtime_set_window_id(int window_id)
{
    app_runtime_current_window_id = window_id;
}

void app_runtime_request_input()
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_request_input(session);
    }
}

void app_runtime_print_char(char c)
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_print_char(session, c);
    }
}

void app_runtime_print(const char* str)
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_print(session, str);
    }
}

void app_runtime_print_int(int value)
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_print_int(session, value);
    }
}

void app_runtime_print_float(float value)
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_print_float(session, value);
    }
}

void app_runtime_draw()
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (!session) {
        draw_screen(THEME_BACKGROUND_COLOR);
        draw_text(10, 22, "No session initialized.", 20, THEME_TEXT_COLOR);
        return;
    }
    
    draw_screen(THEME_BACKGROUND_COLOR);

    if (session->input_requested) {
        draw_rect(0, 6, get_window_width(), 22, THEME_HIGHLIGHT_COLOR);
        draw_text(10, 22, "Input: ", 20, THEME_TEXT_COLOR);
        draw_text(70, 22, session->input_buffer, 20, THEME_TEXT_COLOR);
    }
    else if (session->loaded) {
        char template[] = "Loaded at: XX-XX-XX XX:XX:XX";
        char hex[] = "0123456789ABCDEF";
        template[11] = hex[(session->load_year >> 4) & 0xF];
        template[12] = hex[session->load_year & 0xF];
        template[14] = hex[(session->load_month >> 4) & 0xF];
        template[15] = hex[session->load_month & 0xF];
        template[17] = hex[(session->load_day >> 4) & 0xF];
        template[18] = hex[session->load_day & 0xF];
        template[20] = hex[(session->load_hour >> 4) & 0xF];
        template[21] = hex[session->load_hour & 0xF];
        template[23] = hex[(session->load_minute >> 4) & 0xF];
        template[24] = hex[session->load_minute & 0xF];
        template[26] = hex[(session->load_second >> 4) & 0xF];
        template[27] = hex[session->load_second & 0xF];

        if (session->error_code == 0) {
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
                c[0] = session->video[buffer_row * APP_RUNTIME_WIDTH + x];
                if (c[0] != 0) {
                    draw_text(10 + x * 10, 48 + y * 24, c, 20, THEME_TEXT_COLOR);
                }
            }
        }
    }
}

void app_runtime_clear_buffer()
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_clear_buffer(session);
    }
}

void app_runtime_load_code(const char* code)
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_load_code(session, code);
    }
}

void app_runtime_stop_execute()
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_stop_execute(session);
    }
}

void app_runtime_clear_code()
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_clear_code(session);
    }
}

void app_runtime_request_execute()
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_request_execute(session);
    }
}

void app_runtime_process_deferred()
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_process(session);
    }
}

int app_runtime_push_instance_from_file(const char* filename)
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        return runtime_session_push_instance_from_file(session, filename);
    }
    return -1;
}

void app_runtime_send_io()
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_send_io(session);
    }
}

void app_runtime_key(char key)
{
    RuntimeSession* session = app_runtime_get_current_session();
    if (!session) return;
    
    if (session->input_requested) {
        switch (key) {
            case '\b': {
                if (session->input_buffer_length > 0) {
                    session->input_buffer_length--;
                    session->input_buffer[session->input_buffer_length] = '\0';
                }
                break;
            }
            case '\n': {
                app_runtime_send_io();
                break;
            }
            default: {
                if (key >= 32 && key <= 126) {
                    if (session->input_buffer_length < sizeof(session->input_buffer) - 1) {
                        session->input_buffer[session->input_buffer_length] = key;
                        session->input_buffer_length++;
                        session->input_buffer[session->input_buffer_length] = '\0';
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
    RuntimeSession* session = app_runtime_get_current_session();
    if (session) {
        runtime_session_cleanup(session);
    }
}

void app_runtime_init()
{
    RuntimeSession* session = runtime_session_allocate(app_runtime_current_window_id);
    if (!session) {
        printf("ERROR: Failed to allocate runtime session\n");
        printf("Not enough memory for new runtime instance\n");
        return;
    }
    
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
}
