#include <graphics.h>
#include <math.h>
#include <fonts.mfp.h>
#include <memory.h>
#include <userlib.h>

void draw_screen(unsigned char r, unsigned char g, unsigned char b) 
{
    system_draw_rect(USER_WINDOW_X, USER_WINDOW_Y, USER_WINDOW_WIDTH, USER_WINDOW_HEIGHT, r, g, b);
}

void draw_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
    if (x < 0 || x >= USER_WINDOW_WIDTH || y < 0 || y >= USER_WINDOW_HEIGHT) {
        return;
    }
    system_draw_pixel(x + USER_WINDOW_X, y + USER_WINDOW_Y, r, g, b);
}

void draw_rect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b) 
{
    if (x < 0 || x >= USER_WINDOW_WIDTH || y < 0 || y >= USER_WINDOW_HEIGHT) {
        return;
    }
    width = (x + width > USER_WINDOW_WIDTH) ? USER_WINDOW_WIDTH - x : width;
    height = (y + height > USER_WINDOW_HEIGHT) ? USER_WINDOW_HEIGHT - y : height;
    system_draw_rect(x + USER_WINDOW_X, y + USER_WINDOW_Y, width, height, r, g, b);
}

void draw_line(int x1, int y1, int x2, int y2, int width, unsigned char r, unsigned char g, unsigned char b)
{
    if (x1 < 0 || x1 >= USER_WINDOW_WIDTH || y1 < 0 || y1 >= USER_WINDOW_HEIGHT) {
        return;
    }
    system_draw_line(x1 + USER_WINDOW_X, y1 + USER_WINDOW_Y, x2 + USER_WINDOW_X, y2 + USER_WINDOW_Y, width, r, g, b);


    int dxabs = abs(x2 - x1);
    int dyabs = abs(y2 - y1);

    int start = (width >> 1) - width + 1;
    int end = width - abs(start);

    if (dxabs >= dyabs) {
        for (int i = start; i < end; i++) {
            if (x1 < 0 || x1 >= USER_WINDOW_WIDTH || y1 + i < 0 || y1 + i >= USER_WINDOW_HEIGHT ||
                x2 < 0 || x2 >= USER_WINDOW_WIDTH || y2 + i < 0 || y2 + i >= USER_WINDOW_HEIGHT) {
                return;
            }
            system_draw_thin_line(x1 + USER_WINDOW_X, y1 + USER_WINDOW_Y + i, x2 + USER_WINDOW_X, y2 + USER_WINDOW_Y + i, r, g, b);
        }
    }
    else {
        for (int i = start; i < end; i++) {
            if (x1 + i < 0 || x1 + i >= USER_WINDOW_WIDTH || y1 < 0 || y1 >= USER_WINDOW_HEIGHT ||
                x2 + i < 0 || x2 + i >= USER_WINDOW_WIDTH || y2 < 0 || y2 >= USER_WINDOW_HEIGHT) {
                return;
            }
            system_draw_thin_line(x1 + USER_WINDOW_X + i, y1 + USER_WINDOW_Y, x2 + USER_WINDOW_X + i, y2 + USER_WINDOW_Y, r, g, b);
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
    for (int  i = 0; i < SYSTEM_FONT[0]; i++) {
        if (SYSTEM_FONT[i + 2] == size) {
            pos = i;
            break;
        }
        if (i == SYSTEM_FONT[0] - 1) {
            return;
        }
    }

    // Set up the cursor
    int cursor = SYSTEM_FONT[0] + 2;

    // Get position of selected size
    for (int i = 0; i < pos * SYSTEM_FONT[1]; i++) {
        cursor += SYSTEM_FONT[cursor] + 3;
    }

    // Get character
    for (int i = 0; i < code; i++) {
        cursor += SYSTEM_FONT[cursor] + 3;
    }

    int abs_size = SYSTEM_FONT[cursor] + 3;
    unsigned char data[abs_size];
    memcpy((unsigned char*)&data, &SYSTEM_FONT[cursor], abs_size);

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
    if (x < 0 || x >= USER_WINDOW_WIDTH || y < 0 || y >= USER_WINDOW_HEIGHT ||
        x + width < 0 || x + width >= USER_WINDOW_WIDTH || y + height < 0 || y + height >= USER_WINDOW_HEIGHT) {
        return;
    }
    system_draw_image(x + USER_WINDOW_X, y + USER_WINDOW_Y, width, height, image, r, g, b);
}