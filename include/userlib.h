#ifndef USER_GRAPHICS_H
#define USER_GRAPHICS_H

#define THEME_BACKGROUND_COLOR 249, 249, 224
#define THEME_ACCENT_COLOR 135, 206, 235
#define THEME_TEXT_COLOR 0, 0, 0
#define THEME_HIGHLIGHT_COLOR 58, 166, 185

typedef struct {
    char name[32];
    void (*action)();
} MenuItem;

void draw_screen(unsigned char r, unsigned char g, unsigned char b);
void draw_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
void draw_rect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
void draw_line(int x1, int y1, int x2, int y2, int width, unsigned char r, unsigned char g, unsigned char b);
void draw_circle(int x, int y, int radius, int width, unsigned char r, unsigned char g, unsigned char b);
void draw_text(int x, int y, const char* text, int size, unsigned char r, unsigned char g, unsigned char b);
void draw_image(int x, int y, int width, int height, unsigned char* image, unsigned char r, unsigned char g, unsigned char b);
int get_window_width();
int get_window_height();
void serial_write(const unsigned char* data, unsigned int size);
void serial_read(unsigned char* data, unsigned int size);

void add_app_menu_item(MenuItem menu_item);

#endif
