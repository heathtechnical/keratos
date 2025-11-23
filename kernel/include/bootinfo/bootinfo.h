#ifndef KERATOS_BLINFO_H
#define KERATOS_BLINFO_H

#include <stdint.h>

#define BOOTINFO_MEMMAP_USABLE 0x1
#define BOOTINFO_MEMMAP_RESERVED 0x2

typedef struct
{
    char *name;
    char *version;
    char *cmdline;
    uint64_t hhdm_offset;
} bootinfo_info_t;

bootinfo_info_t *bootinfo(void);
void bootinfo_sync(void);
void bootinfo_memmap_iter(void (*callback)(uint8_t type, uint64_t base, uint64_t length));

#endif