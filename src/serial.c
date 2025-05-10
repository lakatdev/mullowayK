#include <serial.h>
#include <port.h>
#include <interface.h>
#include <memory.h>
#include <interrupts.h>

#define COM1 0x3F8
#define TICKS_PER_MS 1
#define TIMEOUT 1000

void init_serial()
{
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
    printf("COM1 port initialized.\n");
}

void serial_write(const unsigned char* data, unsigned int size)
{
    for (unsigned int i = 0; i < size; ++i) {
        while (!(inb(COM1 + 5) & 0x20));
        outb(COM1, data[i]);
    }
}

void serial_read(unsigned char* data, unsigned int size)
{
    for (unsigned int i = 0; i < size; ++i) {
        while (!(inb(COM1 + 5) & 0x01));
        data[i] = inb(COM1);
    }
}

void send_command(const char* command, const unsigned char* data, unsigned int size)
{
    if (strlen(command) != 2) {
        printf("Error: Command must be exactly 2 characters.\n");
        return;
    }

    serial_write((const unsigned char*)command, 2);

    unsigned char size_bytes[4] = {
        (size >> 24) & 0xFF,
        (size >> 16) & 0xFF,
        (size >> 8) & 0xFF,
        size & 0xFF
    };
    serial_write(size_bytes, 4);

    if (size > 0) {
        serial_write(data, size);
    }
}

void receive_command(char* command, unsigned char* data, unsigned int* size)
{
    serial_read((unsigned char*)command, 2);
    command[2] = '\0';

    unsigned char size_bytes[4];
    serial_read(size_bytes, 4);
    *size = (size_bytes[0] << 24) | (size_bytes[1] << 16) | (size_bytes[2] << 8) | size_bytes[3];
    
    if (*size > 0) {
        serial_read(data, *size);
    }
}

/*int serial_read_with_timeout(unsigned char* buffer, unsigned int size, unsigned int timeout_ms)
{
    unsigned int bytes_read = 0;
    unsigned long long int start_time = system_uptime();
    unsigned long long int timeout_ticks = timeout_ms * TICKS_PER_MS;

    while (bytes_read < size) {
        if (inb(COM1 + 5) & 0x01) {
            buffer[bytes_read++] = inb(COM1);
        }

        if ((system_uptime() - start_time) > timeout_ticks) {
            return -1;
        }
    }
    return 0;
}*/

int serial_read_with_timeout(unsigned char* buffer, unsigned int size, unsigned int timeout_ms)
{
    unsigned int bytes_read = 0;
    unsigned long long int start_time = system_uptime();
    unsigned long long int timeout_ticks = timeout_ms * TICKS_PER_MS;

    while (bytes_read < size) {
        // Check if data is available
        if (inb(COM1 + 5) & 0x01) {
            buffer[bytes_read++] = inb(COM1);
        }

        // Check for timeout
        if ((system_uptime() - start_time) > timeout_ticks) {
            return -1; // Timeout occurred
        }

        // Yield control to allow interrupts to be processed
        __asm__ __volatile__("hlt");
    }
    return 0; // Successfully read all bytes
}

void receive_command_with_timeout(char* command, unsigned char* data, unsigned int* size, unsigned int max_data_size)
{
    if (serial_read_with_timeout((unsigned char*)command, 2, TIMEOUT) == -1) {
        printf("Error: Timeout while waiting for command.\n");
        return;
    }
    command[2] = '\0';

    unsigned char size_bytes[4];
    if (serial_read_with_timeout(size_bytes, 4, TIMEOUT) == -1) {
        printf("Error: Timeout while waiting for size field.\n");
        return;
    }
    *size = (size_bytes[0] << 24) | (size_bytes[1] << 16) | (size_bytes[2] << 8) | size_bytes[3];

    if (*size > 0 && *size <= max_data_size) {
        if (serial_read_with_timeout(data, *size, TIMEOUT) == -1) {
            printf("Error: Timeout while waiting for data.\n");
            return;
        }
    }
}
