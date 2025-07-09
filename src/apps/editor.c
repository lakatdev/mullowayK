#include <userlib.h>
#include <memory.h>

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
    int chars_y = get_window_height() / 20;
    draw_screen(THEME_BACKGROUND_COLOR);

    int lines = app_editor_line_count();
    for (int i = app_editor_scroll; i < lines && i < app_editor_scroll + chars_y; i++) {
        char* line = app_editor_get_line(i);
        if (line != (void*)0) {
            int len = 0;
            while (line[len] != '\n' && line[len] != '\0') {
                len++;
            }
            if (len > chars_x) {
                len = chars_x;
            }
            
            if (i == app_editor_cursor_y) {
                draw_rect(0, (i - app_editor_scroll) * 20, get_window_width(), 20, THEME_HIGHLIGHT_COLOR);
            }
            
            draw_ntext(10, 20 + (i - app_editor_scroll) * 20, line, 20, len, THEME_TEXT_COLOR);
        }
    }

    draw_rect(0, get_window_height() - 20, get_window_width(), 20, THEME_HIGHLIGHT_COLOR);
    draw_line(0, get_window_height() - 20, get_window_width(), get_window_height() - 20, 3, THEME_TEXT_COLOR);
    draw_text(0, get_window_height() - 3, " up down line: XXXXX/XXXXX", 20, THEME_TEXT_COLOR);
}

void app_editor_key(char key)
{
    
}

void app_editor_up()
{

}

void app_editor_down()
{

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
