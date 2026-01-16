

#include <stddef.h>

#include <boot/stivale2.h>
#include <boot/stivale2_boot.h>
#include <firmware/acpi/tables/madt.h>
#include <firmware/acpi/acpi.h>
#include <libk/alloc/kmalloc.h>
#include <libk/log/log.h>

madt_structure_t *madt;

madt_lapic_t		    **madt_lapics;
madt_io_apic_t		    **madt_io_apics;
madt_iso_t		    **madt_isos;
madt_lapic_nmi_t	    **madt_lapic_nmis;

size_t madt_lapics_i	    = 0;
size_t madt_io_apics_i	    = 0;
size_t madt_isos_i	    = 0;
size_t madt_lapic_nmis_i    = 0;

void madt_init(void)
{
    madt = (madt_structure_t *)(uintptr_t)acpi_find_sdt_table("APIC");

    if (madt == NULL)
    {
        serial_log(ERROR, "No MADT was found on this computer!\n");
        kernel_log(ERROR, "No MADT was found on this computer!\n");


        serial_log(ERROR, "Kernel halted!\n");
        kernel_log(ERROR, "Kernel halted!\n");

        for (;;)
            asm ("hlt");
    }


    madt_lapics	    = kmalloc(256);
    madt_io_apics   = kmalloc(256);
    madt_isos	    = kmalloc(256);
    madt_lapic_nmis = kmalloc(256);


    size_t madt_table_length = (size_t)&madt->header + madt->header.length;

    uint8_t *table_ptr = (uint8_t *)&madt->table;

    while ((size_t)table_ptr < madt_table_length)
    {
        switch (*table_ptr)
        {
            case PROCESSOR_LOCAL_APIC:
                serial_log(INFO, "MADT Initialization: Found local APIC\n");
                kernel_log(INFO, "MADT Initialization: Found local APIC\n");

                madt_lapics[madt_lapics_i++] = (madt_lapic_t *)table_ptr;

                break;

            case IO_APIC:
                serial_log(INFO, "MADT Initialization: Found IO APIC\n");
                kernel_log(INFO, "MADT Initialization: Found IO APIC\n");

                madt_io_apics[madt_io_apics_i++] = (madt_io_apic_t *)table_ptr;

                break;

            case INTERRUPT_SOURCE_OVERRIDE:
                serial_log(INFO, "MADT Initialization: Found interrupt source override\n");
                kernel_log(INFO, "MADT Initialization: Found interrupt source override\n");

                madt_isos[madt_isos_i++] = (madt_iso_t *)table_ptr;

                break;

            case LAPIC_NMI:
                serial_log(INFO, "MADT Initialization: Found local APIC non maskable interrupt\n");
                kernel_log(INFO, "MADT Initialization: Found local APIC non maskable interrupt\n");

                madt_lapic_nmis[madt_lapic_nmis_i++] = (madt_lapic_nmi_t *)table_ptr;

                break;
        }

        table_ptr += *(table_ptr + 1);
    }
}
