#include <drivers/usb.h>
#include <system/port.h>
#include <system/interface.h>
#include <drivers/keyboard.h>
#include <system/desktop.h>
#include <tools/graphics.h>
#include <system/memory.h>
#include <drivers/serial.h>
#include <drivers/pci.h>

#define UHCI_CMD       0x00
#define UHCI_STS       0x02
#define UHCI_INTR      0x04
#define UHCI_FRNUM     0x06
#define UHCI_FRBASEADD 0x08
#define UHCI_SOFMOD    0x0C
#define UHCI_PORTSC1   0x10
#define UHCI_PORTSC2   0x12

#define UHCI_CMD_RS    0x0001
#define UHCI_CMD_HCRESET 0x0002
#define UHCI_CMD_GRESET  0x0004
#define UHCI_CMD_MAXP    0x0080

#define UHCI_STS_USBINT  0x0001
#define UHCI_STS_ERROR   0x0002
#define UHCI_STS_RD      0x0004
#define UHCI_STS_HSE     0x0008
#define UHCI_STS_HCPE    0x0010
#define UHCI_STS_HCH     0x0020

#define UHCI_PORT_CCS    0x0001
#define UHCI_PORT_CSC    0x0002
#define UHCI_PORT_PED    0x0004
#define UHCI_PORT_PEDC   0x0008
#define UHCI_PORT_LSDA   0x0100
#define UHCI_PORT_RESET  0x0200

#define EHCI_CAPLENGTH      0x00
#define EHCI_HCIVERSION     0x02
#define EHCI_HCSPARAMS      0x04
#define EHCI_HCCPARAMS      0x08

#define EHCI_USBCMD         0x00
#define EHCI_USBSTS         0x04
#define EHCI_USBINTR        0x08
#define EHCI_FRINDEX        0x0C
#define EHCI_CTRLDSSEGMENT  0x10
#define EHCI_PERIODICLIST   0x14
#define EHCI_ASYNCLISTADDR  0x18
#define EHCI_CONFIGFLAG     0x40
#define EHCI_PORTSC_BASE    0x44

#define EHCI_CMD_RUN        (1 << 0)
#define EHCI_CMD_HCRESET    (1 << 1)
#define EHCI_CMD_PERIODIC_EN (1 << 4)
#define EHCI_CMD_ASYNC_EN   (1 << 5)
#define EHCI_CMD_IAAD       (1 << 6)

#define EHCI_STS_USBINT     (1 << 0)
#define EHCI_STS_USBERRINT  (1 << 1)
#define EHCI_STS_PCD        (1 << 2)
#define EHCI_STS_ASYNC_ADV  (1 << 5)
#define EHCI_STS_HCHALTED   (1 << 12)
#define EHCI_STS_RECLAMATION (1 << 13)
#define EHCI_STS_ASYNC_STAT (1 << 15)

#define EHCI_PORT_CCS       (1 << 0)
#define EHCI_PORT_CSC       (1 << 1)
#define EHCI_PORT_PED       (1 << 2)
#define EHCI_PORT_PEDC      (1 << 3)
#define EHCI_PORT_OCA       (1 << 4)
#define EHCI_PORT_OCC       (1 << 5)
#define EHCI_PORT_FPR       (1 << 6)
#define EHCI_PORT_SUSPEND   (1 << 7)
#define EHCI_PORT_RESET     (1 << 8)
#define EHCI_PORT_LINE_STAT (3 << 10)
#define EHCI_PORT_POWER     (1 << 12)
#define EHCI_PORT_OWNER     (1 << 13)

#define EHCI_QTD_ACTIVE     (1 << 7)
#define EHCI_QTD_HALTED     (1 << 6)
#define EHCI_QTD_BUFERR     (1 << 5)
#define EHCI_QTD_BABBLE     (1 << 4)
#define EHCI_QTD_XACTERR    (1 << 3)
#define EHCI_QTD_MISSED     (1 << 2)
#define EHCI_QTD_SPLIT      (1 << 1)
#define EHCI_QTD_PING       (1 << 0)

#define EHCI_QTD_PID_OUT    0
#define EHCI_QTD_PID_IN     1
#define EHCI_QTD_PID_SETUP  2

typedef struct {
    unsigned int next_qtd;
    unsigned int alt_qtd;
    unsigned int token;
    unsigned int buffer[5];
} __attribute__((packed, aligned(32))) ehci_qtd_t;

typedef struct {
    unsigned int horiz_link;
    unsigned int endpoint_char;
    unsigned int endpoint_caps;
    unsigned int current_qtd;
    unsigned int next_qtd;
    unsigned int alt_qtd;
    unsigned int token;
    unsigned int buffer[5];
} __attribute__((packed, aligned(32))) ehci_qh_t;

#define USB_REQ_GET_STATUS        0x00
#define USB_REQ_SET_ADDRESS       0x05
#define USB_REQ_GET_DESCRIPTOR    0x06
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_SET_PROTOCOL      0x0B

#define USB_DESC_DEVICE      0x01
#define USB_DESC_CONFIG      0x02
#define USB_DESC_INTERFACE   0x04
#define USB_DESC_ENDPOINT    0x05
#define USB_DESC_HID         0x21
#define USB_DESC_HID_REPORT  0x22

#define USB_CLASS_HID    0x03
#define USB_CLASS_CDC    0x02

#define HID_PROTOCOL_KEYBOARD 1
#define HID_PROTOCOL_MOUSE    2

#define VID_FTDI       0x0403
#define PID_FT232      0x6001
#define VID_SILABS     0x10C4
#define PID_CP2102     0xEA60
#define VID_PROLIFIC   0x067B
#define PID_PL2303     0x2303
#define VID_CH340      0x1A86
#define PID_CH340      0x7523

#define SERIAL_CHIP_UNKNOWN  0
#define SERIAL_CHIP_CDC      1
#define SERIAL_CHIP_FTDI     2
#define SERIAL_CHIP_CP2102   3
#define SERIAL_CHIP_PL2303   4
#define SERIAL_CHIP_CH340    5

typedef struct {
    unsigned int link;
    unsigned int status;
    unsigned int token;
    unsigned int buffer;
    unsigned int reserved[4];
} __attribute__((packed, aligned(16))) uhci_td_t;

typedef struct {
    unsigned int head_link;
    unsigned int element;
    unsigned int reserved[2];
} __attribute__((packed, aligned(16))) uhci_qh_t;

// device info
typedef struct {
    unsigned char address;
    unsigned char device_type;
    unsigned char endpoint_in;
    unsigned char endpoint_out;
    unsigned char interface;
    unsigned char max_packet;
    unsigned char present;
    unsigned char configured;
} usb_device_t;

static unsigned short uhci_base = 0;
static int uhci_found = 0;

static unsigned int ehci_base = 0;
static unsigned int ehci_op_base = 0;
static int ehci_found = 0;
static int ehci_port_count = 0;

static ehci_qh_t ehci_async_qh __attribute__((aligned(32)));
static ehci_qh_t ehci_qh_pool[8] __attribute__((aligned(32)));
static int ehci_qh_next = 0;
static ehci_qtd_t ehci_qtd_pool[32] __attribute__((aligned(32)));
static int ehci_qtd_next = 0;

static inline unsigned int ehci_read32(unsigned int offset)
{
    return *(volatile unsigned int*)(ehci_op_base + offset);
}

static inline void ehci_write32(unsigned int offset, unsigned int val)
{
    *(volatile unsigned int*)(ehci_op_base + offset) = val;
}

static inline unsigned int ehci_cap_read32(unsigned int offset)
{
    return *(volatile unsigned int*)(ehci_base + offset);
}

static inline unsigned char ehci_cap_read8(unsigned int offset)
{
    return *(volatile unsigned char*)(ehci_base + offset);
}

static unsigned int frame_list[1024] __attribute__((aligned(4096)));

static uhci_td_t td_pool[32] __attribute__((aligned(32)));
static uhci_qh_t qh_pool[8] __attribute__((aligned(16)));
static int td_next = 0;

static unsigned char setup_buffer[8] __attribute__((aligned(16)));
static unsigned char data_buffer[256] __attribute__((aligned(16)));

static usb_device_t usb_devices[4];
static int next_address = 1;

static unsigned char kbd_report[8];
static unsigned char kbd_prev_report[8];

static unsigned char mouse_report[4];
extern int abs_x, abs_y;
extern char mouse_left;

#define USB_SERIAL_BUFFER_SIZE 256
static unsigned char serial_rx_buffer[USB_SERIAL_BUFFER_SIZE];
static volatile int serial_rx_head = 0;
static volatile int serial_rx_tail = 0;
static unsigned char serial_tx_buffer[USB_SERIAL_BUFFER_SIZE];

static int serial_chip_type = SERIAL_CHIP_UNKNOWN;
static unsigned short serial_vid = 0;
static unsigned short serial_pid = 0;
static volatile int serial_tx_head = 0;
static volatile int serial_tx_tail = 0;

static int usb_kbd_idx = -1;
static int usb_mouse_idx = -1;
static int usb_serial_idx = -1;

static void usb_delay(int count)
{
    for (volatile int i = 0; i < count * 1000; i++);
}

static int find_uhci_controller(void)
{   
    for (unsigned char bus = 0; bus < 8; bus++) {
        for (unsigned char dev = 0; dev < 32; dev++) {
            unsigned int venddev = pci_read32(bus, dev, 0, 0x00);
            if (venddev == 0xFFFFFFFF) continue;
            
            unsigned char header = pci_read32(bus, dev, 0, 0x0C) >> 16;
            int max_fn = (header & 0x80) ? 8 : 1;
            
            for (unsigned char fn = 0; fn < max_fn; fn++) {
                unsigned int vd = pci_read32(bus, dev, fn, 0x00);
                if (vd == 0xFFFFFFFF) continue;
                
                unsigned int classreg = pci_read32(bus, dev, fn, 0x08);
                unsigned char base_class = (classreg >> 24) & 0xFF;
                unsigned char sub_class = (classreg >> 16) & 0xFF;
                unsigned char prog_if = (classreg >> 8) & 0xFF;
                
                if (base_class == 0x0C && sub_class == 0x03) {
                    if (prog_if == 0x00) {
                        unsigned int bar4 = pci_read32(bus, dev, fn, 0x20);
                        if (bar4 & 1) {
                            uhci_base = bar4 & 0xFFE0;
                            
                            unsigned short cmd = pci_read16(bus, dev, fn, 0x04);
                            cmd |= 0x05;
                            pci_write16(bus, dev, fn, 0x04, cmd);
                            
                            printf("UHCI controller found\n");
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

static int find_ehci_controller(void)
{
    for (unsigned char bus = 0; bus < 8; bus++) {
        for (unsigned char dev = 0; dev < 32; dev++) {
            unsigned int venddev = pci_read32(bus, dev, 0, 0x00);
            if (venddev == 0xFFFFFFFF) continue;
            
            unsigned char header = pci_read32(bus, dev, 0, 0x0C) >> 16;
            int max_fn = (header & 0x80) ? 8 : 1;
            
            for (unsigned char fn = 0; fn < max_fn; fn++) {
                unsigned int vd = pci_read32(bus, dev, fn, 0x00);
                if (vd == 0xFFFFFFFF) continue;
                
                unsigned int classreg = pci_read32(bus, dev, fn, 0x08);
                unsigned char base_class = (classreg >> 24) & 0xFF;
                unsigned char sub_class = (classreg >> 16) & 0xFF;
                unsigned char prog_if = (classreg >> 8) & 0xFF;
                
                if (base_class == 0x0C && sub_class == 0x03 && prog_if == 0x20) {
                    unsigned int bar0 = pci_read32(bus, dev, fn, 0x10);
                    if ((bar0 & 1) == 0) {
                        ehci_base = bar0 & 0xFFFFFF00;

                        unsigned short cmd = pci_read16(bus, dev, fn, 0x04);
                        cmd |= 0x06;
                        pci_write16(bus, dev, fn, 0x04, cmd);
                        
                        unsigned char caplength = ehci_cap_read8(EHCI_CAPLENGTH);
                        ehci_op_base = ehci_base + caplength;
                        
                        unsigned int hcsparams = ehci_cap_read32(EHCI_HCSPARAMS);
                        ehci_port_count = hcsparams & 0x0F;
                        
                        printf("EHCI controller found, ");
                        print_hex(ehci_port_count);
                        printf(" ports\n");
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

static void ehci_reset(void)
{
    unsigned int cmd = ehci_read32(EHCI_USBCMD);
    cmd &= ~EHCI_CMD_RUN;
    ehci_write32(EHCI_USBCMD, cmd);
    
    int timeout = 1000;
    while (!(ehci_read32(EHCI_USBSTS) & EHCI_STS_HCHALTED) && timeout-- > 0) {
        usb_delay(1);
    }
    
    ehci_write32(EHCI_USBCMD, EHCI_CMD_HCRESET);
    timeout = 1000;
    while ((ehci_read32(EHCI_USBCMD) & EHCI_CMD_HCRESET) && timeout-- > 0) {
        usb_delay(1);
    }
    
    ehci_write32(EHCI_USBSTS, 0x3F);
    ehci_write32(EHCI_USBINTR, 0);
    ehci_write32(EHCI_CTRLDSSEGMENT, 0);
}

static void ehci_init_async(void)
{
    memset(&ehci_async_qh, 0, sizeof(ehci_async_qh));
    ehci_async_qh.horiz_link = ((unsigned int)&ehci_async_qh) | 0x02;
    ehci_async_qh.endpoint_char = (1 << 15);
    ehci_async_qh.next_qtd = 0x01;
    ehci_async_qh.alt_qtd = 0x01;
    
    ehci_write32(EHCI_ASYNCLISTADDR, (unsigned int)&ehci_async_qh);
}

static void ehci_start(void)
{
    ehci_write32(EHCI_CONFIGFLAG, 1);
    usb_delay(10);
    
    unsigned int cmd = ehci_read32(EHCI_USBCMD);
    cmd |= EHCI_CMD_RUN | EHCI_CMD_ASYNC_EN;
    cmd &= ~(0x3 << 2);
    ehci_write32(EHCI_USBCMD, cmd);
    
    usb_delay(50);
    
    if (ehci_read32(EHCI_USBSTS) & EHCI_STS_HCHALTED) {
        printf("EHCI: Controller failed to start\n");
    }
}

static ehci_qtd_t* ehci_alloc_qtd(void)
{
    if (ehci_qtd_next >= 32) ehci_qtd_next = 0;
    ehci_qtd_t* qtd = &ehci_qtd_pool[ehci_qtd_next++];
    memset(qtd, 0, sizeof(ehci_qtd_t));
    return qtd;
}

static ehci_qh_t* ehci_alloc_qh(void)
{
    if (ehci_qh_next >= 8) ehci_qh_next = 0;
    ehci_qh_t* qh = &ehci_qh_pool[ehci_qh_next++];
    memset(qh, 0, sizeof(ehci_qh_t));
    return qh;
}

static int ehci_reset_port(int port)
{
    unsigned int portsc_addr = EHCI_PORTSC_BASE + (port * 4);
    unsigned int status;
    
    status = ehci_read32(portsc_addr);
    if (!(status & EHCI_PORT_CCS)) {
        return 0;
    }
    
    status |= EHCI_PORT_POWER;
    ehci_write32(portsc_addr, status);
    usb_delay(20);
    
    status = ehci_read32(portsc_addr);
    status |= EHCI_PORT_RESET;
    status &= ~EHCI_PORT_PED;
    ehci_write32(portsc_addr, status);
    usb_delay(50);
    
    status = ehci_read32(portsc_addr);
    status &= ~EHCI_PORT_RESET;
    ehci_write32(portsc_addr, status);
    usb_delay(10);
    
    for (int i = 0; i < 10; i++) {
        status = ehci_read32(portsc_addr);
        if (status & EHCI_PORT_PED) {
            printf("EHCI: Port ");
            print_hex(port);
            printf(" enabled\n");
            return 1;
        }
        usb_delay(10);
    }
    
    status = ehci_read32(portsc_addr);
    if ((status & EHCI_PORT_LINE_STAT) == (1 << 10)) {
        printf("EHCI: Low-speed device on port, needs companion\n");
    }
    
    return 0;
}

static int ehci_control_transfer(unsigned char addr, unsigned char* setup, unsigned char* data, int data_len, int is_in)
{
    ehci_qtd_t* qtd_setup;
    ehci_qtd_t* qtd_data = 0;
    ehci_qtd_t* qtd_status;
    ehci_qh_t* transfer_qh;
    int timeout;
    
    qtd_setup = ehci_alloc_qtd();
    if (data_len > 0) {
        qtd_data = ehci_alloc_qtd();
    }
    qtd_status = ehci_alloc_qtd();
    transfer_qh = ehci_alloc_qh();
    
    memcpy(setup_buffer, setup, 8);
    qtd_setup->buffer[0] = (unsigned int)setup_buffer;
    qtd_setup->token = EHCI_QTD_ACTIVE | (EHCI_QTD_PID_SETUP << 8) | (3 << 10) | (8 << 16);
    
    if (data_len > 0) {
        qtd_setup->next_qtd = (unsigned int)qtd_data;
        qtd_setup->alt_qtd = 0x01;
        
        if (!is_in) {
            memcpy(data_buffer, data, data_len);
        }
        qtd_data->buffer[0] = (unsigned int)data_buffer;
        qtd_data->token = EHCI_QTD_ACTIVE | ((is_in ? EHCI_QTD_PID_IN : EHCI_QTD_PID_OUT) << 8) | (3 << 10) | (data_len << 16) | (1 << 31);  // Data toggle = 1
        qtd_data->next_qtd = (unsigned int)qtd_status;
        qtd_data->alt_qtd = 0x01;
    } else {
        qtd_setup->next_qtd = (unsigned int)qtd_status;
        qtd_setup->alt_qtd = 0x01;
    }
    
    qtd_status->buffer[0] = 0;
    qtd_status->token = EHCI_QTD_ACTIVE | ((is_in ? EHCI_QTD_PID_OUT : EHCI_QTD_PID_IN) << 8) | (3 << 10) | (1 << 15) | (1 << 31);
    qtd_status->next_qtd = 0x01;
    qtd_status->alt_qtd = 0x01;
    
    transfer_qh->horiz_link = ((unsigned int)&ehci_async_qh) | 0x02;
    transfer_qh->endpoint_char = (64 << 16) | (1 << 14) | (2 << 12) | (0 << 8) | addr;
    transfer_qh->endpoint_caps = (1 << 30);
    transfer_qh->current_qtd = 0;
    transfer_qh->next_qtd = (unsigned int)qtd_setup;
    transfer_qh->alt_qtd = 0x01;
    transfer_qh->token = 0;
    
    ehci_async_qh.horiz_link = ((unsigned int)transfer_qh) | 0x02;
    
    timeout = 5000;
    while (timeout-- > 0) {
        if (!(qtd_status->token & EHCI_QTD_ACTIVE)) {
            ehci_async_qh.horiz_link = ((unsigned int)&ehci_async_qh) | 0x02;
            
            if (qtd_status->token & (EHCI_QTD_HALTED | EHCI_QTD_BUFERR | EHCI_QTD_BABBLE | EHCI_QTD_XACTERR)) {
                return -1;
            }
            
            if (is_in && data_len > 0) {
                memcpy(data, data_buffer, data_len);
            }
            return 0;
        }
        usb_delay(1);
    }
    
    ehci_async_qh.horiz_link = ((unsigned int)&ehci_async_qh) | 0x02;
    return -1;
}

static int ehci_interrupt_transfer(unsigned char addr, unsigned char ep, unsigned char* data, int len)
{
    ehci_qtd_t* qtd;
    ehci_qh_t* transfer_qh;
    static unsigned char ehci_toggle[4] = {0, 0, 0, 0};
    int dev_idx = addr - 1;
    int timeout;
    
    if (dev_idx < 0 || dev_idx >= 4) return -1;
    
    qtd = ehci_alloc_qtd();
    transfer_qh = ehci_alloc_qh();
    
    qtd->buffer[0] = (unsigned int)data_buffer;
    qtd->token = EHCI_QTD_ACTIVE | (EHCI_QTD_PID_IN << 8) | (3 << 10) | (len << 16) | (ehci_toggle[dev_idx] << 31);
    qtd->next_qtd = 0x01;
    qtd->alt_qtd = 0x01;
    
    transfer_qh->horiz_link = ((unsigned int)&ehci_async_qh) | 0x02;
    transfer_qh->endpoint_char = (len << 16) | (0 << 14) | (2 << 12) | ((ep & 0x0F) << 8) | addr;
    transfer_qh->endpoint_caps = (1 << 30);
    transfer_qh->next_qtd = (unsigned int)qtd;
    transfer_qh->alt_qtd = 0x01;
    transfer_qh->token = ehci_toggle[dev_idx] << 31;
    
    ehci_async_qh.horiz_link = ((unsigned int)transfer_qh) | 0x02;
    
    timeout = 100;
    while (timeout-- > 0) {
        if (!(qtd->token & EHCI_QTD_ACTIVE)) {
            ehci_async_qh.horiz_link = ((unsigned int)&ehci_async_qh) | 0x02;
            
            if (qtd->token & EHCI_QTD_HALTED) {
                return 0;
            }
            
            ehci_toggle[dev_idx] ^= 1;
            int actual = len - ((qtd->token >> 16) & 0x7FFF);
            if (actual > 0 && actual <= len) {
                memcpy(data, data_buffer, actual);
            }
            return actual > 0 ? actual : 0;
        }
        usb_delay(1);
    }
    
    ehci_async_qh.horiz_link = ((unsigned int)&ehci_async_qh) | 0x02;
    return -1;
}

static void uhci_reset(void)
{
    outw(uhci_base + UHCI_CMD, UHCI_CMD_GRESET);
    usb_delay(50);
    outw(uhci_base + UHCI_CMD, 0);
    usb_delay(10);
    
    outw(uhci_base + UHCI_CMD, UHCI_CMD_HCRESET);
    while (inw(uhci_base + UHCI_CMD) & UHCI_CMD_HCRESET);
    
    outw(uhci_base + UHCI_STS, 0xFFFF);
    outw(uhci_base + UHCI_INTR, 0);
}

static void uhci_init_framelist(void)
{
    for (int i = 0; i < 1024; i++) {
        frame_list[i] = 0x00000001;
    }
    
    outl(uhci_base + UHCI_FRBASEADD, (unsigned int)frame_list);
    outw(uhci_base + UHCI_FRNUM, 0);
    outb(uhci_base + UHCI_SOFMOD, 64);
}

static void uhci_start(void)
{
    outw(uhci_base + UHCI_CMD, UHCI_CMD_RS | UHCI_CMD_MAXP);
    usb_delay(50);
    
    unsigned short status = inw(uhci_base + UHCI_STS);
    
    if (status & UHCI_STS_HCH) {
        printf("USB: Controller halted\n");
    }
}

static int uhci_reset_port(int port)
{
    unsigned short portaddr = uhci_base + (port == 0 ? UHCI_PORTSC1 : UHCI_PORTSC2);
    unsigned short status;
    
    status = inw(portaddr);
    
    if (!(status & UHCI_PORT_CCS)) {
        return 0;
    }
    
    outw(portaddr, UHCI_PORT_RESET);
    usb_delay(50);
    outw(portaddr, 0);
    usb_delay(10);
    
    for (int i = 0; i < 10; i++) {
        status = inw(portaddr);
        if (status & UHCI_PORT_CCS) {
            outw(portaddr, UHCI_PORT_PED | UHCI_PORT_CSC | UHCI_PORT_PEDC);
            usb_delay(10);
            status = inw(portaddr);
            if (status & UHCI_PORT_PED) {
                printf("USB: Port enabled.\n");
                return 1;
            }
        }
        usb_delay(10);
    }
    printf("USB: Port enable failed.\n");
    return 0;
}

static uhci_td_t* alloc_td(void)
{
    if (td_next >= 32) td_next = 0;
    uhci_td_t* td = &td_pool[td_next++];
    memset(td, 0, sizeof(uhci_td_t));
    return td;
}

static int uhci_control_transfer(unsigned char addr, unsigned char* setup, unsigned char* data, int data_len, int is_in)
{
    uhci_td_t* td_setup;
    uhci_td_t* td_data;
    uhci_td_t* td_status;
    int timeout;
    
    unsigned short port1_status = inw(uhci_base + UHCI_PORTSC1);
    unsigned short port2_status = inw(uhci_base + UHCI_PORTSC2);
    int is_lowspeed = ((port1_status & UHCI_PORT_LSDA) || (port2_status & UHCI_PORT_LSDA)) ? 1 : 0;
    unsigned int ls_bit = is_lowspeed ? (1 << 26) : 0;
    
    td_setup = alloc_td();
    td_setup->status = 0x00800000 | ls_bit | (3 << 27);
    td_setup->token = (7 << 21) | (0 << 19) | (0 << 15) | ((unsigned int)addr << 8) | 0x2D;
    memcpy(setup_buffer, setup, 8);
    td_setup->buffer = (unsigned int)setup_buffer;
    
    if (data_len > 0) {
        td_data = alloc_td();
        td_setup->link = (unsigned int)td_data | 0x04;
        
        td_data->status = 0x00800000 | ls_bit | (3 << 27);
        td_data->token = ((data_len - 1) << 21) | (1 << 19) | (0 << 15) | ((unsigned int)addr << 8) | (is_in ? 0x69 : 0xE1);
        td_data->buffer = (unsigned int)data_buffer;
        
        if (!is_in && data_len > 0) {
            memcpy(data_buffer, data, data_len);
        }
        
        td_status = alloc_td();
        td_data->link = (unsigned int)td_status | 0x04;
        
        td_status->link = 0x00000001;
        td_status->status = 0x00800000 | ls_bit | (3 << 27);
        td_status->token = (0x7FF << 21) | (1 << 19) | (0 << 15) | ((unsigned int)addr << 8) | (is_in ? 0xE1 : 0x69);
        td_status->buffer = 0;
    }
    else {
        td_status = alloc_td();
        td_setup->link = (unsigned int)td_status | 0x04;
        
        td_status->link = 0x00000001;
        td_status->status = 0x00800000 | ls_bit | (3 << 27);
        td_status->token = (0x7FF << 21) | (1 << 19) | (0 << 15) | ((unsigned int)addr << 8) | 0x69;
        td_status->buffer = 0;
    }
    
    for (int i = 0; i < 1024; i++) {
        frame_list[i] = (unsigned int)td_setup; 
    }
    
    timeout = 10000;
    while (timeout-- > 0) {
        if (!(td_status->status & 0x00800000)) {
            for (int i = 0; i < 1024; i++) {
                frame_list[i] = 0x00000001;
            }
            if (td_status->status & 0x007E0000) { 
                return -1;
            }
            if (is_in && data_len > 0) {
                memcpy(data, data_buffer, data_len);
            }
            return 0;
        }
        usb_delay(1);
    }
    
    for (int i = 0; i < 1024; i++) {
        frame_list[i] = 0x00000001;
    }
    
    return -1; 
}

static int uhci_interrupt_transfer(unsigned char addr, unsigned char ep, unsigned char* data, int len)
{
    uhci_td_t* td;
    static unsigned char toggle[4] = {0, 0, 0, 0};
    int dev_idx = addr - 1;
    int timeout;
    
    if (dev_idx < 0 || dev_idx >= 4) return -1;
    
    td = alloc_td();
    td->link = 0x00000001;
    td->status = 0x00800000;
    td->token = ((len - 1) << 21) | (toggle[dev_idx] << 19) | ((unsigned int)(ep & 0x0F) << 15) | ((unsigned int)addr << 8) | 0x69;
    td->buffer = (unsigned int)data_buffer;
    
    for (int i = 0; i < 1024; i++) {
        frame_list[i] = (unsigned int)td;
    }
    
    timeout = 100;
    while (timeout-- > 0) {
        if (!(td->status & 0x00800000)) {
            for (int i = 0; i < 1024; i++) {
                frame_list[i] = 0x00000001;
            }
            if (td->status & 0x00760000) { // error not nak
                return -1;
            }
            if (td->status & 0x00080000) { // nak
                return 0;
            }
            toggle[dev_idx] ^= 1;
            int actual = (td->status & 0x7FF) + 1;
            if (actual > 0x7FF) actual = 0; 
            if (actual > len) actual = len;
            memcpy(data, data_buffer, actual);
            return actual;
        }
        usb_delay(1);
    }
    
    for (int i = 0; i < 1024; i++) {
        frame_list[i] = 0x00000001;
    }
    return -1;
}

static int usb_control_transfer(unsigned char addr, unsigned char* setup, unsigned char* data, int data_len, int is_in)
{
    if (uhci_found) {
        return uhci_control_transfer(addr, setup, data, data_len, is_in);
    }
    else if (ehci_found) {
        return ehci_control_transfer(addr, setup, data, data_len, is_in);
    }
    return -1;
}

static int usb_interrupt_transfer(unsigned char addr, unsigned char ep, unsigned char* data, int len)
{
    if (uhci_found) {
        return uhci_interrupt_transfer(addr, ep, data, len);
    }
    else if (ehci_found) {
        return ehci_interrupt_transfer(addr, ep, data, len);
    }
    return -1;
}

static int usb_set_address(unsigned char addr)
{
    unsigned char setup[8] = {
        0x00,            
        USB_REQ_SET_ADDRESS,
        addr, 0,            
        0, 0,               
        0, 0                
    };
    return usb_control_transfer(0, setup, 0, 0, 0);
}

static int usb_get_device_descriptor(unsigned char addr, unsigned char* buf)
{
    unsigned char setup[8] = {
        0x80,
        USB_REQ_GET_DESCRIPTOR,
        0, USB_DESC_DEVICE,
        0, 0,
        18, 0
    };
    return usb_control_transfer(addr, setup, buf, 18, 1);
}

static int usb_get_config_descriptor(unsigned char addr, unsigned char* buf, int len)
{
    unsigned char setup[8] = {
        0x80,
        USB_REQ_GET_DESCRIPTOR,
        0, USB_DESC_CONFIG,
        0, 0,
        len & 0xFF, (len >> 8) & 0xFF
    };
    return usb_control_transfer(addr, setup, buf, len, 1);
}

static int usb_set_configuration(unsigned char addr, unsigned char config)
{
    unsigned char setup[8] = {
        0x00,
        USB_REQ_SET_CONFIGURATION,
        config, 0,
        0, 0,
        0, 0
    };
    return usb_control_transfer(addr, setup, 0, 0, 0);
}

static int usb_set_hid_protocol(unsigned char addr, unsigned char iface, unsigned char proto)
{
    unsigned char setup[8] = {
        0x21,                   
        USB_REQ_SET_PROTOCOL,
        proto, 0,               
        iface, 0,              
        0, 0
    };
    return usb_control_transfer(addr, setup, 0, 0, 0);
}

static void usb_serial_init_chip(unsigned char addr)
{
    unsigned char setup[8];
    unsigned char buf[8];
    
    switch (serial_chip_type) {
        case SERIAL_CHIP_CH340:
            setup[0] = 0x40; setup[1] = 0xA1; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(addr, setup, 0, 0, 0);
            usb_delay(10);
            
            setup[0] = 0x40; setup[1] = 0x9A; setup[2] = 0x33; setup[3] = 0x83;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(addr, setup, 0, 0, 0);
            
            setup[0] = 0x40; setup[1] = 0x9A; setup[2] = 0xC3; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(addr, setup, 0, 0, 0);
            break;
            
        case SERIAL_CHIP_CP2102:
            setup[0] = 0x41; setup[1] = 0x00; setup[2] = 0x01; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(addr, setup, 0, 0, 0);
            
            buf[0] = 0x00; buf[1] = 0xC2; buf[2] = 0x01; buf[3] = 0x00;
            setup[0] = 0x41; setup[1] = 0x1E; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x04; setup[7] = 0x00;
            usb_control_transfer(addr, setup, buf, 4, 0);
            
            setup[0] = 0x41; setup[1] = 0x03; setup[2] = 0x00; setup[3] = 0x08;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(addr, setup, 0, 0, 0);
            break;
            
        case SERIAL_CHIP_FTDI:
            setup[0] = 0x40; setup[1] = 0x00; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(addr, setup, 0, 0, 0);
            
            setup[0] = 0x40; setup[1] = 0x03; setup[2] = 0x1A; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(addr, setup, 0, 0, 0);
            
            setup[0] = 0x40; setup[1] = 0x04; setup[2] = 0x08; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(addr, setup, 0, 0, 0);
            
            setup[0] = 0x40; setup[1] = 0x02; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(addr, setup, 0, 0, 0);
            break;
            
        case SERIAL_CHIP_PL2303:
            setup[0] = 0xC0; setup[1] = 0x01; setup[2] = 0x84; setup[3] = 0x84;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x01; setup[7] = 0x00;
            usb_control_transfer(addr, setup, buf, 1, 1);
            
            buf[0] = 0x00; buf[1] = 0xC2; buf[2] = 0x01; buf[3] = 0x00;  // 115200
            buf[4] = 0x00;  
            buf[5] = 0x00; 
            buf[6] = 0x08;
            setup[0] = 0x21; setup[1] = 0x20; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x07; setup[7] = 0x00;
            usb_control_transfer(addr, setup, buf, 7, 0);
            break;
            
        case SERIAL_CHIP_CDC:
        default:
            buf[0] = 0x00; buf[1] = 0xC2; buf[2] = 0x01; buf[3] = 0x00;
            buf[4] = 0x00;
            buf[5] = 0x00;
            buf[6] = 0x08;
            setup[0] = 0x21; setup[1] = 0x20; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x07; setup[7] = 0x00;
            usb_control_transfer(addr, setup, buf, 7, 0);
            
            setup[0] = 0x21; setup[1] = 0x22; setup[2] = 0x03; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(addr, setup, 0, 0, 0);
            break;
    }
    printf("USB: Serial chip initialized.\n");
}

static void enumerate_device(int port)
{
    unsigned char desc[64];
    usb_device_t* dev;
    int dev_idx;
    
    usb_delay(50);
    
    if (usb_set_address(next_address) < 0) {
        printf("USB: Failed to set device address.\n");
        return;
    }
    usb_delay(10);
    
    dev_idx = next_address - 1;
    if (dev_idx >= 4) return;
    
    dev = &usb_devices[dev_idx];
    dev->address = next_address;
    dev->present = 1;
    next_address++;
    
    if (usb_get_device_descriptor(dev->address, desc) < 0) {
        printf("USB: Failed to get device descriptor.\n");
        dev->present = 0;
        return;
    }
    
    unsigned char dev_class = desc[4];
    unsigned short vid = desc[8] | (desc[9] << 8);
    unsigned short pid = desc[10] | (desc[11] << 8);
    
    int detected_serial = 0;
    if (vid == VID_FTDI && pid == PID_FT232) {
        serial_chip_type = SERIAL_CHIP_FTDI;
        detected_serial = 1;
        printf("USB: Serial device detected: FTDI FT232.\n");
    }
    else if (vid == VID_SILABS && pid == PID_CP2102) {
        serial_chip_type = SERIAL_CHIP_CP2102;
        detected_serial = 1;
        printf("USB: Serial device detected: CP2102.\n");
    }
    else if (vid == VID_PROLIFIC && pid == PID_PL2303) {
        serial_chip_type = SERIAL_CHIP_PL2303;
        detected_serial = 1;
        printf("USB: Serial device detected: PL2303.\n");
    }
    else if (vid == VID_CH340 && pid == PID_CH340) {
        serial_chip_type = SERIAL_CHIP_CH340;
        detected_serial = 1;
        printf("USB: Serial device detected: CH340.\n");
    }
    
    if (detected_serial) {
        dev->device_type = USB_DEV_SERIAL;
        usb_serial_idx = dev_idx;
        serial_vid = vid;
        serial_pid = pid;
    }
    
    if (usb_get_config_descriptor(dev->address, desc, 64) < 0) {
        printf("USB: Failed to get config descriptor.\n");
        return;
    }
    
    unsigned char config_val = desc[5];
    int offset = 0;
    int total_len = desc[2] | (desc[3] << 8);
    if (total_len > 64) total_len = 64;
    
    while (offset < total_len) {
        unsigned char len = desc[offset];
        unsigned char type = desc[offset + 1];
        
        if (len == 0) break;
        
        if (type == USB_DESC_INTERFACE) {
            unsigned char iface_num = desc[offset + 2];
            unsigned char iface_class = desc[offset + 5];
            unsigned char iface_subclass = desc[offset + 6];
            unsigned char iface_protocol = desc[offset + 7];
            
            if (iface_class == USB_CLASS_HID) {
                dev->interface = iface_num;
                
                if (iface_protocol == HID_PROTOCOL_KEYBOARD) {
                    dev->device_type = USB_DEV_KEYBOARD;
                    usb_kbd_idx = dev_idx;
                    printf("USB: Keyboard detected.\n");
                }
                else if (iface_protocol == HID_PROTOCOL_MOUSE) {
                    dev->device_type = USB_DEV_MOUSE;
                    usb_mouse_idx = dev_idx;
                    printf("USB: Mouse detected.\n");
                }
            }
            else if (iface_class == USB_CLASS_CDC || 
                       (iface_class == 0xFF && iface_subclass == 0xFF)) {
                dev->device_type = USB_DEV_SERIAL;
                usb_serial_idx = dev_idx;
                printf("USB: Serial device detected.\n");
            }
        }
        else if (type == USB_DESC_ENDPOINT) {
            unsigned char ep_addr = desc[offset + 2];
            unsigned char ep_attr = desc[offset + 3];
            unsigned char max_pkt = desc[offset + 4];
            
            if (ep_addr & 0x80) {
                dev->endpoint_in = ep_addr & 0x0F;
            }
            else {
                dev->endpoint_out = ep_addr & 0x0F;
            }
            dev->max_packet = max_pkt;
        }
        
        offset += len;
    }
    
    if (usb_set_configuration(dev->address, config_val) < 0) {
        printf("USB: Failed to set device configuration.\n");
        return;
    }
    
    if (dev->device_type == USB_DEV_KEYBOARD || dev->device_type == USB_DEV_MOUSE) {
        usb_set_hid_protocol(dev->address, dev->interface, 0);
    }
    
    if (dev->device_type == USB_DEV_SERIAL) {
        usb_serial_init_chip(dev->address);
    }
    
    dev->configured = 1;
}

static unsigned char hid_to_scancode[256] = {
    [0x04] = 0x1E,  // a
    [0x05] = 0x30,  // b
    [0x06] = 0x2E,  // c
    [0x07] = 0x20,  // d
    [0x08] = 0x12,  // e
    [0x09] = 0x21,  // f
    [0x0A] = 0x22,  // g
    [0x0B] = 0x23,  // h
    [0x0C] = 0x17,  // i
    [0x0D] = 0x24,  // j
    [0x0E] = 0x25,  // k
    [0x0F] = 0x26,  // l
    [0x10] = 0x32,  // m
    [0x11] = 0x31,  // n
    [0x12] = 0x18,  // o
    [0x13] = 0x19,  // p
    [0x14] = 0x10,  // q
    [0x15] = 0x13,  // r
    [0x16] = 0x1F,  // s
    [0x17] = 0x14,  // t
    [0x18] = 0x16,  // u
    [0x19] = 0x2F,  // v
    [0x1A] = 0x11,  // w
    [0x1B] = 0x2D,  // x
    [0x1C] = 0x15,  // y
    [0x1D] = 0x2C,  // z
    [0x1E] = 0x02,  // 1
    [0x1F] = 0x03,  // 2
    [0x20] = 0x04,  // 3
    [0x21] = 0x05,  // 4
    [0x22] = 0x06,  // 5
    [0x23] = 0x07,  // 6
    [0x24] = 0x08,  // 7
    [0x25] = 0x09,  // 8
    [0x26] = 0x0A,  // 9
    [0x27] = 0x0B,  // 0
    [0x28] = 0x1C,  // return
    [0x29] = 0x01,  // esc
    [0x2A] = 0x0E,  // backspace
    [0x2B] = 0x0F,  // tab
    [0x2C] = 0x39,  // space
    [0x2D] = 0x0C,  // -
    [0x2E] = 0x0D,  // =
    [0x2F] = 0x1A,  // [
    [0x30] = 0x1B,  // ]
    [0x31] = 0x2B,  // backslash
    [0x33] = 0x27,  // ;
    [0x34] = 0x28,  // '
    [0x35] = 0x29,  // `
    [0x36] = 0x33,  // ,
    [0x37] = 0x34,  // .
    [0x38] = 0x35,  // /
    [0x4F] = 0x4D,  // right
    [0x50] = 0x4B,  // left
    [0x51] = 0x50,  // down
    [0x52] = 0x48,  // up
};

static void process_keyboard(void)
{
    usb_device_t* dev = &usb_devices[usb_kbd_idx];
    int len;
    
    if (!dev->configured) return;
    
    len = usb_interrupt_transfer(dev->address, dev->endpoint_in, kbd_report, 8);
    if (len < 0) return;
    
    unsigned char modifiers = kbd_report[0];
    if (modifiers & 0x22) { 
        if (!(kbd_prev_report[0] & 0x22)) {
            handle_keyboard(0x2A);
        }
    }
    else {
        if (kbd_prev_report[0] & 0x22) {
            handle_keyboard(0xAA);
        }
    }
    
    for (int i = 2; i < 8; i++) {
        unsigned char key = kbd_report[i];
        if (key == 0) continue;
        
        int was_pressed = 0;
        for (int j = 2; j < 8; j++) {
            if (kbd_prev_report[j] == key) {
                was_pressed = 1;
                break;
            }
        }
        
        if (!was_pressed && key < 256) {
            unsigned char scancode = hid_to_scancode[key];
            if (scancode) {
                handle_keyboard(scancode);
            }
        }
    }
    
    memcpy(kbd_prev_report, kbd_report, 8);
}

static void process_mouse(void)
{
    usb_device_t* dev = &usb_devices[usb_mouse_idx];
    int len;
    
    if (!dev->configured) return;
    
    len = usb_interrupt_transfer(dev->address, dev->endpoint_in, mouse_report, 4);
    
    if (len < 3) return;  
    
    signed char dx = (signed char)mouse_report[1];
    signed char dy = (signed char)mouse_report[2];
    unsigned char buttons = mouse_report[0];
    
    abs_x += dx;
    abs_y += dy; 
    
    if (abs_x < 0) abs_x = 0;
    if (abs_x >= WIDTH) abs_x = WIDTH - 1;
    if (abs_y < 0) abs_y = 0;
    if (abs_y >= HEIGHT) abs_y = HEIGHT - 1;
    
    if (buttons & 0x01) {
        if (!mouse_left) {
            mouse_click(abs_x, abs_y);
        }
        mouse_left = 1;
    }
    else {
        mouse_left = 0;
    }
    
    invalidate();
}

static void process_serial(void)
{
    usb_device_t* dev = &usb_devices[usb_serial_idx];
    unsigned char buf[64];
    int len;
    
    if (!dev->configured) return;
    
    len = usb_interrupt_transfer(dev->address, dev->endpoint_in, buf, dev->max_packet ? dev->max_packet : 8);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            int next = (serial_rx_head + 1) % USB_SERIAL_BUFFER_SIZE;
            if (next != serial_rx_tail) {
                serial_rx_buffer[serial_rx_head] = buf[i];
                serial_rx_head = next;
            }
        }
    }
}

// external

void init_usb_legacy(void)
{
    printf("USB: Init started.\n");

    memset(usb_devices, 0, sizeof(usb_devices));
    memset(kbd_report, 0, sizeof(kbd_report));
    memset(kbd_prev_report, 0, sizeof(kbd_prev_report));
    
    if (find_uhci_controller()) {
        uhci_found = 1;
        uhci_reset();
        uhci_init_framelist();
        uhci_start();
        usb_delay(100);
        
        for (int port = 0; port < 2; port++) {
            if (uhci_reset_port(port)) {
                enumerate_device(port);
            }
        }
    }
    else if (find_ehci_controller()) {
        ehci_found = 1;
        ehci_reset();
        ehci_init_async();
        ehci_start();
        usb_delay(100);
        
        for (int port = 0; port < ehci_port_count; port++) {
            if (ehci_reset_port(port)) {
                enumerate_device(port);
            }
        }
    }
    else {
        printf("USB: No supported controller found.\n");
        return;
    }
    
    printf("USB: Init completed.\n");
}

void usb_poll_legacy(void)
{
    if (!uhci_found && !ehci_found) return;
    
    if (usb_kbd_idx >= 0) {
        process_keyboard();
    }
    
    if (usb_mouse_idx >= 0) {
        process_mouse();
    }
    
    if (usb_serial_idx >= 0) {
        process_serial();
    }
}


static void uhci_serial_write(usb_device_t* dev, const unsigned char* data, unsigned int size)
{
    uhci_td_t* td = alloc_td();
    unsigned char* buf = data_buffer;
    
    if (size > 64) size = 64;
    memcpy(buf, data, size);
    
    td->link = 0x00000001;
    td->status = 0x00800000;
    td->token = ((size - 1) << 21) | (0 << 19) | ((unsigned int)dev->endpoint_out << 15) | ((unsigned int)dev->address << 8) | 0xE1;
    td->buffer = (unsigned int)buf;
    
    for (int i = 0; i < 1024; i++) {
        frame_list[i] = (unsigned int)td;
    }
    
    int timeout = 1000;
    while ((td->status & 0x00800000) && timeout-- > 0) {
        usb_delay(1);
    }
    
    for (int i = 0; i < 1024; i++) {
        frame_list[i] = 0x00000001;
    }
}

static void ehci_serial_write(usb_device_t* dev, const unsigned char* data, unsigned int size)
{
    ehci_qtd_t* qtd;
    ehci_qh_t* transfer_qh;
    static unsigned char ehci_bulk_toggle[4] = {0, 0, 0, 0};
    int dev_idx = dev->address - 1;
    int timeout;
    
    if (dev_idx < 0 || dev_idx >= 4) return;
    
    if (size > 64) size = 64;
    memcpy(data_buffer, data, size);
    
    qtd = ehci_alloc_qtd();
    transfer_qh = ehci_alloc_qh();
    
    qtd->buffer[0] = (unsigned int)data_buffer;
    qtd->token = EHCI_QTD_ACTIVE | (EHCI_QTD_PID_OUT << 8) | (3 << 10) | (size << 16) | (ehci_bulk_toggle[dev_idx] << 31);
    qtd->next_qtd = 0x01;
    qtd->alt_qtd = 0x01;
    
    transfer_qh->horiz_link = ((unsigned int)&ehci_async_qh) | 0x02;
    transfer_qh->endpoint_char = (size << 16) | (0 << 14) | (2 << 12) | ((dev->endpoint_out & 0x0F) << 8) | dev->address;
    transfer_qh->endpoint_caps = (1 << 30);
    transfer_qh->next_qtd = (unsigned int)qtd;
    transfer_qh->alt_qtd = 0x01;
    transfer_qh->token = ehci_bulk_toggle[dev_idx] << 31;
    
    ehci_async_qh.horiz_link = ((unsigned int)transfer_qh) | 0x02;
    
    timeout = 1000;
    while (timeout-- > 0) {
        if (!(qtd->token & EHCI_QTD_ACTIVE)) {
            ehci_async_qh.horiz_link = ((unsigned int)&ehci_async_qh) | 0x02;
            
            if (qtd->token & EHCI_QTD_HALTED) {
                return;
            }
            
            ehci_bulk_toggle[dev_idx] ^= 1;
            return;
        }
        usb_delay(1);
    }
    
    ehci_async_qh.horiz_link = ((unsigned int)&ehci_async_qh) | 0x02;
}

void usb_serial_write_legacy(const unsigned char* data, unsigned int size)
{
    if (usb_serial_idx < 0) return;
    
    usb_device_t* dev = &usb_devices[usb_serial_idx];
    if (!dev->configured) return;
    
    if (uhci_found) {
        uhci_serial_write(dev, data, size);
    }
    else if (ehci_found) {
        ehci_serial_write(dev, data, size);
    }
}


int usb_serial_read_legacy(unsigned char* data, unsigned int size)
{
    int count = 0;
    while (count < (int)size && serial_rx_tail != serial_rx_head) {
        data[count++] = serial_rx_buffer[serial_rx_tail];
        serial_rx_tail = (serial_rx_tail + 1) % USB_SERIAL_BUFFER_SIZE;
    }
    return count;
}

int usb_serial_data_available_legacy(void)
{
    return serial_rx_head != serial_rx_tail;
}

unsigned char usb_serial_read_byte_legacy(void)
{
    if (serial_rx_tail == serial_rx_head) return 0;
    unsigned char c = serial_rx_buffer[serial_rx_tail];
    serial_rx_tail = (serial_rx_tail + 1) % USB_SERIAL_BUFFER_SIZE;
    return c;
}

void usb_serial_write_byte_legacy(unsigned char c)
{
    usb_serial_write_legacy(&c, 1);
}

void usb_serial_write_string_legacy(const char* str)
{
    int len = 0;
    while (str[len]) len++;
    usb_serial_write_legacy((const unsigned char*)str, len);
}

int usb_serial_set_baudrate_legacy(unsigned int baudrate)
{
    if (usb_serial_idx < 0) return -1;
    
    usb_device_t* dev = &usb_devices[usb_serial_idx];
    if (!dev->configured) return -1;
    
    unsigned char setup[8];
    unsigned char buf[8];
    
    switch (serial_chip_type) {
        case SERIAL_CHIP_CH340: {
            unsigned short div;
            switch (baudrate) {
                case 9600:   div = 0xB2; break;
                case 19200:  div = 0xD9; break;
                case 38400:  div = 0x6C; break;
                case 57600:  div = 0x48; break;
                case 115200: div = 0x33; break;
                default:     div = 0x33; break;
            }
            setup[0] = 0x40; setup[1] = 0x9A;
            setup[2] = div & 0xFF; setup[3] = 0x83;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(dev->address, setup, 0, 0, 0);
            break;
        }
        
        case SERIAL_CHIP_CP2102:
            buf[0] = baudrate & 0xFF;
            buf[1] = (baudrate >> 8) & 0xFF;
            buf[2] = (baudrate >> 16) & 0xFF;
            buf[3] = (baudrate >> 24) & 0xFF;
            setup[0] = 0x41; setup[1] = 0x1E; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x04; setup[7] = 0x00;
            usb_control_transfer(dev->address, setup, buf, 4, 0);
            break;
            
        case SERIAL_CHIP_FTDI: {
            unsigned short div;
            switch (baudrate) {
                case 9600:   div = 0x4138; break;
                case 19200:  div = 0x809C; break;
                case 38400:  div = 0xC04E; break;
                case 57600:  div = 0x0034; break;
                case 115200: div = 0x001A; break;
                default:     div = 0x001A; break;
            }
            setup[0] = 0x40; setup[1] = 0x03;
            setup[2] = div & 0xFF; setup[3] = (div >> 8) & 0xFF;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            usb_control_transfer(dev->address, setup, 0, 0, 0);
            break;
        }
            
        case SERIAL_CHIP_PL2303:
        case SERIAL_CHIP_CDC:
        default:
            buf[0] = baudrate & 0xFF;
            buf[1] = (baudrate >> 8) & 0xFF;
            buf[2] = (baudrate >> 16) & 0xFF;
            buf[3] = (baudrate >> 24) & 0xFF;
            buf[4] = 0x00;
            buf[5] = 0x00;
            buf[6] = 0x08;
            setup[0] = 0x21; setup[1] = 0x20; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x07; setup[7] = 0x00;
            usb_control_transfer(dev->address, setup, buf, 7, 0);
            break;
    }
    
    return 0;
}

int usb_keyboard_present_legacy(void)
{
    return usb_kbd_idx >= 0 && usb_devices[usb_kbd_idx].configured;
}

int usb_mouse_present_legacy(void)
{
    return usb_mouse_idx >= 0 && usb_devices[usb_mouse_idx].configured;
}

int usb_serial_present_legacy(void)
{
    return usb_serial_idx >= 0 && usb_devices[usb_serial_idx].configured;
}
