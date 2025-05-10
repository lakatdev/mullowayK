#include <userlib.h>
#include <serial.h>
#include <memory.h>
#include <interface.h>

int app_network_device_last_ping_success = 0;

void app_network_device_send_reset()
{
    send_command("SR", (void*)0, 0);
}

void app_network_device_send_ping()
{
    send_command("SP", (void*)0, 0);
    printf("Ping sent.\n");
    char response_command[3] = {0};
    char response_data[4] = {0};
    unsigned int size = 0;
    //receive_command_with_timeout(response_command, response_data, &size, 4);
    printf("Received command.\n");
    response_command[2] = '\0';
    if (strcmp(response_command, "SP") == 0) {
        if (strcmp(response_data, "pong") == 0) {
            app_network_device_last_ping_success = 1;
            printf("Ping success.\n");
            return;
        }
    }
    app_network_device_last_ping_success = 0;
    printf("Ping fail.\n");
}

void app_network_device_send_connect()
{

}

void app_network_device_draw()
{
    draw_screen(255, 255, 255);
    if (app_network_device_last_ping_success) {
        draw_text(10, 30, "Connected.", 24, 0, 200, 0);
    }
    else {
        draw_text(10, 30, "Connection failed or not requested.", 24, 200, 0, 0);
    }
}

void app_network_device_key(char key)
{

}

void app_network_device_mouse(int x, int y)
{

}

void app_network_device_init()
{
    add_app_menu_item((MenuItem) {
        .name = "Ping",
        .action = app_network_device_send_ping
    });

    add_app_menu_item((MenuItem) {
        .name = "Reset",
        .action = app_network_device_send_reset
    });

    add_app_menu_item((MenuItem) {
        .name = "Connect",
        .action = app_network_device_send_connect
    });
}
