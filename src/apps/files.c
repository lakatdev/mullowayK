#include <userlib.h>
#include <storage.h>
#include <memory.h>
#include <interface.h>

#define FILES_ON_PAGE 10

extern char storage_initialized;

char app_files_record_name_list[STORAGE_KEY_SIZE][FILES_ON_PAGE];
int app_files_page = 0;
int app_files_record_count = 0;
int app_files_selected_record = -1;
int app_files_records_on_page = 0;

void app_files_draw()
{
    draw_screen(THEME_BACKGROUND_COLOR);
    if (app_files_record_count == 0) {
        draw_text(10, 30, "No files found", 24, THEME_TEXT_COLOR);
        return;
    }

    char page_info[] = "Page XXXX";
    page_info[5] = '0' + (app_files_page / 1000) % 10;
    page_info[6] = '0' + (app_files_page / 100) % 10;
    page_info[7] = '0' + (app_files_page / 10) % 10;
    page_info[8] = '0' + (app_files_page % 10);
    draw_text(10, 25, page_info, 20, THEME_TEXT_COLOR);

    for (int i = 0; i < app_files_records_on_page; i++) {
        int y = 30 + i * 20;
        if (i == app_files_selected_record) {
            draw_text(10, y, app_files_record_name_list[i], 20, THEME_TEXT_COLOR);
        }
        else {
            draw_text(10, y, app_files_record_name_list[i], 20, THEME_TEXT_COLOR);
        }
    }
}

void app_files_key(char key)
{

}

void app_files_mouse(int x, int y)
{

}

void app_files_read_files()
{
    app_files_records_on_page = 0;
    app_files_record_count = get_record_count();

    printf("Files: found ");
    print_hex(app_files_record_count);
    printf(" records in storage.\n");

    int start_index = app_files_page * FILES_ON_PAGE;
    int end_index = start_index + FILES_ON_PAGE;
    if (end_index > app_files_record_count) {
        end_index = app_files_record_count;
    }
    for (int i = start_index; i < end_index; i++) {
        char key[STORAGE_KEY_SIZE];
        if (get_record_key(i, key) == 0) {
            strncpy(app_files_record_name_list[i - start_index], key, STORAGE_KEY_SIZE);
            app_files_record_name_list[i - start_index][STORAGE_KEY_SIZE - 1] = '\0';
        }
        else {
            app_files_record_name_list[i - start_index][0] = '\0';
        }
        app_files_records_on_page++;
    }
}

void app_files_next()
{
    app_files_record_count = get_record_count();
    if ((app_files_page + 1) * FILES_ON_PAGE < app_files_record_count) {
        app_files_page++;
    }
    app_files_read_files();
}

void app_files_prev()
{
    if (app_files_page > 0) {
        app_files_page--;
    }
    app_files_read_files();
}

void app_files_init()
{
    app_files_page = 0;
    app_files_record_count = 0;
    app_files_selected_record = -1;
    app_files_read_files();

    add_app_menu_item((MenuItem) {
        .name = "Refresh",
        .action = (void*)0
    });

    add_app_menu_item((MenuItem) {
        .name = "Edit",
        .action = (void*)0
    });

    add_app_menu_item((MenuItem) {
        .name = "Run",
        .action = (void*)0
    });

    add_app_menu_item((MenuItem) {
        .name = "Delete",
        .action = (void*)0
    });
}
