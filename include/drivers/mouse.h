#ifndef MOUSE_H
#define MOUSE_H

void handle_mouse();
void draw_cursor();
void init_mouse();
int get_mouse_x();
int get_mouse_y();
void set_mouse_pos(int x, int y);

extern int abs_x, abs_y;
extern char mouse_left;

#endif
