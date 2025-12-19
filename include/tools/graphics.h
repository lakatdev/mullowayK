#ifndef GRAPHICS_H
#define GRAPHICS_H

#define WIDTH 800
#define HEIGHT 600

#define SYSTEM_FONT ubuntu_mono_v2_mfp

#define BITGET(var, pos) (((var) & (1 << pos)))

extern unsigned char convert_ascii[];

void init_graphics(unsigned char* buffer);
void system_draw_screen(unsigned char r, unsigned char g, unsigned char b);
void system_draw_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
void system_draw_rect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
void system_draw_line(int x1, int y1, int x2, int y2, int width, unsigned char r, unsigned char g, unsigned char b);
void system_draw_circle(int x, int y, int radius, int width, unsigned char r, unsigned char g, unsigned char b);
void system_draw_text(int x, int y, const char* text, int size, unsigned char r, unsigned char g, unsigned char b);
void system_draw_image(int x, int y, int width, int height, unsigned char* image, unsigned char r, unsigned char g, unsigned char b);
void system_draw_thin_line(int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b);
void update_video();
void invalidate_buffer();
void system_reset_clip_region();
void system_set_clip_region(int x, int y, int width, int height);
void system_get_clip_region(int* x, int* y, int* width, int* height);
void force_draw();

#endif
