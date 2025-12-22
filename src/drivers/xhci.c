#include <drivers/usb.h>
#include <system/port.h>
#include <system/interface.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <system/desktop.h>
#include <tools/graphics.h>
#include <system/memory.h>
#include <drivers/serial.h>
#include <drivers/pci.h>
#include <drivers/xhci.h>

static unsigned int xhci_base = 0;
static unsigned int xhci_op_base = 0;
static unsigned int xhci_rt_base = 0;
static unsigned int xhci_db_base = 0;
static int xhci_found = 0;
static int using_legacy = 0;
static int xhci_max_slots = 0;
static int xhci_max_ports = 0;
static int xhci_context_size = 32;

static xhci_trb_t command_ring[64] __attribute__((aligned(64)));
static xhci_trb_t event_ring[64] __attribute__((aligned(64)));
static xhci_trb_t transfer_rings[4][64] __attribute__((aligned(64)));
static xhci_trb_t int_rings[4][64] __attribute__((aligned(64)));
static unsigned long long dcbaa[256] __attribute__((aligned(64)));
static xhci_dev_ctx_t device_contexts[4] __attribute__((aligned(64)));
static unsigned char raw_input_context[2112] __attribute__((aligned(64)));
static xhci_erst_entry_t erst[1] __attribute__((aligned(64)));

static int cmd_ring_enq = 0;
static int cmd_ring_cycle = 1;
static int evt_ring_deq = 0;
static int evt_ring_cycle = 1;
static int tr_ring_enq[4] = {0, 0, 0, 0};
static int tr_ring_cycle[4] = {1, 1, 1, 1};
static int int_ring_enq[4] = {0, 0, 0, 0};
static int int_ring_cycle[4] = {1, 1, 1, 1};
static int bulk_out_ring_enq[4] = {0, 0, 0, 0};
static int bulk_out_ring_cycle[4] = {1, 1, 1, 1};
static int int_pending[4] = {0, 0, 0, 0};

static unsigned char setup_buffer[8] __attribute__((aligned(16)));
static unsigned char control_data_buffer[256] __attribute__((aligned(64)));
static unsigned char int_data_buffers[4][64] __attribute__((aligned(64)));
static xhci_trb_t bulk_out_rings[4][64] __attribute__((aligned(64)));


static usb_device_t_xhci usb_devices[4];
static int next_address = 1;

static unsigned char kbd_report[8];
static unsigned char kbd_prev_report[8];
static unsigned char mouse_report[4];

#define USB_SERIAL_BUFFER_SIZE 256
static unsigned char serial_rx_buffer[USB_SERIAL_BUFFER_SIZE];
static volatile int serial_rx_head = 0;
static volatile int serial_rx_tail = 0;

static int serial_chip_type = SERIAL_CHIP_UNKNOWN;
static int usb_kbd_idx = -1;
static int usb_mouse_idx = -1;
static int usb_serial_idx = -1;

static unsigned int* get_input_control_ctx(void)
{
    return (unsigned int*)&raw_input_context[0];
}

static unsigned int* get_input_slot_ctx(void)
{
    return (unsigned int*)&raw_input_context[xhci_context_size];
}

static unsigned int* get_input_ep_ctx(int ep_idx)
{
    return (unsigned int*)&raw_input_context[(2 + ep_idx) * xhci_context_size];
}

static void xhci_delay(int count)
{
    for (volatile int i = 0; i < count * 1000; i++);
}

static unsigned int xhci_read32(unsigned int base, unsigned int offset)
{
    return *(volatile unsigned int*)(base + offset);
}

static void xhci_write32(unsigned int base, unsigned int offset, unsigned int val)
{
    *(volatile unsigned int*)(base + offset) = val;
}

static void xhci_write64(unsigned int base, unsigned int offset, unsigned long long val)
{
    *(volatile unsigned int*)(base + offset) = (unsigned int)val;
    *(volatile unsigned int*)(base + offset + 4) = (unsigned int)(val >> 32);
}

static int find_xhci_controller(void)
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
                
                if (base_class == 0x0C && sub_class == 0x03 && prog_if == 0x30) {
                    unsigned int bar0 = pci_read32(bus, dev, fn, 0x10);
                    if ((bar0 & 0x7) == 0x4) {
                        unsigned int bar1 = pci_read32(bus, dev, fn, 0x14);
                        xhci_base = (bar0 & 0xFFFFFFF0);
                    }
                    else {
                        xhci_base = bar0 & 0xFFFFFFF0;
                    }
                    
                    unsigned short cmd = pci_read16(bus, dev, fn, 0x04);
                    cmd |= 0x06;
                    pci_write16(bus, dev, fn, 0x04, cmd);
                    
                    unsigned char caplength = xhci_read32(xhci_base, XHCI_CAPLENGTH) & 0xFF;
                    xhci_op_base = xhci_base + caplength;
                    
                    unsigned int hcsparams1 = xhci_read32(xhci_base, XHCI_HCSPARAMS1);
                    xhci_max_slots = hcsparams1 & 0xFF;
                    xhci_max_ports = (hcsparams1 >> 24) & 0xFF;
                    
                    unsigned int hccparams1 = xhci_read32(xhci_base, XHCI_HCCPARAMS1);
                    if (hccparams1 & 0x04) xhci_context_size = 64;
                    
                    unsigned int dboff = xhci_read32(xhci_base, XHCI_DBOFF) & ~0x3;
                    unsigned int rtsoff = xhci_read32(xhci_base, XHCI_RTSOFF) & ~0x1F;
                    xhci_db_base = xhci_base + dboff;
                    xhci_rt_base = xhci_base + rtsoff;
                    
                    printf("xHCI controller found, ");
                    print_hex(xhci_max_ports);
                    printf(" ports\n");
                    return 1;
                }
            }
        }
    }
    return 0;
}

static void xhci_reset(void)
{
    unsigned int cmd = xhci_read32(xhci_op_base, XHCI_USBCMD);
    cmd &= ~XHCI_CMD_RUN;
    xhci_write32(xhci_op_base, XHCI_USBCMD, cmd);
    
    int timeout = 1000;
    while (!(xhci_read32(xhci_op_base, XHCI_USBSTS) & XHCI_STS_HCH) && timeout-- > 0) {
        xhci_delay(1);
    }
    
    xhci_write32(xhci_op_base, XHCI_USBCMD, XHCI_CMD_HCRST);
    timeout = 1000;
    while ((xhci_read32(xhci_op_base, XHCI_USBCMD) & XHCI_CMD_HCRST) && timeout-- > 0) {
        xhci_delay(1);
    }
    
    timeout = 1000;
    while ((xhci_read32(xhci_op_base, XHCI_USBSTS) & XHCI_STS_CNR) && timeout-- > 0) {
        xhci_delay(1);
    }
}

static void xhci_init_rings(void)
{
    memset(command_ring, 0, sizeof(command_ring));
    command_ring[63].control = (TRB_TYPE_LINK << 10) | (1 << 5);
    command_ring[63].param_lo = (unsigned int)command_ring;
    cmd_ring_enq = 0;
    cmd_ring_cycle = 1;
    
    memset(event_ring, 0, sizeof(event_ring));
    evt_ring_deq = 0;
    evt_ring_cycle = 1;
    
    erst[0].ring_base_lo = (unsigned int)event_ring;
    erst[0].ring_base_hi = 0;
    erst[0].ring_size = 64;
    erst[0].reserved = 0;
    
    for (int i = 0; i < 4; i++) {
        memset(transfer_rings[i], 0, sizeof(transfer_rings[i]));
        transfer_rings[i][63].control = (TRB_TYPE_LINK << 10) | (1 << 5);
        transfer_rings[i][63].param_lo = (unsigned int)transfer_rings[i];
        tr_ring_enq[i] = 0;
        tr_ring_cycle[i] = 1;
    }
    
    for (int i = 0; i < 4; i++) {
        memset(int_rings[i], 0, sizeof(int_rings[i]));
        int_rings[i][63].control = (TRB_TYPE_LINK << 10) | (1 << 5) | (1 << 1) | 1;
        int_rings[i][63].param_lo = (unsigned int)int_rings[i];
        int_ring_enq[i] = 0;
        int_ring_cycle[i] = 1;

        memset(bulk_out_rings[i], 0, sizeof(bulk_out_rings[i]));
        bulk_out_rings[i][63].control = (TRB_TYPE_LINK << 10) | (1 << 5) | (1 << 1) | 1;
        bulk_out_rings[i][63].param_lo = (unsigned int)bulk_out_rings[i];
        bulk_out_ring_enq[i] = 0;
        bulk_out_ring_cycle[i] = 1;
    }
    
    memset(dcbaa, 0, sizeof(dcbaa));
    memset(device_contexts, 0, sizeof(device_contexts));
}

static void xhci_start(void)
{
    int slots = xhci_max_slots > 4 ? 4 : xhci_max_slots;
    xhci_write32(xhci_op_base, XHCI_CONFIG, slots);
    
    xhci_write64(xhci_op_base, XHCI_DCBAAP, (unsigned int)dcbaa);
    
    xhci_write64(xhci_op_base, XHCI_CRCR, ((unsigned int)command_ring) | cmd_ring_cycle);
    
    unsigned int ir0_base = xhci_rt_base + 0x20;
    xhci_write32(ir0_base, 0x08, 1);
    xhci_write64(ir0_base, 0x18, (unsigned int)event_ring | (1 << 3));
    xhci_write64(ir0_base, 0x10, (unsigned int)&erst[0]);

    unsigned int cmd = xhci_read32(xhci_op_base, XHCI_USBCMD);
    cmd |= XHCI_CMD_RUN;
    xhci_write32(xhci_op_base, XHCI_USBCMD, cmd);
    
    xhci_delay(100);
    
    if (xhci_read32(xhci_op_base, XHCI_USBSTS) & XHCI_STS_HCH) {
        printf("xHCI: Controller failed to start\n");
    }
}

static void xhci_ring_doorbell(int slot, int endpoint)
{
    xhci_write32(xhci_db_base, slot * 4, endpoint);
}

static xhci_trb_t* xhci_get_next_event(void)
{
    xhci_trb_t* evt = &event_ring[evt_ring_deq];
    if ((evt->control & 1) == evt_ring_cycle) {
        evt_ring_deq++;
        if (evt_ring_deq >= 64) {
            evt_ring_deq = 0;
            evt_ring_cycle ^= 1;
        }
        unsigned int ir0_base = xhci_rt_base + 0x20;
        xhci_write64(ir0_base, 0x18, (unsigned int)&event_ring[evt_ring_deq] | (1 << 3));
        return evt;
    }
    return 0;
}

static xhci_trb_t* xhci_wait_command_event(int timeout_ms)
{
    int timeout = timeout_ms;
    while (timeout-- > 0) {
        xhci_trb_t* evt = xhci_get_next_event();
        if (evt) {
            int evt_type = (evt->control >> 10) & 0x3F;
            if (evt_type == TRB_TYPE_CMD_COMPLETE) {
                return evt;
            }
        }
        xhci_delay(1);
    }
    return 0;
}

static xhci_trb_t* xhci_wait_transfer_event(int timeout_ms)
{
    int timeout = timeout_ms;
    while (timeout-- > 0) {
        xhci_trb_t* evt = xhci_get_next_event();
        if (evt) {
            int evt_type = (evt->control >> 10) & 0x3F;
            if (evt_type == TRB_TYPE_TRANSFER) {
                return evt;
            }
        }
        xhci_delay(1);
    }
    return 0;
}

static int xhci_send_command(xhci_trb_t* cmd)
{
    command_ring[cmd_ring_enq].param_lo = cmd->param_lo;
    command_ring[cmd_ring_enq].param_hi = cmd->param_hi;
    command_ring[cmd_ring_enq].status = cmd->status;
    command_ring[cmd_ring_enq].control = cmd->control | cmd_ring_cycle;
    
    cmd_ring_enq++;
    if (cmd_ring_enq >= 63) {
        command_ring[63].control ^= 1;
        cmd_ring_cycle ^= 1;
        cmd_ring_enq = 0;
    }
    
    xhci_ring_doorbell(0, 0);
    xhci_ring_doorbell(0, 0);
    
    xhci_trb_t* evt = xhci_wait_command_event(1000);
    if (!evt) return -1;
    
    int cc = (evt->status >> 24) & 0xFF;
    if (cc != TRB_CC_SUCCESS) return -1;
    
    return (evt->control >> 24) & 0xFF;
}

static int xhci_enable_slot(void)
{
    xhci_trb_t cmd = {0};
    cmd.control = (TRB_TYPE_ENABLE_SLOT << 10);
    return xhci_send_command(&cmd);
}

static int xhci_address_device(int slot_id, int port)
{
    memset(raw_input_context, 0, sizeof(raw_input_context));
    
    unsigned int* ctrl_ctx = get_input_control_ctx();
    unsigned int* slot_ctx = get_input_slot_ctx();
    unsigned int* ep0_ctx = get_input_ep_ctx(0);
    
    ctrl_ctx[0] = 0;
    ctrl_ctx[1] = 0x03;
    
    unsigned int portsc_addr = xhci_op_base + 0x400 + (port * 0x10);
    unsigned int status = xhci_read32(portsc_addr, 0);
    unsigned int speed = (status >> 10) & 0xF;
    
    slot_ctx[0] = (1 << 27) | (speed << 20);
    slot_ctx[1] = ((port + 1) << 16);
    
    ep0_ctx[0] = 0;
    ep0_ctx[1] = (4 << 3) | (3 << 1);
    ep0_ctx[1] |= (64 << 16);
    ep0_ctx[2] = ((unsigned int)transfer_rings[slot_id - 1]) | tr_ring_cycle[slot_id - 1];
    ep0_ctx[4] = 8;
    
    memset(&device_contexts[slot_id - 1], 0, sizeof(xhci_dev_ctx_t));
    dcbaa[slot_id] = (unsigned int)&device_contexts[slot_id - 1];
    
    xhci_trb_t cmd = {0};
    cmd.param_lo = (unsigned int)raw_input_context;
    cmd.control = (TRB_TYPE_ADDRESS_DEV << 10) | (slot_id << 24);
    
    if (xhci_send_command(&cmd) < 0) return -1;
    
    return 0;
}


static int xhci_configure_hid_endpoint(int slot_id, int ep_num, int max_packet)
{
    int ring_idx = slot_id - 1;
    if (ring_idx < 0 || ring_idx >= 4) return -1;
    
    memset(raw_input_context, 0, sizeof(raw_input_context));
    
    unsigned int* ctrl_ctx = get_input_control_ctx();
    unsigned int* slot_ctx = get_input_slot_ctx();
    
    int ep_dci = (ep_num * 2) + 1;
    ctrl_ctx[0] = 0;
    ctrl_ctx[1] = (1 << ep_dci) | 1;
    
    slot_ctx[0] = (ep_dci << 27);
    
    int ep_idx = ep_num * 2;
    unsigned int* ep_ctx = get_input_ep_ctx(ep_idx);
    
    ep_ctx[0] = (3 << 16);
    ep_ctx[1] = (3 << 1) | (7 << 3) | (max_packet << 16);
    ep_ctx[2] = ((unsigned int)int_rings[ring_idx]) | int_ring_cycle[ring_idx];
    ep_ctx[4] = 8;
    
    xhci_trb_t cmd = {0};
    cmd.param_lo = (unsigned int)raw_input_context;
    cmd.control = (TRB_TYPE_CONFIG_EP << 10) | (slot_id << 24);
    
    if (xhci_send_command(&cmd) < 0) {
        return -1;
    }
    
    return 0;
}


static int xhci_configure_bulk_endpoint(int slot_id, int ep_num, int max_packet, int is_in)
{
    int ring_idx = slot_id - 1;
    if (ring_idx < 0 || ring_idx >= 4) return -1;
    
    memset(raw_input_context, 0, sizeof(raw_input_context));
    
    unsigned int* ctrl_ctx = get_input_control_ctx();
    unsigned int* slot_ctx = get_input_slot_ctx();
    
    int ep_dci = (ep_num * 2) + (is_in ? 1 : 0);
    ctrl_ctx[0] = 0;
    ctrl_ctx[1] = (1 << ep_dci) | 1;
    
    slot_ctx[0] = (ep_dci << 27);
    
    int ep_idx = ep_dci - 1; 
    unsigned int* ep_ctx = get_input_ep_ctx(ep_idx);
    
    int ep_type = is_in ? 6 : 2;
    
    ep_ctx[0] = (0 << 16); 
    ep_ctx[1] = (ep_type << 3) | (max_packet << 16) | 1;
    
    if (is_in) {
        ep_ctx[2] = ((unsigned int)int_rings[ring_idx]) | int_ring_cycle[ring_idx]; 
    } else {
        ep_ctx[2] = ((unsigned int)bulk_out_rings[ring_idx]) | bulk_out_ring_cycle[ring_idx];
    }

    ep_ctx[4] = 8;
    xhci_trb_t cmd = {0};
    cmd.param_lo = (unsigned int)raw_input_context;
    cmd.control = (TRB_TYPE_CONFIG_EP << 10) | (slot_id << 24);
    
    if (xhci_send_command(&cmd) < 0) {
        return -1;
    }
    
    return 0;
}

static void xhci_reset_transfer_ring(int slot_id)
{
    int ring_idx = slot_id - 1;
    if (ring_idx < 0 || ring_idx >= 4) return;
    
    tr_ring_enq[ring_idx] = 0;
    tr_ring_cycle[ring_idx] = 1;
    
    memset(&transfer_rings[ring_idx][0], 0, sizeof(xhci_trb_t) * 8);
    
    transfer_rings[ring_idx][63].control = (TRB_TYPE_LINK << 10) | (1 << 5);
    transfer_rings[ring_idx][63].param_lo = (unsigned int)transfer_rings[ring_idx];
    
    xhci_trb_t cmd = {0};
    cmd.param_lo = ((unsigned int)transfer_rings[ring_idx]) | 1;
    cmd.param_hi = 0;
    cmd.status = 0;
    cmd.control = (TRB_TYPE_SET_TR_DEQ << 10) | (slot_id << 24) | (1 << 16);
    
    xhci_send_command(&cmd);
}

static int xhci_control_transfer(int slot_id, unsigned char* setup, unsigned char* data, int len, int is_in)
{
    int ring_idx = slot_id - 1;
    if (ring_idx < 0 || ring_idx >= 4) return -1;
    
    xhci_trb_t* ring = transfer_rings[ring_idx];
    int enq = tr_ring_enq[ring_idx];
    int cycle = tr_ring_cycle[ring_idx];
    
    memcpy(setup_buffer, setup, 8);
    ring[enq].param_lo = ((unsigned int*)setup_buffer)[0];
    ring[enq].param_hi = ((unsigned int*)setup_buffer)[1];
    ring[enq].status = 8;
    ring[enq].control = (TRB_TYPE_SETUP << 10) | (1 << 6) | cycle;
    if (len > 0) ring[enq].control |= (is_in ? 3 : 2) << 16;
    enq++;
    
    if (len > 0) {
        if (!is_in) memcpy(control_data_buffer, data, len);
        ring[enq].param_lo = (unsigned int)control_data_buffer;
        ring[enq].param_hi = 0;
        ring[enq].status = len;
        ring[enq].control = (TRB_TYPE_DATA << 10) | cycle;
        if (is_in) ring[enq].control |= (1 << 16);
        enq++;
    }
    
    ring[enq].param_lo = 0;
    ring[enq].param_hi = 0;
    ring[enq].status = 0;
    ring[enq].control = (TRB_TYPE_STATUS << 10) | (1 << 5) | cycle;
    if (len == 0 || !is_in) ring[enq].control |= (1 << 16);
    enq++;
    
    if (enq >= 63) {
        ring[63].control ^= 1;
        tr_ring_cycle[ring_idx] ^= 1;
        enq = 0;
    }
    tr_ring_enq[ring_idx] = enq;
    
    xhci_ring_doorbell(slot_id, 1);
    
    xhci_trb_t* evt = xhci_wait_transfer_event(1000);
    if (!evt) {
        return -1;
    }
    
    int cc = (evt->status >> 24) & 0xFF;
    if (cc != TRB_CC_SUCCESS && cc != TRB_CC_SHORT_PACKET) return -1;
    
    if (is_in && len > 0) {
        int residue = evt->status & 0xFFFFFF;
        int actual = len - residue;
        if (actual > 0) memcpy(data, control_data_buffer, actual);
    }
    
    return 0;
}

static int xhci_bulk_transfer(int slot_id, int ep, unsigned char* data, int len, int is_in)
{
    int ring_idx = slot_id - 1;
    if (ring_idx < 0 || ring_idx >= 4) return -1;
    
    unsigned char* dev_buffer = int_data_buffers[ring_idx];    
    int expected_dci = (ep * 2) + (is_in ? 1 : 0);
    
    if (!is_in && len > 0) {
        memcpy(dev_buffer, data, len);
    }

    xhci_trb_t* ring;
    int enq;
    int cycle;
    
    if (is_in) {
        ring = int_rings[ring_idx];
        enq = int_ring_enq[ring_idx];
        cycle = int_ring_cycle[ring_idx];
    }
    else {
        ring = bulk_out_rings[ring_idx];
        enq = bulk_out_ring_enq[ring_idx];
        cycle = bulk_out_ring_cycle[ring_idx];
    }
    
    ring[enq].param_lo = (unsigned int)dev_buffer;
    ring[enq].param_hi = 0;
    ring[enq].status = len;
    ring[enq].control = (TRB_TYPE_NORMAL << 10) | (1 << 5) | cycle;
    
    enq++;
    if (enq >= 63) {
        ring[63].control = (TRB_TYPE_LINK << 10) | (1 << 5) | (1 << 1) | cycle;
        if (is_in) int_ring_cycle[ring_idx] ^= 1;
        else bulk_out_ring_cycle[ring_idx] ^= 1;
        enq = 0;
    }
    
    if (is_in) int_ring_enq[ring_idx] = enq;
    else bulk_out_ring_enq[ring_idx] = enq;
    
    xhci_ring_doorbell(slot_id, expected_dci);
    
    int timeout = 1000;
    while (timeout--) {
         xhci_trb_t* evt = &event_ring[evt_ring_deq];
        
        if ((evt->control & 1) != evt_ring_cycle) {
            xhci_delay(1);
            continue;
        }
        
        int evt_type = (evt->control >> 10) & 0x3F;
        int evt_slot = (evt->control >> 24) & 0xFF;
        int evt_dci = (evt->control >> 16) & 0x1F;
        
        unsigned int ir0_base = xhci_rt_base + 0x20;
        xhci_write64(ir0_base, 0x18, (unsigned int)evt | (1 << 3));
        
        evt_ring_deq++;
        if (evt_ring_deq >= 64) {
            evt_ring_deq = 0;
            evt_ring_cycle ^= 1;
        }
        
        if (evt_type == TRB_TYPE_TRANSFER && evt_slot == slot_id && evt_dci == expected_dci) {
            int cc = (evt->status >> 24) & 0xFF;
            if (cc == TRB_CC_SUCCESS || cc == TRB_CC_SHORT_PACKET) {
                if (is_in) {
                     int residue = evt->status & 0xFFFFFF;
                     int actual = len - residue;
                     if (actual > 0 && actual <= len) {
                         memcpy(data, dev_buffer, actual);
                         return actual;
                     }
                }
                return len;
            }
            return -1;
        }
    }
    
    return -2;
}

static int xhci_interrupt_transfer(int slot_id, int ep, unsigned char* data, int len)
{
    return xhci_bulk_transfer(slot_id, ep, data, len, 1);
}



static int xhci_reset_port(int port)
{
    unsigned int portsc = xhci_op_base + 0x400 + (port * 0x10);
    unsigned int status = xhci_read32(portsc, 0);
    
    if (!(status & XHCI_PORT_CCS)) {
        return 0;
    }
    
    status |= XHCI_PORT_PP;
    xhci_write32(portsc, 0, status);
    xhci_delay(20);
    
    status = xhci_read32(portsc, 0);
    status |= XHCI_PORT_PR;
    status &= ~(XHCI_PORT_PED);
    xhci_write32(portsc, 0, status);
    xhci_delay(50);
    
    int timeout = 100;
    while (timeout-- > 0) {
        status = xhci_read32(portsc, 0);
        if (status & XHCI_PORT_PRC) {
            xhci_write32(portsc, 0, status | XHCI_PORT_PRC);
            break;
        }
        xhci_delay(1);
    }
    
    status = xhci_read32(portsc, 0);
    if (status & XHCI_PORT_PED) {
        printf("xHCI: Port ");
        print_hex(port);
        printf(" enabled\n");
        return 1;
    }
    
    return 0;
}

static int usb_set_address_xhci(int slot_id, int port)
{
    return xhci_address_device(slot_id, port);
}

static int usb_get_device_descriptor(int slot_id, unsigned char* buf)
{
    unsigned char setup[8] = {0x80, USB_REQ_GET_DESCRIPTOR, 0, USB_DESC_DEVICE, 0, 0, 18, 0};
    return xhci_control_transfer(slot_id, setup, buf, 18, 1);
}

static int usb_get_config_descriptor(int slot_id, unsigned char* buf, int len)
{
    unsigned char setup[8] = {0x80, USB_REQ_GET_DESCRIPTOR, 0, USB_DESC_CONFIG, 0, 0, len & 0xFF, (len >> 8) & 0xFF};
    return xhci_control_transfer(slot_id, setup, buf, len, 1);
}

static int usb_set_configuration(int slot_id, int config)
{
    unsigned char setup[8] = {0x00, USB_REQ_SET_CONFIGURATION, config, 0, 0, 0, 0, 0};
    return xhci_control_transfer(slot_id, setup, 0, 0, 0);
}

static int usb_set_hid_protocol(int slot_id, int iface, int proto)
{
    unsigned char setup[8] = {0x21, USB_REQ_SET_PROTOCOL, proto, 0, iface, 0, 0, 0};
    return xhci_control_transfer(slot_id, setup, 0, 0, 0);
}


static void usb_serial_init_chip_xhci(int slot_id)
{
    unsigned char setup[8];
    unsigned char buf[8];
    
    switch (serial_chip_type) {
        case SERIAL_CHIP_CH340:
            setup[0] = 0x40; setup[1] = 0xA1; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            xhci_delay(10);
            
            setup[0] = 0x40; setup[1] = 0x9A; setup[2] = 0x33; setup[3] = 0x83;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            
            setup[0] = 0x40; setup[1] = 0x9A; setup[2] = 0xC3; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            break;
            
        case SERIAL_CHIP_CP2102:
            setup[0] = 0x41; setup[1] = 0x00; setup[2] = 0x01; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            
            buf[0] = 0x00; buf[1] = 0xC2; buf[2] = 0x01; buf[3] = 0x00;
            setup[0] = 0x41; setup[1] = 0x1E; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x04; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, buf, 4, 0);
            
            setup[0] = 0x41; setup[1] = 0x03; setup[2] = 0x00; setup[3] = 0x08;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            break;
            
        case SERIAL_CHIP_FTDI:
            setup[0] = 0x40; setup[1] = 0x00; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            
            setup[0] = 0x40; setup[1] = 0x03; setup[2] = 0x1A; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            
            setup[0] = 0x40; setup[1] = 0x04; setup[2] = 0x08; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            
            setup[0] = 0x40; setup[1] = 0x02; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            break;
            
        case SERIAL_CHIP_PL2303:
            setup[0] = 0xC0; setup[1] = 0x01; setup[2] = 0x84; setup[3] = 0x84;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x01; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, buf, 1, 1);
            
            buf[0] = 0x00; buf[1] = 0xC2; buf[2] = 0x01; buf[3] = 0x00;  // 115200
            buf[4] = 0x00;  
            buf[5] = 0x00; 
            buf[6] = 0x08;
            setup[0] = 0x21; setup[1] = 0x20; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x07; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, buf, 7, 0);
            break;
            
        case SERIAL_CHIP_CDC:
        default:
            buf[0] = 0x00; buf[1] = 0xC2; buf[2] = 0x01; buf[3] = 0x00;
            buf[4] = 0x00;
            buf[5] = 0x00;
            buf[6] = 0x08;
            setup[0] = 0x21; setup[1] = 0x20; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x07; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, buf, 7, 0);
            
            setup[0] = 0x21; setup[1] = 0x22; setup[2] = 0x03; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x00; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            break;
    }
    printf("xHCI: Serial chip initialized.\n");
}

static void enumerate_device_xhci(int port)
{
    int slot_id = xhci_enable_slot();
    if (slot_id <= 0) {
        printf("xHCI: Failed to enable slot\n");
        return;
    }
    
    if (usb_set_address_xhci(slot_id, port) < 0) {
        printf("xHCI: Failed to address device\n");
        return;
    }
    
    xhci_delay(50);
    
    unsigned char desc[64];
    if (usb_get_device_descriptor(slot_id, desc) < 0) {
        printf("xHCI: Failed to get device descriptor\n");
        return;
    }
    
    unsigned short vid = desc[8] | (desc[9] << 8);
    unsigned short pid = desc[10] | (desc[11] << 8);
    unsigned char dev_class = desc[4];
    
    int dev_idx = slot_id - 1;
    if (dev_idx >= 4) return;
    
    usb_devices[dev_idx].slot_id = slot_id;
    usb_devices[dev_idx].present = 1;
    
    int detected_serial = 0;
    if (vid == VID_FTDI && pid == PID_FT232) {
        serial_chip_type = SERIAL_CHIP_FTDI;
        detected_serial = 1;
    }
    else if (vid == VID_SILABS && pid == PID_CP2102) {
        serial_chip_type = SERIAL_CHIP_CP2102;
        detected_serial = 1;
    }
    else if (vid == VID_PROLIFIC && pid == PID_PL2303) {
        serial_chip_type = SERIAL_CHIP_PL2303;
        detected_serial = 1;
    }
    else if (vid == VID_CH340 && pid == PID_CH340) {
        serial_chip_type = SERIAL_CHIP_CH340;
        detected_serial = 1;
    }
    
    if (detected_serial) {
        usb_devices[dev_idx].device_type = USB_DEV_SERIAL;
        usb_serial_idx = dev_idx;
        printf("xHCI: Serial device detected.\n");
    }
    
    xhci_delay(50);
    
    if (usb_get_config_descriptor(slot_id, desc, 64) < 0) {
        return;
    }
    
    unsigned char config_val = desc[5];
    int offset = 0;
    int total_len = desc[2] | (desc[3] << 8);
    if (total_len > 64) total_len = 64;
    
    while (offset < total_len) {
        unsigned char dlen = desc[offset];
        unsigned char type = desc[offset + 1];
        if (dlen == 0) break;
        
        if (type == USB_DESC_INTERFACE) {
            unsigned char iface_class = desc[offset + 5];
            unsigned char iface_proto = desc[offset + 7];
            
            if (iface_class == USB_CLASS_HID) {
                usb_devices[dev_idx].interface = desc[offset + 2];
                if (iface_proto == HID_PROTOCOL_KEYBOARD) {
                    usb_devices[dev_idx].device_type = USB_DEV_KEYBOARD;
                    usb_kbd_idx = dev_idx;
                    printf("xHCI: Keyboard detected.\n");
                }
                else if (iface_proto == HID_PROTOCOL_MOUSE) {
                    usb_devices[dev_idx].device_type = USB_DEV_MOUSE;
                    usb_mouse_idx = dev_idx;
                    printf("xHCI: Mouse detected.\n");
                }
            }
            else if (iface_class == USB_CLASS_CDC) {
                usb_devices[dev_idx].device_type = USB_DEV_SERIAL;
                usb_serial_idx = dev_idx;
            }
        }
        else if (type == USB_DESC_ENDPOINT) {
            unsigned char ep_addr = desc[offset + 2];
            unsigned char max_pkt = desc[offset + 4];
            if (ep_addr & 0x80) {
                usb_devices[dev_idx].endpoint_in = ep_addr & 0x0F;
            }
            else {
                usb_devices[dev_idx].endpoint_out = ep_addr & 0x0F;
            }
            usb_devices[dev_idx].max_packet = max_pkt;
        }
        offset += dlen;
    }
    
    if (usb_set_configuration(slot_id, config_val) < 0) {
        printf("xHCI: Failed to set configuration.\n");
        return;
    }
    
    if (usb_devices[dev_idx].device_type == USB_DEV_KEYBOARD || 
        usb_devices[dev_idx].device_type == USB_DEV_MOUSE) {
        usb_set_hid_protocol(slot_id, usb_devices[dev_idx].interface, 0);

        int ep_in = usb_devices[dev_idx].endpoint_in;
        int max_pkt = usb_devices[dev_idx].max_packet ? usb_devices[dev_idx].max_packet : 8;
        if (xhci_configure_hid_endpoint(slot_id, ep_in, max_pkt) < 0) {
            printf("xHCI: Failed to configure HID endpoint.\n");
        }
    }
    else if (usb_devices[dev_idx].device_type == USB_DEV_SERIAL) {
        int ep_in = usb_devices[dev_idx].endpoint_in;
        int ep_out = usb_devices[dev_idx].endpoint_out;
        int max_pkt = usb_devices[dev_idx].max_packet ? usb_devices[dev_idx].max_packet : 64;
        
        if (ep_in) {
            xhci_configure_bulk_endpoint(slot_id, ep_in, max_pkt, 1);
        }
        if (ep_out) {
            xhci_configure_bulk_endpoint(slot_id, ep_out, max_pkt, 0);
        }
        usb_serial_init_chip_xhci(slot_id);
    }
    
    usb_devices[dev_idx].configured = 1;
}

static unsigned char hid_to_scancode[256] = {
    [0x04] = 0x1E, [0x05] = 0x30, [0x06] = 0x2E, [0x07] = 0x20,
    [0x08] = 0x12, [0x09] = 0x21, [0x0A] = 0x22, [0x0B] = 0x23,
    [0x0C] = 0x17, [0x0D] = 0x24, [0x0E] = 0x25, [0x0F] = 0x26,
    [0x10] = 0x32, [0x11] = 0x31, [0x12] = 0x18, [0x13] = 0x19,
    [0x14] = 0x10, [0x15] = 0x13, [0x16] = 0x1F, [0x17] = 0x14,
    [0x18] = 0x16, [0x19] = 0x2F, [0x1A] = 0x11, [0x1B] = 0x2D,
    [0x1C] = 0x15, [0x1D] = 0x2C, [0x1E] = 0x02, [0x1F] = 0x03,
    [0x20] = 0x04, [0x21] = 0x05, [0x22] = 0x06, [0x23] = 0x07,
    [0x24] = 0x08, [0x25] = 0x09, [0x26] = 0x0A, [0x27] = 0x0B,
    [0x28] = 0x1C, [0x29] = 0x01, [0x2A] = 0x0E, [0x2B] = 0x0F,
    [0x2C] = 0x39, [0x2D] = 0x0C, [0x2E] = 0x0D, [0x2F] = 0x1A,
    [0x30] = 0x1B, [0x31] = 0x2B, [0x33] = 0x27, [0x34] = 0x28,
    [0x35] = 0x29, [0x36] = 0x33, [0x37] = 0x34, [0x38] = 0x35,
    [0x4F] = 0x4D, [0x50] = 0x4B, [0x51] = 0x50, [0x52] = 0x48,
};

static void process_keyboard_xhci(void)
{
    usb_device_t_xhci* dev = &usb_devices[usb_kbd_idx];
    if (!dev->configured) return;
    
    int len = xhci_interrupt_transfer(dev->slot_id, dev->endpoint_in, kbd_report, 8);
    if (len < 0) return;
    
    unsigned char modifiers = kbd_report[0];
    if (modifiers & 0x22) {
        if (!(kbd_prev_report[0] & 0x22)) handle_keyboard(0x2A);
    }
    else {
        if (kbd_prev_report[0] & 0x22) handle_keyboard(0xAA);
    }
    
    for (int i = 2; i < 8; i++) {
        unsigned char key = kbd_report[i];
        if (key == 0) continue;
        
        int was_pressed = 0;
        for (int j = 2; j < 8; j++) {
            if (kbd_prev_report[j] == key) {
                was_pressed = 1; break;
            }
        }
        
        if (!was_pressed && key < 256) {
            unsigned char scancode = hid_to_scancode[key];
            if (scancode) handle_keyboard(scancode);
        }
    }
    
    memcpy(kbd_prev_report, kbd_report, 8);
}

static void process_mouse_xhci(void)
{
    usb_device_t_xhci* dev = &usb_devices[usb_mouse_idx];
    if (!dev->configured) {
        return;
    }
    
    int len = xhci_interrupt_transfer(dev->slot_id, dev->endpoint_in, mouse_report, 4);
    if (len < 3) {
        return;
    }
    
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

static void process_serial_xhci(void)
{
    usb_device_t_xhci* dev = &usb_devices[usb_serial_idx];
    if (!dev->configured) return;
    
    unsigned char buf[64];
    int len = xhci_interrupt_transfer(dev->slot_id, dev->endpoint_in, buf, dev->max_packet ? dev->max_packet : 8);
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

void init_usb(void)
{
    printf("USB: Init started.\n");
    
    memset(usb_devices, 0, sizeof(usb_devices));
    memset(kbd_report, 0, sizeof(kbd_report));
    memset(kbd_prev_report, 0, sizeof(kbd_prev_report));
    
    if (find_xhci_controller()) {
        xhci_found = 1;
        xhci_reset();
        xhci_init_rings();
        xhci_start();
        xhci_delay(100);
        
        for (int port = 0; port < xhci_max_ports && port < 8; port++) {
            if (xhci_reset_port(port)) {
                enumerate_device_xhci(port);
            }
        }
        printf("USB: Init completed (xHCI).\n");
    }
    else {
        printf("USB: No xHCI, trying legacy.\n");
        using_legacy = 1;
        init_usb_legacy();
    }
}

void usb_poll(void)
{
    if (using_legacy) {
        usb_poll_legacy();
        return;
    }
    
    if (!xhci_found) return;
    
    if (usb_kbd_idx >= 0) process_keyboard_xhci();
    if (usb_mouse_idx >= 0) process_mouse_xhci();
    if (usb_serial_idx >= 0) process_serial_xhci();
}

int usb_keyboard_present(void)
{
    if (using_legacy) return usb_keyboard_present_legacy();
    return usb_kbd_idx >= 0 && usb_devices[usb_kbd_idx].configured;
}

int usb_mouse_present(void)
{
    if (using_legacy) return usb_mouse_present_legacy();
    return usb_mouse_idx >= 0 && usb_devices[usb_mouse_idx].configured;
}

int usb_serial_present(void)
{
    if (using_legacy) return usb_serial_present_legacy();
    return usb_serial_idx >= 0 && usb_devices[usb_serial_idx].configured;
}


void usb_serial_write(const unsigned char* data, unsigned int size)
{
    if (using_legacy) { usb_serial_write_legacy(data, size); return; }
    
    usb_device_t_xhci* dev = &usb_devices[usb_serial_idx];
    if (!dev->configured) return;
    
    unsigned char buf[64];
    while (size > 0) {
        int chunk = size > 64 ? 64 : size;
        memcpy(buf, data, chunk);
        xhci_bulk_transfer(dev->slot_id, dev->endpoint_out, buf, chunk, 0); // 0 = OUT
        data += chunk;
        size -= chunk;
    }
}


int usb_serial_read(unsigned char* data, unsigned int size)
{
    if (using_legacy) return usb_serial_read_legacy(data, size);
    int count = 0;
    while (count < (int)size && serial_rx_tail != serial_rx_head) {
        data[count++] = serial_rx_buffer[serial_rx_tail];
        serial_rx_tail = (serial_rx_tail + 1) % USB_SERIAL_BUFFER_SIZE;
    }
    return count;
}

int usb_serial_data_available(void)
{
    if (using_legacy) return usb_serial_data_available_legacy();
    return serial_rx_head != serial_rx_tail;
}

unsigned char usb_serial_read_byte(void)
{
    if (using_legacy) return usb_serial_read_byte_legacy();
    if (serial_rx_tail == serial_rx_head) return 0;
    unsigned char c = serial_rx_buffer[serial_rx_tail];
    serial_rx_tail = (serial_rx_tail + 1) % USB_SERIAL_BUFFER_SIZE;
    return c;
}

void usb_serial_write_byte(unsigned char c)
{
    if (using_legacy) { usb_serial_write_byte_legacy(c); return; }
    usb_serial_write(&c, 1);
}

void usb_serial_write_string(const char* str)
{
    if (using_legacy) { usb_serial_write_string_legacy(str); return; }
    
    int len = 0;
    while (str[len]) len++;
    
    usb_serial_write((const unsigned char*)str, len);
}


int usb_serial_set_baudrate(unsigned int baudrate)
{
    if (using_legacy) return usb_serial_set_baudrate_legacy(baudrate);
    
    usb_device_t_xhci* dev = &usb_devices[usb_serial_idx];
    if (!dev->configured) return -1;
    
    unsigned char setup[8];
    unsigned char buf[8];
    int slot_id = dev->slot_id;
    
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
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
            break;
        }
        
        case SERIAL_CHIP_CP2102:
            buf[0] = baudrate & 0xFF;
            buf[1] = (baudrate >> 8) & 0xFF;
            buf[2] = (baudrate >> 16) & 0xFF;
            buf[3] = (baudrate >> 24) & 0xFF;
            setup[0] = 0x41; setup[1] = 0x1E; setup[2] = 0x00; setup[3] = 0x00;
            setup[4] = 0x00; setup[5] = 0x00; setup[6] = 0x04; setup[7] = 0x00;
            xhci_control_transfer(slot_id, setup, buf, 4, 0);
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
            xhci_control_transfer(slot_id, setup, 0, 0, 0);
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
            xhci_control_transfer(slot_id, setup, buf, 7, 0);
            break;
    }
    
    return 0;
}
