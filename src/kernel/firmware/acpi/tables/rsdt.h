

#include <firmware/acpi/tables/sdth.h>

#ifndef RSDT_H
#define RSDT_H

typedef struct __attribute__((__packed__)) 
{
    sdt_header_t header;
    uint32_t entries[];
} rsdt_structure_t;

#endif
