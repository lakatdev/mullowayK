#ifndef APP_IMAGE_VIEWER_H
#define APP_IMAGE_VIEWER_H

void app_image_viewer_draw();
void app_image_viewer_key(char key);
void app_image_viewer_mouse(int x, int y);
void app_image_viewer_init();
void app_image_viewer_on_close();

void app_image_viewer_display_bmp(char* path);

#endif
