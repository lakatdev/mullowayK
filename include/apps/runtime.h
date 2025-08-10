#ifndef APP_RUNTIME_H
#define APP_RUNTIME_H

void app_runtime_draw();
void app_runtime_key(char key);
void app_runtime_mouse(int x, int y);
void app_runtime_init();

void app_runtime_load_code(const char* code);
void app_runtime_print(const char* str);
void app_runtime_print_char(char c);
void app_runtime_print_int(int value);
void app_runtime_print_float(float value);
void app_runtime_clear_buffer();

#endif
