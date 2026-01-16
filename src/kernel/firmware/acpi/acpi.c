

#include <stddef.h>

#include <boot/stivale2.h>
#include <boot/stivale2_boot.h>
#include <firmware/acpi/tables/madt.h>
#include <firmware/acpi/tables/rsdp.h>
#include <firmware/acpi/tables/rsdt.h>
#include <firmware/acpi/tables/sdth.h>
#include <firmware/acpi/acpi.h>
#include <memory/mem.h>
#include <libk/debug/debug.h>
#include <libk/log/log.h>
#include <libk/stdio/stdio.h>
#include <libk/string/string.h>

static rsdt_structure_t *rsdt;

// TODO: write description when fully done (at least when MADT and APIC are done)
void acpi_init(struct stivale2_struct *stivale2_struct)
{
    struct stivale2_struct_tag_rsdp *rsdp_tag = stivale2_get_tag(stivale2_struct,
            STIVALE2_STRUCT_TAG_RSDP_ID);

    rsdp_init(rsdp_tag->rsdp);

    rsdt = (rsdt_structure_t *)phys_to_higher_half_data((uintptr_t)get_rsdp_structure()->rsdt_address);

    // having a RSDT is equivalent to having ACPI supported
    if (acpi_check_sdt_header(&rsdt->header, "RSDT") != 0)
    {
        serial_log(ERROR, "No ACPI was found on this computer!\n");
        kernel_log(ERROR, "No ACPI was found on this computer!\n");


        serial_log(ERROR, "Kernel halted!\n");
        kernel_log(ERROR, "Kernel halted!\n");

        for (;;)
            asm ("hlt");
    }

    madt_init();

    serial_log(INFO, "ACPI initialized\n");
    kernel_log(INFO, "ACPI initialized\n");
}

// check if the signature of the SDT header matches the parameter (signature) and
// verify it's checksum
int acpi_check_sdt_header(sdt_header_t *sdt_header, const char *signature)
{
    if (memcmp(sdt_header->signature, signature, 4) == 0 &&
            acpi_verify_sdt_header_checksum(sdt_header, signature) == 0)
        return 0;

    return 1;
}

// check if the sum of all bytes in a SDT header is equal to zero
int acpi_verify_sdt_header_checksum(sdt_header_t *sdt_header, const char *signature)
{
    uint8_t checksum = 0;
    uint8_t *ptr = (uint8_t *)sdt_header;
    uint8_t current_byte;

    serial_log(INFO, "Verifying %s checksum:\n", signature);
    kernel_log(INFO, "Verifying %s checksum:\n", signature);

    serial_set_color(TERM_PURPLE);

    debug("First %d bytes are being checked: ", sdt_header->length);
    printk(GFX_PURPLE, "First %d bytes are being checked: ", sdt_header->length);

    for (uint8_t i = 0; i < sdt_header->length; i++)
    {
        current_byte = ptr[i];
        debug("%x ", current_byte);
        printk(GFX_PURPLE, "%x ", current_byte);

        checksum += current_byte;
    }

    debug("\n");
    printk(GFX_PURPLE, "\n");

    serial_set_color(TERM_COLOR_RESET);

    checksum = checksum & 0xFF;

    if (checksum == 0)
    {
        serial_log(INFO, "%s checksum is verified.\n", signature);
        kernel_log(INFO, "%s checksum is verified.\n", signature);

        return 0;
    }
    else
    {
        serial_log(ERROR, "%s checksum isn't 0! Checksum: 0x%x\n", signature, checksum);
        kernel_log(ERROR, "%s checksum isn't 0! Checksum: 0x%x\n", signature, checksum);

        return 1;
    }
}

// traverse RSDT to find table according to identifier
sdt_header_t *acpi_find_sdt_table(const char *signature)
{
    size_t entry_count = (rsdt->header.length - sizeof(rsdt->header)) / (has_xsdt() ? 8 : 4);
    sdt_header_t *current_entry;

    for (size_t i = 0; i < entry_count; i++)
    {
        current_entry = (sdt_header_t *)(uintptr_t)rsdt->entries[i];

        if (acpi_check_sdt_header(current_entry, signature) == 0)
            return (sdt_header_t *)phys_to_higher_half_data((uintptr_t)current_entry);
    }

    serial_log(ERROR, "Could not find SDT with signature '%s'!\n", signature);
    kernel_log(ERROR, "Could not find SDT with signature '%s'!\n", signature);

    return NULL;
}
