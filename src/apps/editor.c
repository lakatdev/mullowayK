#include <userlib.h>
#include <memory.h>
#include <keyboard.h>

char app_editor_buffer[32000000];
int app_editor_buffer_size = 0;
int app_editor_cursor_x = 0;
int app_editor_cursor_y = 0;
int app_editor_scroll = 0;

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
                } else {
                    if (visual_cols + 1 > chars_x) break;
                    visual_cols++;
                }
                len++;
                if (visual_cols >= chars_x) break;
            }
            if (i == app_editor_cursor_y) {
                draw_rect(0, (i - app_editor_scroll) * 20, get_window_width(), 20, THEME_HIGHLIGHT_COLOR);
            }
            draw_ntext(10, 15 + (i - app_editor_scroll) * 20, line, 20, len, THEME_TEXT_COLOR);
        }
    }

    draw_rect(0, get_window_height() - 20, get_window_width(), 20, THEME_HIGHLIGHT_COLOR);
    draw_line(0, get_window_height() - 20, get_window_width(), get_window_height() - 20, 3, THEME_TEXT_COLOR);

    char toolbar[] = " up down line: XXXXX/XXXXX";
    int lines_count = app_editor_line_count();
    int line_number = app_editor_cursor_y + 1;

    toolbar[15] = '0' + (line_number / 10000) % 10;
    toolbar[16] = '0' + (line_number / 1000) % 10;
    toolbar[17] = '0' + (line_number / 100) % 10;
    toolbar[18] = '0' + (line_number / 10) % 10;
    toolbar[19] = '0' + (line_number % 10);
    toolbar[21] = '0' + (lines_count / 10000) % 10;
    toolbar[22] = '0' + (lines_count / 1000) % 10;
    toolbar[23] = '0' + (lines_count / 100) % 10;
    toolbar[24] = '0' + (lines_count / 10) % 10;
    toolbar[25] = '0' + (lines_count % 10);

    draw_text(0, get_window_height() - 3, toolbar, 20, THEME_TEXT_COLOR);
}

void app_editor_up()
{
    if (app_editor_cursor_y > 0) {
        app_editor_cursor_y--;
        char* line = app_editor_get_line(app_editor_cursor_y);
        int len = 0;
        while ((line + len) < (app_editor_buffer + app_editor_buffer_size) &&
               line[len] != '\n' && line[len] != '\0') {
            len++;
        }
        app_editor_cursor_x = len;
        if (app_editor_cursor_y < app_editor_scroll) {
            app_editor_scroll--;
        }
    }
}

void app_editor_down()
{
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
        app_editor_cursor_x = len;
        if (app_editor_cursor_y >= app_editor_scroll + visible_lines) {
            app_editor_scroll++;
        }
    }
}

void app_editor_key(char key)
{
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
            app_editor_cursor_x = 0;
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
                app_editor_cursor_x = len;

            }
            else {
                if (line_end == line_start) return;
                memmove(app_editor_buffer + line_end - 1, app_editor_buffer + line_end, app_editor_buffer_size - line_end);

                app_editor_buffer_size--;
                if (app_editor_cursor_x > 0) {
                    app_editor_cursor_x--;
                }
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

            app_editor_cursor_x++;
            break;
        }
    }
}

void app_editor_mouse(int x, int y)
{
    if (y >= get_window_height() - 20) {
        if (x < 40) {
            app_editor_up();
        }
        else if (x >= 40 && x < 80) {
            app_editor_down();
        }
    }
}

char* app_editor_get_text_ptr()
{
    return app_editor_buffer;
}

void app_editor_set_length(unsigned int length)
{
    if (length < sizeof(app_editor_buffer)) {
        app_editor_buffer_size = length;
    } else {
        app_editor_buffer_size = sizeof(app_editor_buffer) - 1;
    }
    app_editor_buffer[app_editor_buffer_size] = '\0';
}

void app_editor_init()
{
    add_app_menu_item((MenuItem) {
        .name = "Save",
        .action = (void*)0
    });

    add_app_menu_item((MenuItem) {
        .name = "Run",
        .action = (void*)0
    });

    add_app_menu_item((MenuItem) {
        .name = "New",
        .action = (void*)0
    });

    add_app_menu_item((MenuItem) {
        .name = "Up",
        .action = app_editor_up
    });

    add_app_menu_item((MenuItem) {
        .name = "Down",
        .action = app_editor_down
    });
}
