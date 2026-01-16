

#ifndef SDTH_H
#define SDTH_H

typedef struct __attribute__((__packed__))
{
    char	signature[4];
    uint32_t	length;
    uint8_t	revision;
    uint8_t	checksum;
    char	oem_string[6];
    char	oem_table_id[8];
    uint32_t	oem_revision;
    char	creator_id[4];
    uint32_t	creator_revision;
} sdt_header_t;

#endif
