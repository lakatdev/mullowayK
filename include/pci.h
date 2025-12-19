#ifndef PCI_H
#define PCI_H

#define PCI_CFG_ADDR 0xCF8
#define PCI_CFG_DATA 0xCFC

void scan_bus(unsigned char bus);
int pci_get_ide_selected_ports(unsigned short* io, unsigned short* ctrl);
int pci_next_ide_controller(unsigned short* io, unsigned short* ctrl);

static inline unsigned int pci_read32(unsigned char bus, unsigned char dev, unsigned char fn, unsigned char off)
{
    unsigned int addr = (1u << 31) | ((unsigned)bus << 16) | ((unsigned)dev << 11) | ((unsigned)fn << 8) | (off & 0xFC);
    outl(PCI_CFG_ADDR, addr);
    return inl(PCI_CFG_DATA);
}

static void pci_write32(unsigned char bus, unsigned char dev, unsigned char fn, unsigned char off, unsigned int val)
{
    unsigned int addr = (1u << 31) | ((unsigned int)bus << 16) | ((unsigned int)dev << 11) | ((unsigned int)fn << 8) | (off & 0xFC);
    outl(PCI_CFG_ADDR, addr);
    outl(PCI_CFG_DATA, val);
}

static inline unsigned short pci_read16(unsigned char bus, unsigned char dev, unsigned char fn, unsigned char off)
{
    unsigned int v = pci_read32(bus, dev, fn, off & 0xFC);
    return (unsigned short)((v >> ((off & 2) * 8)) & 0xFFFF);
}

static inline void pci_write16(unsigned char bus, unsigned char dev, unsigned char fn, unsigned char off, unsigned short val)
{
    unsigned int addr = (1u << 31) | ((unsigned)bus << 16) | ((unsigned)dev << 11) | ((unsigned)fn << 8) | (off & 0xFC);
    outl(PCI_CFG_ADDR, addr);
    unsigned int old = inl(PCI_CFG_DATA);
    unsigned int shift = (off & 2) * 8;
    unsigned int mask = 0xFFFFu << shift;
    unsigned int nv = (old & ~mask) | ((unsigned int)val << shift);
    outl(PCI_CFG_DATA, nv);
}

static inline unsigned char pci_read8(unsigned char bus, unsigned char dev, unsigned char fn, unsigned char off)
{
    unsigned int v = pci_read32(bus, dev, fn, off & 0xFC);
    return (unsigned char)((v >> ((off & 3) * 8)) & 0xFF);
}

#endif
