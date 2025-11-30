#include <userlib.h>
#include <memory.h>
#include <keyboard.h>
#include <storage.h>
#include <apps/runtime.h>

typedef enum {
    EDITOR_FIELD_MAIN,
    EDITOR_FIELD_PATH
} Editor_SelectedField;

char app_editor_buffer[16000000];
char app_editor_path[256] = "Untitled";
int app_editor_buffer_size = 0;
int app_editor_cursor_y = 0;
int app_editor_scroll = 0;
Editor_SelectedField app_editor_selected_field = EDITOR_FIELD_MAIN;


int app_editor_line_count()
{
    int lines = 1;
    for (int i = 0; i < app_editor_buffer_size; i++) {
        if (app_editor_buffer[i] == '\n') {
            lines++;
        }
    }
    return lines;
}

char* app_editor_get_line(int n)
{
    int line = 0;
    for (int i = 0; i < app_editor_buffer_size; i++) {
        if (line == n) {
            return &app_editor_buffer[i];
        }
        if (app_editor_buffer[i] == '\n') {
            line++;
        }
    }
    return (line == n) ? &app_editor_buffer[app_editor_buffer_size] : (void*)0;
}

void app_editor_draw()
{
    
    int visible_lines = (get_window_height() / 20) - 1;
    if (app_editor_cursor_y < app_editor_scroll) {
        app_editor_scroll = app_editor_cursor_y;
    }
    else if (app_editor_cursor_y >= app_editor_scroll + visible_lines) {
        app_editor_scroll = app_editor_cursor_y - visible_lines + 1;
    }

    int chars_x = (get_window_width() - 10) / 10;
    int chars_y = (get_window_height() / 20) - 1;
    draw_screen(THEME_BACKGROUND_COLOR);

    int lines = app_editor_line_count();
    for (int i = app_editor_scroll; i < lines && i < app_editor_scroll + chars_y; i++) {
        char* line = app_editor_get_line(i);
        if (line != (void*)0) {
            int len = 0;
            int visual_cols = 0;
            while ((line + len) < (app_editor_buffer + app_editor_buffer_size) &&
                   line[len] != '\n' && line[len] != '\0') {
                if (line[len] == '\t') {
                    int spaces = 4 - (visual_cols % 4);
                    if (visual_cols + spaces > chars_x) break;
                    visual_cols += spaces;
                }
                else {
                    if (visual_cols + 1 > chars_x) break;
                    visual_cols++;
                }
                len++;
                if (visual_cols >= chars_x) break;
            }
            if (i == app_editor_cursor_y && app_editor_selected_field == EDITOR_FIELD_MAIN) {
                draw_rect(0, (i - app_editor_scroll) * 20, get_window_width(), 20, THEME_HIGHLIGHT_COLOR);
            }
            draw_ntext(10, 15 + (i - app_editor_scroll) * 20, line, 20, len, THEME_TEXT_COLOR);
        }
    }

    if (app_editor_selected_field == EDITOR_FIELD_PATH) {
        draw_rect(0, get_window_height() - 20, get_window_width(), 20, THEME_HIGHLIGHT_COLOR);
    }
    draw_line(0, get_window_height() - 20, get_window_width(), get_window_height() - 20, 3, THEME_TEXT_COLOR);

    char toolbar[] = "line: XXXXX/XXXXX";
    int lines_count = app_editor_line_count();
    int line_number = app_editor_cursor_y + 1;

    toolbar[6] = '0' + (line_number / 10000) % 10;
    toolbar[7] = '0' + (line_number / 1000) % 10;
    toolbar[8] = '0' + (line_number / 100) % 10;
    toolbar[9] = '0' + (line_number / 10) % 10;
    toolbar[10] = '0' + (line_number % 10);
    toolbar[12] = '0' + (lines_count / 10000) % 10;
    toolbar[13] = '0' + (lines_count / 1000) % 10;
    toolbar[14] = '0' + (lines_count / 100) % 10;
    toolbar[15] = '0' + (lines_count / 10) % 10;
    toolbar[16] = '0' + (lines_count % 10);

    draw_text(0, get_window_height() - 3, toolbar, 20, THEME_TEXT_COLOR);
    draw_text(180, get_window_height() - 3, app_editor_path, 20, THEME_TEXT_COLOR);
}

void app_editor_up()
{
    if (app_editor_selected_field != EDITOR_FIELD_MAIN) {
        return;
    }
    if (app_editor_cursor_y > 0) {
        app_editor_cursor_y--;
        char* line = app_editor_get_line(app_editor_cursor_y);
        int len = 0;
        while ((line + len) < (app_editor_buffer + app_editor_buffer_size) &&
               line[len] != '\n' && line[len] != '\0') {
            len++;
        }
        if (app_editor_cursor_y < app_editor_scroll) {
            app_editor_scroll--;
        }
    }
}

void app_editor_down()
{
    if (app_editor_selected_field != EDITOR_FIELD_MAIN) {
        return;
    }
    int lines = app_editor_line_count();
    int visible_lines = (get_window_height() / 20) - 1;
    if (app_editor_cursor_y < lines - 1) {
        app_editor_cursor_y++;
        char* line = app_editor_get_line(app_editor_cursor_y);
        int len = 0;
        while ((line + len) < (app_editor_buffer + app_editor_buffer_size) &&
               line[len] != '\n' && line[len] != '\0') {
            len++;
        }
        if (app_editor_cursor_y >= app_editor_scroll + visible_lines) {
            app_editor_scroll++;
        }
    }
}

void app_editor_key(char key)
{
    if (app_editor_selected_field == EDITOR_FIELD_PATH) {
        int length = strlen(app_editor_path);
        switch (key) {
            case '\b': {
                if (length > 0) {
                    app_editor_path[length - 1] = '\0';
                }
                break;
            }
            case '\n': {
                app_editor_selected_field = EDITOR_FIELD_MAIN;
                break;
            }
            default: {
                if (key < 32 || key > 126) {
                    return;
                }
                if (length < sizeof(app_editor_path) - 1) {
                    app_editor_path[length++] = key;
                    app_editor_path[length] = '\0';
                }
                break;
            }
        }
        return;
    }

    switch (key) {
        case KEY_LEFT: {
            break;
        }
        case KEY_RIGHT: {
            break;
        }
        case KEY_UP: {
            app_editor_up();
            break;
        }
        case KEY_DOWN: {
            app_editor_down();
            break;
        }
        case '\n': {
            if (app_editor_buffer_size >= sizeof(app_editor_buffer) - 1) {
                return;
            }

            char* line = app_editor_get_line(app_editor_cursor_y);
            if (!line) return;
            int offset = line - app_editor_buffer;
            while (offset < app_editor_buffer_size && app_editor_buffer[offset] != '\n') {
                offset++;
            }

            memmove(app_editor_buffer + offset + 1, app_editor_buffer + offset, app_editor_buffer_size - offset);
            app_editor_buffer[offset] = '\n';
            app_editor_buffer_size++;

            app_editor_cursor_y++;
            break;
        }
        case '\b': {
            char* line = app_editor_get_line(app_editor_cursor_y);
            if (!line) return;

            int line_start = line - app_editor_buffer;
            int line_end = line_start;

            while (line_end < app_editor_buffer_size &&
                app_editor_buffer[line_end] != '\n') {
                line_end++;
            }

            int line_length = line_end - line_start;

            if (line_length == 0) {
                if (app_editor_cursor_y == 0) {
                    return;
                }

                int prev_line_end = line_start - 1;
                if (prev_line_end < 0 || app_editor_buffer[prev_line_end] != '\n') {
                    return;
                }

                memmove(app_editor_buffer + prev_line_end, app_editor_buffer + prev_line_end + 1, app_editor_buffer_size - (prev_line_end + 1));
                app_editor_buffer_size--;
                app_editor_cursor_y--;

                char* prev_line = app_editor_get_line(app_editor_cursor_y);
                int len = 0;
                while ((prev_line + len) < app_editor_buffer + app_editor_buffer_size && prev_line[len] != '\n') {
                    len++;
                }

            }
            else {
                if (line_end == line_start) return;
                memmove(app_editor_buffer + line_end - 1, app_editor_buffer + line_end, app_editor_buffer_size - line_end);

                app_editor_buffer_size--;
            }
            break;
        }
        default: {
            if (app_editor_buffer_size >= sizeof(app_editor_buffer) - 1) {
                return;
            }
            char* line = app_editor_get_line(app_editor_cursor_y);
            if (!line) {
                return;
            }

            int offset = line - app_editor_buffer;
            while (offset < app_editor_buffer_size && app_editor_buffer[offset] != '\n') {
                offset++;
            }

            memmove(app_editor_buffer + offset + 1, app_editor_buffer + offset, app_editor_buffer_size - offset);
            app_editor_buffer[offset] = key;
            app_editor_buffer_size++;

            break;
        }
    }
}

void app_editor_edit_path()
{
    app_editor_selected_field = EDITOR_FIELD_PATH;
}

void app_editor_edit_main()
{
    app_editor_selected_field = EDITOR_FIELD_MAIN;
}

void app_editor_mouse(int x, int y)
{
    if (y >= get_window_height() - 20) {
        app_editor_edit_path();
        return;
    }
    app_editor_edit_main();
}

char* app_editor_get_text_ptr()
{
    return app_editor_buffer;
}

char* app_editor_get_path_ptr()
{
    return app_editor_path;
}

void app_editor_set_length(unsigned int length)
{
    if (length < sizeof(app_editor_buffer)) {
        app_editor_buffer_size = length;
    } else {
        app_editor_buffer_size = sizeof(app_editor_buffer) - 1;
    }
    app_editor_buffer[app_editor_buffer_size] = '\0';
    app_editor_cursor_y = 0;
    app_editor_selected_field = EDITOR_FIELD_MAIN;
}

void app_editor_save()
{
    if (strlen(app_editor_path) <= 0) {
        return;
    }
    write_to_storage(app_editor_path, app_editor_buffer, app_editor_buffer_size);
}

void app_editor_on_close()
{
    
}

void app_editor_new(int result)
{
    if (result) {
        app_editor_buffer_size = 0;
        app_editor_cursor_y = 0;
        app_editor_scroll = 0;
        app_editor_selected_field = EDITOR_FIELD_MAIN;
        strncpy(app_editor_path, "Untitled", sizeof(app_editor_path) - 1);
    }
}

void app_editor_new_clicked()
{
    confirm_dialog(app_editor_new);
}

void app_editor_run()
{
    if (app_editor_buffer_size <= 0) {
        return;
    }
    if (app_editor_buffer_size >= sizeof(app_editor_buffer)) {
        app_editor_buffer_size = sizeof(app_editor_buffer) - 1;
    }
    app_editor_buffer[app_editor_buffer_size] = '\0';
    app_runtime_load_code(app_editor_buffer);
    open_app("Runtime");
    app_runtime_request_execute();
}

void app_editor_init()
{
    app_editor_selected_field = EDITOR_FIELD_MAIN;

    add_app_menu_item((MenuItem) {
        .name = "Save",
        .action = app_editor_save
    });

    add_app_menu_item((MenuItem) {
        .name = "Run",
        .action = app_editor_run
    });

    add_app_menu_item((MenuItem) {
        .name = "New",
        .action = app_editor_new_clicked
    });

    add_app_menu_item((MenuItem) {
        .name = "Up",
        .action = app_editor_up
    });

    add_app_menu_item((MenuItem) {
        .name = "Down",
        .action = app_editor_down
    });

    add_app_menu_item((MenuItem) {
        .name = "Edit path",
        .action = app_editor_edit_path
    });

    add_app_menu_item((MenuItem) {
        .name = "Edit main",
        .action = app_editor_edit_main
    });
}
