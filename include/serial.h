#ifndef SERIAL_H
#define SERIAL_H

void init_serial();
void serial_write(const unsigned char* data, unsigned int size);
void serial_read(unsigned char* data, unsigned int size);
void send_command(const char* command, const unsigned char* data, unsigned int size);
void receive_command(char* command, unsigned char* data, unsigned int* size);
void receive_command_with_timeout(char* command, unsigned char* data, unsigned int* size, unsigned int max_data_size);
int serial_read_with_timeout(unsigned char* buffer, unsigned int size, unsigned int timeout_ms);

#endif
