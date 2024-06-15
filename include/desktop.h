#ifndef DESKTOP_H
#define DESKTOP_H

void init_desktop();
void draw_desktop();
void key_press(char key);
void mouse_click(int x, int y);
void invalidate();

#endif