#ifndef SERIAL_H
#define SERIAL_H

void init_serial();
void com1_write(const unsigned char* data, unsigned int size);
void com1_read(unsigned char* data, unsigned int size);
int com1_data_available();
unsigned char com1_read_byte();

#endif
