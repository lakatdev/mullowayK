#include <userlib.h>
#include <serial.h>
#include <memory.h>
#include <interface.h>

void app_thermal_printer_draw()
{
    draw_screen(255, 255, 255);
    draw_text(10, 30, "Thermal Printer", 24, THEME_TEXT_COLOR);
}

void app_thermal_printer_key(char key)
{

}

void app_thermal_printer_mouse(int x, int y)
{

}

void app_thermal_printer_send_hello()
{
    unsigned char data[] = "hello";
    com1_write(data, sizeof(data));
}

void app_thermal_printer_init()
{
    add_app_menu_item((MenuItem) {
        .name = "Ping",
        .action = app_thermal_printer_send_hello
    });
}
