#ifndef SERIAL_H
#define SERIAL_H

void init_serial();
void serial_write(const unsigned char* data, unsigned int size);
void serial_read(unsigned char* data, unsigned int size);
void send_command(const char* command, const unsigned char* data, unsigned int size);

#endif
