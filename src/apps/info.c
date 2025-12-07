#include <userlib.h>
#include <apps/runtime_session.h>
#include <desktop.h>

void app_info_draw()
{
    draw_screen(THEME_BACKGROUND_COLOR);
    draw_text(10, 30, "MullowayK 2.1.0 build 2025-12-07", 24, THEME_TEXT_COLOR);
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
    
    // memory info

    draw_text(10, 78, "Available Memory: ", 24, THEME_TEXT_COLOR);
    draw_text(222, 78, &mem_str[start], 24, THEME_TEXT_COLOR);
    
    int instances_possible = runtime_session_get_max_count();

    int active_sessions = runtime_session_get_active_count();
    draw_text(10, 102, "Runtime Instances: ", 20, THEME_TEXT_COLOR);
    char progress_bar[32];
    progress_bar[0] = '[';
    int pos = 1;
    for (int i = 0; i < active_sessions && i < instances_possible; i++) {
        progress_bar[pos++] = 'O';
    }
    
    for (int i = active_sessions; i < instances_possible; i++) {
        progress_bar[pos++] = '_';
    }
    
    progress_bar[pos++] = ']';
    progress_bar[pos++] = ' ';
    progress_bar[pos++] = digits[active_sessions % 10];
    progress_bar[pos++] = '/';
    progress_bar[pos++] = digits[instances_possible % 10];
    progress_bar[pos] = '\0';
    
    draw_text(202, 102, progress_bar, 20, THEME_TEXT_COLOR);
    
    int window_count = get_application_count();
    int window_limit = get_max_applications();
    
    draw_text(10, 126, "Open Windows: ", 20, THEME_TEXT_COLOR);
    
    char window_info[48];
    int w_pos = 0;
    window_info[w_pos++] = digits[(window_count / 10) % 10];
    window_info[w_pos++] = digits[window_count % 10];
    window_info[w_pos++] = ' ';
    window_info[w_pos++] = '/';
    window_info[w_pos++] = ' ';
    window_info[w_pos++] = digits[(window_limit / 10) % 10];
    window_info[w_pos++] = digits[window_limit % 10];
    window_info[w_pos++] = '\0';
    
    draw_text(160, 126, window_info, 20, THEME_TEXT_COLOR);
    
    if (window_count >= window_limit && instances_possible > active_sessions) {
        draw_text(10, 150, "Limited by window limit (32 max).", 20, 255, 0, 0);
    }
    else if (active_sessions >= instances_possible && instances_possible < MAX_RUNTIME_SESSIONS) {
        draw_text(10, 150, "Limited by memory.", 20, 255, 0, 0);
    }
    else if (active_sessions >= instances_possible && instances_possible >= MAX_RUNTIME_SESSIONS) {
        draw_text(10, 150, "System design.", 20, 0, 255, 0);
    }
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
