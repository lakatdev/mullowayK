#ifndef APP_EDITOR_H
#define APP_EDITOR_H

void app_editor_draw();
void app_editor_key(char key);
void app_editor_mouse(int x, int y);
void app_editor_init();
void app_editor_on_close();

char* app_editor_get_path_ptr();
char* app_editor_get_text_ptr();
void app_editor_set_length(unsigned int length);

#endif
