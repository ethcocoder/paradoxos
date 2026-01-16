
#include <stdbool.h>

#ifndef RSDP_H
#define RSDP_H

typedef struct __attribute__((__packed__)) 
{
    // ACPI version 1.0
    char	signature[8];
    uint8_t 	checksum;
    char    	oemid[6];
    uint8_t 	revision;
    uint32_t	rsdt_address;

    // ACPI version 2.0+
    uint32_t	length;
    uint64_t	xsdt_address;
    uint8_t	extended_checksum;
    uint8_t	reserved[3];

} rsdp_structure_t;

void rsdp_init(uint64_t rsdp_address);
void rsdp_verify_checksum(uint64_t rsdp_address);
rsdp_structure_t *get_rsdp_structure(void);
bool has_xsdt(void);

#endif
