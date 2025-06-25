#ifndef SERIAL_H
#define SERIAL_H

void init_serial();
void com1_write(const unsigned char* data, unsigned int size);
void com1_read(unsigned char* data, unsigned int size);

#endif
