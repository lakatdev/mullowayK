#ifndef PCI_H
#define PCI_H

void scan_bus(unsigned char bus);
int pci_get_ide_selected_ports(unsigned short* io, unsigned short* ctrl);
int pci_next_ide_controller(unsigned short* io, unsigned short* ctrl);

#endif
