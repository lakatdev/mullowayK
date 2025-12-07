#include <userlib.h>
#include <storage.h>

typedef struct __attribute__((packed)) {
    unsigned short type;
    unsigned int size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;
} BMP_FileHeader;

typedef struct __attribute__((packed)) {
    unsigned int header_size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bits_per_pixel;
    unsigned int compression;
    unsigned int image_size;
    int x_pixels_per_meter;
    int y_pixels_per_meter;
    unsigned int colors_used;
    unsigned int colors_important;
} BMP_InfoHeader;

char app_image_viewer_loaded_image[16000000];
int app_image_viewer_loaded_image_width = 0;
int app_image_viewer_loaded_image_height = 0;

void app_image_viewer_display_bmp(char* path)
{
    app_image_viewer_loaded_image_width = 0;
    app_image_viewer_loaded_image_height = 0;
    
    unsigned int file_size = 0;
    read_from_storage(path, app_image_viewer_loaded_image, &file_size);
    
    if (file_size < sizeof(BMP_FileHeader) + sizeof(BMP_InfoHeader)) {
        printf("Image Viewer: Invalid BMP.\n");
        return;
    }
    
    BMP_FileHeader* file_header = (BMP_FileHeader*)app_image_viewer_loaded_image;
    BMP_InfoHeader* info_header = (BMP_InfoHeader*)(app_image_viewer_loaded_image + sizeof(BMP_FileHeader));
    
    if (file_header->type != 0x4D42) {
        printf("Image Viewer: Not a valid BMP file.\n");
        return;
    }
    
    if (info_header->header_size != 40) {
        printf("Image Viewer: Unsupported BMP format.\n");
        return;
    }
    
    if (info_header->width <= 0 || info_header->height == 0) {
        printf("Image Viewer: Invalid dimensions.\n");
        return;
    }
    
    if (info_header->bits_per_pixel != 24) {
        printf("Image Viewer: Only 24-bit BMP is supported.\n");
        return;
    }
    
    if (info_header->compression != 0) {
        printf("Image Viewer: Compressed BMP is not supported.\n");
        return;
    }
    
    if (file_header->offset >= file_size) {
        printf("Image Viewer: Invalid pixel data offset.\n");
        return;
    }
    
    int height = info_header->height;
    int is_bottom_up = 1;
    if (height < 0) {
        height = -height;
        is_bottom_up = 0;
    }
    
    int width = info_header->width;
    int row_size = ((width * 3 + 3) / 4) * 4;
    unsigned int expected_data_size = row_size * height;
    if ((unsigned int)(width * height * 3) > sizeof(app_image_viewer_loaded_image)) {
        printf("Image Viewer: Image too large.\n");
        return;
    }
    
    if (file_header->offset + expected_data_size > file_size) {
        printf("Image Viewer: Incomplete pixel data.\n");
        return;
    }
    
    char* src = app_image_viewer_loaded_image + file_header->offset;
    char* temp_buffer = app_image_viewer_loaded_image + file_size;
    
    for (unsigned int i = 0; i < expected_data_size && i < (sizeof(app_image_viewer_loaded_image) - file_size); i++) {
        temp_buffer[i] = src[i];
    }
    
    char* dest = app_image_viewer_loaded_image;
    for (int y = 0; y < height; y++) {
        int src_row = is_bottom_up ? (height - 1 - y) : y;
        char* row_src = temp_buffer + (src_row * row_size);
        
        for (int x = 0; x < width; x++) {
            dest[0] = row_src[x * 3 + 2]; // r
            dest[1] = row_src[x * 3 + 1]; // g
            dest[2] = row_src[x * 3 + 0]; // b
            dest += 3;
        }
    }
    
    app_image_viewer_loaded_image_width = width;
    app_image_viewer_loaded_image_height = height;
}

void app_image_viewer_draw()
{
    draw_screen(0, 0, 0);
    if (app_image_viewer_loaded_image_width == 0 || app_image_viewer_loaded_image_height == 0) {
        draw_text(10, 40, "No image loaded.", 24, 255, 255, 255);
        return;
    }
    
    int draw_width = app_image_viewer_loaded_image_width;
    int draw_height = app_image_viewer_loaded_image_height;
    int window_width = get_window_width();
    int window_height = get_window_height();

    if (draw_width > window_width) {
        float scale = (float)window_width / (float)draw_width;
        draw_width = window_width;
        draw_height = (int)(draw_height * scale);
    }

    if (draw_height > window_height) {
        float scale = (float)window_height / (float)draw_height;
        draw_height = window_height;
        draw_width = (int)(draw_width * scale);
    }

    int offset_x = (window_width - draw_width) / 2;
    int offset_y = (window_height - draw_height) / 2;

    for (int y = 0; y < draw_height; y++) {
        for (int x = 0; x < draw_width; x++) {
            int src_x = x * app_image_viewer_loaded_image_width / draw_width;
            int src_y = y * app_image_viewer_loaded_image_height / draw_height;
            char* pixel = app_image_viewer_loaded_image + (src_y * app_image_viewer_loaded_image_width + src_x) * 3;
            unsigned char r = (unsigned char)pixel[0];
            unsigned char g = (unsigned char)pixel[1];
            unsigned char b = (unsigned char)pixel[2];
            draw_pixel(offset_x + x, offset_y + y, r, g, b);
        }
    }
}

void app_image_viewer_key(char key)
{
    
}

void app_image_viewer_mouse(int x, int y)
{

}

void app_image_viewer_clear()
{
    app_image_viewer_loaded_image_width = 0;
    app_image_viewer_loaded_image_height = 0;
}

void app_image_viewer_init()
{
    add_app_menu_item((MenuItem) {
        .name = "Clear",
        .action = app_image_viewer_clear
    });
}

void app_image_viewer_on_close()
{

}
