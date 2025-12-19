#include <tools/userlib.h>
#include <system/interface.h>
#include <system/memory.h>

#define APP_DEBUG_WIDTH 80
#define APP_DEBUG_HEIGHT 25

void app_debug_draw()
{
    draw_rect(0, 0, 10 * APP_DEBUG_WIDTH, 20 * APP_DEBUG_HEIGHT, 0, 0, 0);
    
    for (int line = 0; line < APP_DEBUG_HEIGHT; line++) {
        char line_buffer[APP_DEBUG_WIDTH + 1] = {0};
        
        for (int col = 0; col < APP_DEBUG_WIDTH; col++) {
            int index = line * APP_DEBUG_WIDTH + col;
            if (index < 80 * 25) {
                line_buffer[col] = boot_messages[index];
            }
        }
        
        draw_text(0, 15 + line * 20, line_buffer, 20, 255, 255, 255);
    }
}

void app_debug_key(char key)
{

}

void app_debug_mouse(int x, int y)
{

}

void app_debug_print_boot_messages()
{

}

void app_debug_on_close()
{

}

void app_debug_init()
{

}
