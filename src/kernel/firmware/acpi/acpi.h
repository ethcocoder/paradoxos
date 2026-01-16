

#include <firmware/acpi/tables/sdth.h>

#ifndef ACPI_H
#define ACPI_H

void acpi_init(struct stivale2_struct *stivale2_struct);
int acpi_check_sdt_header(sdt_header_t *sdt_header, const char *signature);
int acpi_verify_sdt_header_checksum(sdt_header_t *sdt_header, const char *signature);
sdt_header_t *acpi_find_sdt_table(const char *signature);

#endif
