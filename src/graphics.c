#include <interface.h>
#include <graphics.h>
#include <memory.h>
#include <ubuntu-mono-v2.mfp.h>
#include <math.h>

#define BITGET(var, pos) (((var) & (1 << pos)))

unsigned char* video;
unsigned char buffer[WIDTH * HEIGHT * 4];
unsigned char video_live = 0;
unsigned char frame_cap = 0;

unsigned char convert_ascii[128] = {
    0,0,0,0,0,0,0,0,108,109,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,106,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,64,65,66,67,
    68,69,70,71,72,73,89,90,91,92,93,94,95,32,33,34,36,37,38,39,41,43,
    44,46,47,48,49,50,51,52,53,54,56,57,59,60,61,62,63,96,98,97,99,100,
    101,0,1,2,4,5,6,7,9,11,12,14,15,16,17,18,19,20,21,22,24,25,27,28,
    29,30,31,102,103,104,105,0
};

void init_graphics(unsigned char* buffer)
{
    video_live = 1;
    video = buffer;
}

void draw_screen(unsigned char r, unsigned char g, unsigned char b)
{
    for (int i = 0; i < WIDTH * HEIGHT * 4; i += 4) {
        buffer[i] = b;
        buffer[i + 1] = g;
        buffer[i + 2] = r;
        buffer[i + 3] = 0;
    }
}

void draw_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        return;
    }

    int i = (y * WIDTH + x) * 4;
    buffer[i] = b;
    buffer[i + 1] = g;
    buffer[i + 2] = r;
    buffer[i + 3] = 0;
}

void draw_rect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b)
{
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            draw_pixel(x + i, y + j, r, g, b);
        }
    }
}

void draw_thin_line(int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        draw_pixel(x1, y1, r, g, b);

        if (x1 == x2 && y1 == y2) {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void draw_line(int x1, int y1, int x2, int y2, int width, unsigned char r, unsigned char g, unsigned char b)
{
    int dxabs = abs(x2 - x1);
    int dyabs = abs(y2 - y1);

    int start = (width >> 1) - width + 1;
    int end = width - abs(start);

    if (dxabs >= dyabs) {
        for (int i = start; i < end; i++) {
            draw_thin_line(x1, y1 + i, x2, y2 + i, r, g, b);
        }
    }
    else {
        for (int i = start; i < end; i++) {
            draw_thin_line(x1 + i, y1, x2 + i, y2, r, g, b);
        }
    }
}

void draw_circle(int x, int y, int radius, int width, unsigned char r, unsigned char g, unsigned char b)
{

}

void draw_character(int x, int y, unsigned char* data, unsigned char r, unsigned char g, unsigned char b)
{
    int h = y;
    int w = x;
    for (int i = 0; i < (data[0] << 3); i++) {
        int j = 7 - (i % 8);
        if (BITGET(data[(i >> 3) + 3], j)) {
            draw_pixel(w, h, r, g, b);
        }
        w++;
        if (w >= data[1] + x) {
            h--;
            w = x;
        }
    }
}

void render_character(unsigned char code, int x, int y, int size, unsigned char r, unsigned char g, unsigned char b)
{
    int pos = 0;

    // Check if the size is available
    for (int  i = 0; i < ubuntu_mono_v2_mfp[0]; i++) {
        if (ubuntu_mono_v2_mfp[i + 2] == size) {
            pos = i;
            break;
        }
        if (i == ubuntu_mono_v2_mfp[0] - 1) {
            return;
        }
    }

    // Set up the cursor
    int cursor = ubuntu_mono_v2_mfp[0] + 2;

    // Get position of selected size
    for (int i = 0; i < pos * ubuntu_mono_v2_mfp[1]; i++) {
        cursor += ubuntu_mono_v2_mfp[cursor] + 3;
    }

    // Get character
    for (int i = 0; i < code; i++) {
        cursor += ubuntu_mono_v2_mfp[cursor] + 3;
    }

    int abs_size = ubuntu_mono_v2_mfp[cursor] + 3;
    unsigned char data[abs_size];
    memcpy((unsigned char*)&data, &ubuntu_mono_v2_mfp[cursor], abs_size);

    x += ((size / 2) - data[1]) / 2;
    switch (data[2]) {
        case (1): {
            y += size * 0.16;
            break;
        }
        case (2): {
            y -= size / 3;
            break;
        }
        case (3): {
            y -= size / 10;
            break;
        }
    }

    draw_character(x, y, (unsigned char*)&data, r, g, b);
}

void draw_text(int x, int y, const char* text, int size, unsigned char r, unsigned char g, unsigned char b)
{
    int dx = x;
    for (int i = 0; text[i] != '\0'; i++) {
        switch (text[i]) {
            case (' '): {
                x += size / 2;
                break;
            }
            case ('\n'): {
                x = dx;
                y += size;
                break;
            }
            case ('\t'): {
                x += size * 2;
                break;
            }
            default: {
                render_character(convert_ascii[text[i]], x, y, size, r, g, b);
                x += size / 2;
                break;
            }
        }
    }
}

void draw_image(int x, int y, int width, int height, unsigned char* image, unsigned char r, unsigned char g, unsigned char b)
{
    int h = y;
    int w = x;
    for (int i = 0; i < width * height; i++) {
        int j = 7 - (i % 8);
        if (BITGET(image[i >> 3], j)) {
            draw_pixel(w, h, r, g, b);
        }
        w++;
        if (w >= width + x) {
            h++;
            w = x;
        }
    }
}

void invalidate_buffer()
{
    frame_cap = 1;
}

void update_video()
{
    if (video_live && frame_cap) {
        memcpy_sse(video, buffer, WIDTH * HEIGHT * 4);
        frame_cap = 0;
    }
}
