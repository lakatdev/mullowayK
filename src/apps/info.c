#include <userlib.h>

void app_info_draw()
{
    draw_screen(THEME_BACKGROUND_COLOR);
    draw_text(10, 30, "MullowayK 1.1.2 build 2025-11-30", 24, THEME_TEXT_COLOR);
    draw_text(10, 54, "Using Keszeg 4 interpreter.", 24, THEME_TEXT_COLOR);
    
    unsigned int mem_mb = get_memory_mb();
    
    char mem_str[16];
    char* digits = "0123456789";
    
    mem_str[0] = digits[(mem_mb / 10000) % 10];
    mem_str[1] = digits[(mem_mb / 1000) % 10];
    mem_str[2] = digits[(mem_mb / 100) % 10];
    mem_str[3] = digits[(mem_mb / 10) % 10];
    mem_str[4] = digits[mem_mb % 10];
    mem_str[5] = ' ';
    mem_str[6] = 'M';
    mem_str[7] = 'B';
    mem_str[8] = '\0';
    
    int start = 0;
    for (int i = 0; i < 5; i++) {
        if (mem_str[i] != '0') {
            start = i;
            break;
        }
    }
    
    draw_text(10, 78, "Available Memory: ", 24, THEME_TEXT_COLOR);
    draw_text(222, 78, &mem_str[start], 24, THEME_TEXT_COLOR);
}

void app_info_key(char key)
{

}

void app_info_on_close()
{

}

void app_info_mouse(int x, int y)
{

}

void app_info_init()
{

}
