#ifndef DESKTOP_H
#define DESKTOP_H

void init_desktop();
void draw_desktop();
void key_press(char key);
void mouse_click(int x, int y);
void invalidate();
void desktop_confirm_dialog(void (*callback)(int result));
void desktop_open_app(const char* app_name);
int desktop_create_runtime_window(const char* title);

#endif
