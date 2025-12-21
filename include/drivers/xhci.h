#ifndef XHCI_H
#define XHCI_H

#define XHCI_CAPLENGTH     0x00
#define XHCI_HCIVERSION    0x02
#define XHCI_HCSPARAMS1    0x04
#define XHCI_HCSPARAMS2    0x08
#define XHCI_HCSPARAMS3    0x0C
#define XHCI_HCCPARAMS1    0x10
#define XHCI_DBOFF         0x14
#define XHCI_RTSOFF        0x18

#define XHCI_USBCMD        0x00
#define XHCI_USBSTS        0x04
#define XHCI_PAGESIZE      0x08
#define XHCI_DNCTRL        0x14
#define XHCI_CRCR          0x18
#define XHCI_DCBAAP        0x30
#define XHCI_CONFIG        0x38

#define XHCI_CMD_RUN       (1 << 0)
#define XHCI_CMD_HCRST     (1 << 1)
#define XHCI_CMD_INTE      (1 << 2)
#define XHCI_CMD_HSEE      (1 << 3)

#define XHCI_STS_HCH       (1 << 0)
#define XHCI_STS_HSE       (1 << 2)
#define XHCI_STS_EINT      (1 << 3)
#define XHCI_STS_PCD       (1 << 4)
#define XHCI_STS_CNR       (1 << 11)

#define XHCI_PORT_CCS      (1 << 0)
#define XHCI_PORT_PED      (1 << 1)
#define XHCI_PORT_OCA      (1 << 3)
#define XHCI_PORT_PR       (1 << 4)
#define XHCI_PORT_PLS_MASK (0xF << 5)
#define XHCI_PORT_PP       (1 << 9)
#define XHCI_PORT_PIC_MASK (3 << 14)
#define XHCI_PORT_LWS      (1 << 16)
#define XHCI_PORT_CSC      (1 << 17)
#define XHCI_PORT_PEC      (1 << 18)
#define XHCI_PORT_WRC      (1 << 19)
#define XHCI_PORT_OCC      (1 << 20)
#define XHCI_PORT_PRC      (1 << 21)
#define XHCI_PORT_PLC      (1 << 22)
#define XHCI_PORT_CEC      (1 << 23)
#define XHCI_PORT_CAS      (1 << 24)
#define XHCI_PORT_WCE      (1 << 25)
#define XHCI_PORT_WDE      (1 << 26)
#define XHCI_PORT_WOE      (1 << 27)
#define XHCI_PORT_DR       (1 << 30)
#define XHCI_PORT_WPR      (1 << 31)

#define TRB_TYPE_NORMAL       1
#define TRB_TYPE_SETUP        2
#define TRB_TYPE_DATA         3
#define TRB_TYPE_STATUS       4
#define TRB_TYPE_LINK         6
#define TRB_TYPE_EVENT_DATA   7
#define TRB_TYPE_NOOP         8
#define TRB_TYPE_ENABLE_SLOT  9
#define TRB_TYPE_DISABLE_SLOT 10
#define TRB_TYPE_ADDRESS_DEV  11
#define TRB_TYPE_CONFIG_EP    12
#define TRB_TYPE_EVAL_CTX     13
#define TRB_TYPE_RESET_EP     14
#define TRB_TYPE_STOP_EP      15
#define TRB_TYPE_SET_TR_DEQ   16
#define TRB_TYPE_RESET_DEV    17
#define TRB_TYPE_NOOP_CMD     23

#define TRB_TYPE_TRANSFER     32
#define TRB_TYPE_CMD_COMPLETE 33
#define TRB_TYPE_PORT_STATUS  34

#define TRB_CC_SUCCESS        1
#define TRB_CC_SHORT_PACKET   13

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
    unsigned int param_lo;
    unsigned int param_hi;
    unsigned int status;
    unsigned int control;
} __attribute__((packed)) xhci_trb_t;

typedef struct {
    unsigned int info[8];
} __attribute__((packed, aligned(32))) xhci_slot_ctx_t;

typedef struct {
    unsigned int info[8];
} __attribute__((packed, aligned(32))) xhci_ep_ctx_t;

typedef struct {
    xhci_slot_ctx_t slot;
    xhci_ep_ctx_t ep[31];
} __attribute__((packed, aligned(64))) xhci_dev_ctx_t;

typedef struct {
    unsigned int control_ctx[8];
    xhci_slot_ctx_t slot;
    xhci_ep_ctx_t ep[31];
} __attribute__((packed, aligned(64))) xhci_input_ctx_t;

typedef struct {
    unsigned int ring_base_lo;
    unsigned int ring_base_hi;
    unsigned int ring_size;
    unsigned int reserved;
} __attribute__((packed)) xhci_erst_entry_t;

typedef struct {
    unsigned char address;
    unsigned char device_type;
    unsigned char endpoint_in;
    unsigned char endpoint_out;
    unsigned char interface;
    unsigned char max_packet;
    unsigned char present;
    unsigned char configured;
    unsigned char slot_id;
} usb_device_t_xhci;

#endif
