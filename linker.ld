/* ParadoxOS Linker Script */
/* Placeholder - to be populated later */

ENTRY(_start)
SECTIONS
{
    . = 1M;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss  : { *(.bss) }
}
