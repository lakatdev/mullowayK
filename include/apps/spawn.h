#ifndef APP_SPAWN_H
#define APP_SPAWN_H

void app_spawn_draw();
void app_spawn_key(char key);
void app_spawn_mouse(int x, int y);
void app_spawn_init();

void app_spawn_load_code(const char* code);
void app_spawn_print(const char* str);
void app_spawn_print_char(char c);
void app_spawn_print_int(int value);
void app_spawn_print_float(float value);
void app_spawn_clear_buffer();

#endif
