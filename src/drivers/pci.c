#include <system/port.h>
#include <system/interface.h>
#include <drivers/pci.h>

typedef struct {
    unsigned short io_base;
    unsigned short ctrl_base;
    int found;
} ata_channel_t;

static ata_channel_t primary = {0}, secondary = {0};

typedef struct {
    unsigned short io;
    unsigned short ctrl;
} ide_port_pair_t;

#define MAX_IDE_CONTROLLERS 9
static ide_port_pair_t ide_controllers[MAX_IDE_CONTROLLERS];
static int ide_controller_count = 0;

static unsigned short g_sel_io = 0;
static unsigned short g_sel_ctrl = 0;

static void probe_ide(unsigned char bus, unsigned char dev, unsigned char fn)
{
    unsigned int classreg = pci_read32(bus, dev, fn, 0x08);
    unsigned char base_class = (classreg >> 24) & 0xFF;
    unsigned char sub_class  = (classreg >> 16) & 0xFF;
    unsigned char prog_if    = (classreg >> 8)  & 0xFF;

    if (base_class != 0x01) return;

    if (sub_class == 0x01) {
        unsigned short cmdreg = pci_read16(bus, dev, fn, 0x04);
        unsigned short newcmd = cmdreg | 0x0001 | 0x0004;
        if (newcmd != cmdreg) {
            pci_write16(bus, dev, fn, 0x04, newcmd);
        }

        unsigned int bar0 = pci_read32(bus, dev, fn, 0x10);
        unsigned int bar1 = pci_read32(bus, dev, fn, 0x14);
        unsigned int bar2 = pci_read32(bus, dev, fn, 0x18);
        unsigned int bar3 = pci_read32(bus, dev, fn, 0x1C);

        unsigned short p_cmd  = (bar0 & 1) ? (bar0 & ~3) : 0x1F0;
        unsigned short p_ctrl = (bar1 & 1) ? ((bar1 & ~3) + 2) : 0x3F6;
        unsigned short s_cmd  = (bar2 & 1) ? (bar2 & ~3) : 0x170;
        unsigned short s_ctrl = (bar3 & 1) ? ((bar3 & ~3) + 2) : 0x376;

        primary.io_base   = p_cmd;  primary.ctrl_base   = p_ctrl;  primary.found   = 1;
        secondary.io_base = s_cmd;  secondary.ctrl_base = s_ctrl;  secondary.found = 1;

        printf("IDE: progIF = "); print_hex(prog_if); printf("\n");
        printf("IDE primary: cmd = "); print_hex(p_cmd);
        printf(" ctrl = "); print_hex(p_ctrl); printf("\n");
        printf("IDE secondary: cmd = "); print_hex(s_cmd);
        printf(" ctrl = "); print_hex(s_ctrl); printf("\n");

        if (ide_controller_count < MAX_IDE_CONTROLLERS) {
            ide_controllers[ide_controller_count].io = p_cmd;
            ide_controllers[ide_controller_count].ctrl = p_ctrl;
            ide_controller_count++;
        }
        if (ide_controller_count < MAX_IDE_CONTROLLERS) {
            ide_controllers[ide_controller_count].io = s_cmd;
            ide_controllers[ide_controller_count].ctrl = s_ctrl;
            ide_controller_count++;
        }

        if (g_sel_io == 0) {
            unsigned char stp = inb(p_cmd + 7);
            if (stp != 0xFF && stp != 0x00) { 
                g_sel_io = p_cmd; 
                g_sel_ctrl = p_ctrl;
            }
        }

        if (g_sel_io == 0) {
            unsigned char sts = inb(s_cmd + 7);
            if (sts != 0xFF && sts != 0x00) { 
                g_sel_io = s_cmd; 
                g_sel_ctrl = s_ctrl;
            }
        }
    }
    else if (sub_class == 0x06) {
        printf("AHCI controller detected; PIO IDE driver cannot use it.\n");
    }
}

int pci_get_ide_selected_ports(unsigned short* io, unsigned short* ctrl)
{
    if (g_sel_io != 0) {
        if (io)   *io = g_sel_io;
        if (ctrl) *ctrl = g_sel_ctrl;
        return 1;
    }
    return 0;
}

int pci_next_ide_controller(unsigned short* io, unsigned short* ctrl)
{
    static int tried_controller = 0;
    if (tried_controller >= ide_controller_count) {
        return 0;
    }
    
    if (io) {
        *io = ide_controllers[tried_controller].io;
    }

    if (ctrl) {
        *ctrl = ide_controllers[tried_controller].ctrl;
    }

    tried_controller++;
    return 1;
}

void scan_bus(unsigned char bus)
{
    for (unsigned char dev = 0; dev < 32; dev++) {
        unsigned int venddev = pci_read32(bus, dev, 0, 0x00);
        if (venddev == 0xFFFFFFFF) continue;

        unsigned char header_type = pci_read8(bus, dev, 0, 0x0E);
        unsigned char multi_fn = header_type & 0x80;

        for (unsigned char fn = 0; fn < (multi_fn ? 8 : 1); fn++) {
            unsigned int vd = pci_read32(bus, dev, fn, 0x00);
            if (vd == 0xFFFFFFFF) continue;

            probe_ide(bus, dev, fn);

            unsigned char hdr = pci_read8(bus, dev, fn, 0x0E) & 0x7F;
            if (hdr == 0x01) {
                unsigned char sec_bus = pci_read8(bus, dev, fn, 0x19);
                if (sec_bus != 0 && sec_bus != bus) {
                    scan_bus(sec_bus);
                }
            }
        }
    }
}
