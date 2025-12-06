#include <userlib.h>
#include <storage.h>
#include <memory.h>
#include <interface.h>
#include <apps/editor.h>
#include <apps/runtime.h>
#include <interpreter/interpreter.h>
#include <desktop.h>

#define FILES_ON_PAGE 10

extern char storage_initialized;

char app_files_record_name_list[FILES_ON_PAGE][STORAGE_KEY_SIZE];
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

    int displayed_page = app_files_page + 1;
    char page_info[] = "Page XXXX/XXXX";
    page_info[5] = '0' + (displayed_page / 1000) % 10;
    page_info[6] = '0' + (displayed_page / 100) % 10;
    page_info[7] = '0' + (displayed_page / 10) % 10;
    page_info[8] = '0' + (displayed_page % 10);

    int total_pages = (app_files_record_count + FILES_ON_PAGE - 1) / FILES_ON_PAGE;
    page_info[10] = '0' + (total_pages / 1000) % 10;
    page_info[11] = '0' + (total_pages / 100) % 10;
    page_info[12] = '0' + (total_pages / 10) % 10;
    page_info[13] = '0' + (total_pages % 10);

    draw_text(10, 25, page_info, 24, THEME_TEXT_COLOR);

    for (int i = 0; i < app_files_records_on_page; i++) {
        int y = 60 + i * 24;
        if (i == app_files_selected_record) {
            int length = strlen(app_files_record_name_list[i]);
            draw_rect(10, y - 20, length * 12, 24, THEME_HIGHLIGHT_COLOR);
            draw_text(10, y, app_files_record_name_list[i], 24, THEME_TEXT_COLOR);
        }
        else {
            draw_text(10, y, app_files_record_name_list[i], 24, THEME_TEXT_COLOR);
        }
    }
}

void app_files_key(char key)
{

}

void app_files_mouse(int x, int y)
{
    if (x < 10 || y < 40 || y > 40 + app_files_records_on_page * 24) {
        return;
    }

    int index = (y - 40) / 24;
    if (index >= 0 && index < app_files_records_on_page) {
        app_files_selected_record = index;
    }
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
        if (get_record_key(i, key)) {
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

void app_files_refresh()
{
    app_files_page = 0;
    app_files_read_files();
}

void TEST_app_files_write_files()
{
    write_to_storage("image.bmp", "this is an image", 17);
    write_to_storage("text.txt", "this is a text", 16);
    write_to_storage("video.mp4", "this is a video", 16);
    write_to_storage("program.k", "this is an application", 23);
}

void app_files_edit()
{
    if (app_files_selected_record < 0 || app_files_selected_record >= app_files_records_on_page) {
        return;
    }
    unsigned int size = 0;
    read_from_storage(app_files_record_name_list[app_files_selected_record], app_editor_get_text_ptr(), &size);
    app_editor_set_length(size);
    strncpy(app_editor_get_path_ptr(), app_files_record_name_list[app_files_selected_record], 256);
    open_app("Editor");
}

void app_files_delete(int result)
{
    if (result) {
        char key[STORAGE_KEY_SIZE];
        strncpy(key, app_files_record_name_list[app_files_selected_record], STORAGE_KEY_SIZE);
        key[STORAGE_KEY_SIZE - 1] = '\0';
        
        delete_from_storage(key);   
        app_files_read_files();
    }
}

void app_files_delete_clicked()
{
    if (app_files_selected_record < 0 || app_files_selected_record >= app_files_records_on_page) {
        return;
    }
    confirm_dialog(app_files_delete);
}

void app_files_run()
{
    if (app_files_selected_record < 0 || app_files_selected_record >= app_files_records_on_page) {
        return;
    }
    unsigned int size = 0;
    read_from_storage(app_files_record_name_list[app_files_selected_record], interpreter_public_buffer, &size);
    interpreter_public_buffer[size] = '\0';
    
    int runtime_window_id = desktop_create_runtime_window();
    if (runtime_window_id >= 0) {
        app_runtime_set_window_id(runtime_window_id);
        app_runtime_load_code(interpreter_public_buffer);
        app_runtime_request_execute();
    }
}

void app_files_make_editable(int result)
{
    if (result) {
        unsigned int file_size = 0;
        read_from_storage(app_files_record_name_list[app_files_selected_record], interpreter_public_buffer, &file_size);
        
        if (file_size < sizeof(int)) {
            printf("Warning: File too small to convert.\n");
            return;
        }
        
        int stored_size;
        memcpy(&stored_size, interpreter_public_buffer, sizeof(int));
        
        if (stored_size >= 0 && stored_size == (int)(file_size - sizeof(int)) && stored_size < STORAGE_RECORD_SIZE) {
            char* text_data = interpreter_public_buffer + sizeof(int);
            unsigned int text_size = stored_size;
            write_to_storage(app_files_record_name_list[app_files_selected_record], text_data, text_size);
            printf("File converted to editable format.\n");
        }
        else {
            printf("File is already editable or not a text file.\n");
        }
    }
}

void app_files_make_editable_clicked()
{
    if (app_files_selected_record < 0 || app_files_selected_record >= app_files_records_on_page) {
        return;
    }
    confirm_dialog(app_files_make_editable);
}

void app_files_on_close()
{

}

void app_files_init()
{
    app_files_page = 0;
    app_files_record_count = 0;
    app_files_selected_record = -1;
    app_files_read_files();

    add_app_menu_item((MenuItem) {
        .name = "Refresh",
        .action = app_files_refresh
    });

    add_app_menu_item((MenuItem) {
        .name = "Next",
        .action = app_files_next
    });

    add_app_menu_item((MenuItem) {
        .name = "Previous",
        .action = app_files_prev
    });

    add_app_menu_item((MenuItem) {
        .name = "Edit",
        .action = app_files_edit
    });

    add_app_menu_item((MenuItem) {
        .name = "Run",
        .action = app_files_run
    });

    add_app_menu_item((MenuItem) {
        .name = "Make Editable",
        .action = app_files_make_editable_clicked
    });

    add_app_menu_item((MenuItem) {
        .name = "Delete",
        .action = app_files_delete_clicked
    });

    add_app_menu_item((MenuItem) {
        .name = "[TEST] Write",
        .action = TEST_app_files_write_files
    });
}
