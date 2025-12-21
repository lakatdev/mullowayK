#ifndef USB_H
#define USB_H

#define USB_DEV_NONE     0
#define USB_DEV_KEYBOARD 1
#define USB_DEV_MOUSE    2
#define USB_DEV_SERIAL   3

void init_usb(void);
void usb_poll(void);

void usb_serial_write(const unsigned char* data, unsigned int size);
void usb_serial_write_byte(unsigned char c);
void usb_serial_write_string(const char* str);
int usb_serial_read(unsigned char* data, unsigned int size);
int usb_serial_data_available(void);
unsigned char usb_serial_read_byte(void);

int usb_serial_set_baudrate(unsigned int baudrate);

int usb_keyboard_present(void);
int usb_mouse_present(void);
int usb_serial_present(void);

void init_usb_legacy(void);
void usb_poll_legacy(void);
int usb_keyboard_present_legacy(void);
int usb_mouse_present_legacy(void);
int usb_serial_present_legacy(void);
void usb_serial_write_legacy(const unsigned char* data, unsigned int size);
int usb_serial_read_legacy(unsigned char* data, unsigned int size);
int usb_serial_data_available_legacy(void);
unsigned char usb_serial_read_byte_legacy(void);
void usb_serial_write_byte_legacy(unsigned char c);
void usb_serial_write_string_legacy(const char* str);
int usb_serial_set_baudrate_legacy(unsigned int baudrate);

#endif

