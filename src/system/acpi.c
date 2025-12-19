#include <system/acpi.h>
#include <system/port.h>
#include <system/memory.h>
#include <system/interface.h>
#include <system/interrupts.h>

unsigned int *smi_cmd;
unsigned char acpi_enable_cmd;
unsigned char acpi_disable_cmd;
unsigned int *pm1a_cnt;
unsigned int *pm1b_cnt;
unsigned short slp_typa;
unsigned short slp_typb;
unsigned short slp_en;
unsigned short sci_en;
unsigned char pm1_cnt_len;

typedef struct {
    unsigned char signature[8];
    unsigned char checksum;
    unsigned char oem_id[6];
    unsigned char revision;
    unsigned int *rsdt_address;
} __attribute__((packed)) RSDPtr;

typedef struct {
    unsigned char signature[4];
    unsigned int length;
    unsigned char unneded1[40 - 8];
    unsigned int *dsdt;
    unsigned char unneded2[48 - 44];
    unsigned int *smi_cmd;
    unsigned char acpi_enable;
    unsigned char acpi_disable;
    unsigned char unneded3[64 - 54];
    unsigned int *pm1a_cnt_blk;
    unsigned int *pm1b_cnt_blk;
    unsigned char unneded4[89 - 72];
    unsigned char pm1_cnt_len;
} __attribute__((packed)) FACP;

unsigned int *acpi_check_rsd_ptr(unsigned int *ptr)
{
    char *sig = "RSD PTR ";
    RSDPtr *rsdp = (RSDPtr *) ptr;
    unsigned char *bptr;
    unsigned char check = 0;
    int i;

    if (memcmp(sig, rsdp, 8) == 0) {
        bptr = (unsigned char *) ptr;
        for (i = 0; i < sizeof(RSDPtr); i++) {
            check += *bptr;
            bptr++;
        }

        if (check == 0) {
            return (unsigned int *) rsdp->rsdt_address;
        }
    }

    return 0;
}

unsigned int *acpi_get_rsd_ptr(void)
{
    unsigned int *addr;
    unsigned int *rsdp;

    for (addr = (unsigned int *) 0x000E0000; (int) addr < 0x00100000; addr += 0x10 / sizeof(addr)) {
        rsdp = acpi_check_rsd_ptr(addr);
        if (rsdp != 0) {
            return rsdp;
        }
    }

    int ebda = *((short *) 0x40E);
    ebda = ebda * 0x10 & 0x000FFFFF;

    for (addr = (unsigned int *) ebda; (int) addr < ebda + 1024; addr += 0x10 / sizeof(addr)) {
        rsdp = acpi_check_rsd_ptr(addr);
        if (rsdp != 0) {
            return rsdp;
        }
    }

    return 0;
}

int acpi_check_header(unsigned int *ptr, char *sig)
{
    if (memcmp(ptr, sig, 4) == 0) {
        char *check_ptr = (char *) ptr;
        int len = *(ptr + 1);
        char check = 0;
        while (0 < len--) {
            check += *check_ptr;
            check_ptr++;
        }
        if (check == 0) {
            return 0;
        }
    }
    return -1;
}

int acpi_enable(void)
{
    if ((inw((unsigned int) pm1a_cnt) & sci_en) == 0) {
        if (smi_cmd != 0 && acpi_enable_cmd != 0) {
            outb((unsigned int) smi_cmd, acpi_enable_cmd);
            
            int i;
            for (i = 0; i < 300; i++) {
                if ((inw((unsigned int) pm1a_cnt) & sci_en) == 1) {
                    break;
                }
                sleep(10);
            }
            if (pm1b_cnt != 0) {
                for (; i < 300; i++) {
                    if ((inw((unsigned int) pm1b_cnt) & sci_en) == 1) {
                        break;
                    }
                    sleep(10);
                }
            }
            if (i < 300) {
                printf("ACPI enabled.\n");
                return 0;
            }
            else {
                printf("Could not enable ACPI.\n");
                return -1;
            }
        }
        else {
            printf("No known way to enable ACPI.\n");
            return -1;
        }
    }
    else {
        return 0;
    }
}

int init_acpi(void)
{
    unsigned int *ptr = acpi_get_rsd_ptr();

    if (ptr != 0 && acpi_check_header(ptr, "RSDT") == 0) {
        int entrys = *(ptr + 1);
        entrys = (entrys - 36) / 4;
        ptr += 36 / 4;

        while (0 < entrys--) {
            if (acpi_check_header((unsigned int *) *ptr, "FACP") == 0) {
                entrys = -2;
                FACP *facp = (FACP *) *ptr;
                if (acpi_check_header((unsigned int *) facp->dsdt, "DSDT") == 0) {
                    char *s5_addr = (char *) facp->dsdt + 36;
                    int dsdt_length = *(facp->dsdt + 1) - 36;
                    while (0 < dsdt_length--) {
                        if (memcmp(s5_addr, "_S5_", 4) == 0) {
                            break;
                        }
                        s5_addr++;
                    }
                    
                    if (dsdt_length > 0) {
                        if ((*(s5_addr - 1) == 0x08 || (*(s5_addr - 2) == 0x08 && *(s5_addr - 1) == '\\')) && *(s5_addr + 4) == 0x12)
                        {
                            s5_addr += 5;
                            s5_addr += ((*s5_addr & 0xC0) >> 6) + 2;

                            if (*s5_addr == 0x0A) {
                                s5_addr++;
                            }
                            slp_typa = *(s5_addr) << 10;
                            s5_addr++;

                            if (*s5_addr == 0x0A) {
                                s5_addr++;
                            }
                            slp_typb = *(s5_addr) << 10;

                            smi_cmd = facp->smi_cmd;

                            acpi_enable_cmd = facp->acpi_enable;
                            acpi_disable_cmd = facp->acpi_disable;

                            pm1a_cnt = facp->pm1a_cnt_blk;
                            pm1b_cnt = facp->pm1b_cnt_blk;
                            
                            pm1_cnt_len = facp->pm1_cnt_len;

                            slp_en = 1 << 13;
                            sci_en = 1;

                            return 0;
                        }
                        else {
                            printf("\\_S5 parse error.\n");
                        }
                    }
                    else {
                        printf("\\_S5 not present.\n");
                    }
                }
                else {
                    printf("DSDT invalid.\n");
                }
            }
            ptr++;
        }
        printf("No valid FACP present.\n");
    }
    else {
        printf("No ACPI available.\n");
    }

    return -1;
}

void acpi_poweroff(void)
{
    if (sci_en == 0) {
        printf("ACPI not initialized.\n");
        return;
    }

    acpi_enable();

    outw((unsigned int) pm1a_cnt, slp_typa | slp_en);
    if (pm1b_cnt != 0) {
        outw((unsigned int) pm1b_cnt, slp_typb | slp_en);
    }

    printf("ACPI poweroff failed.\n");
}
