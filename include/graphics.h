#ifndef GRAPHICS_H
#define GRAPHICS_H

#define WIDTH 1280
#define HEIGHT 720

void init_graphics(unsigned char* buffer);
void draw_screen(unsigned char r, unsigned char g, unsigned char b);
void draw_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
void draw_rect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
void draw_line(int x1, int y1, int x2, int y2, int width, unsigned char r, unsigned char g, unsigned char b);
void draw_circle(int x, int y, int radius, int width, unsigned char r, unsigned char g, unsigned char b);
void draw_text(int x, int y, const char* text, int size, unsigned char r, unsigned char g, unsigned char b);
void draw_image(int x, int y, int width, int height, unsigned char* image, unsigned char r, unsigned char g, unsigned char b);
void update_video();
void invalidate_buffer();

#endif
