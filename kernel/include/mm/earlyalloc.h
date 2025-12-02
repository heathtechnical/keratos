#ifndef EARLYALLOC_H
#define EARLYALLOC_H

#include <stdint.h>
#include <stddef.h>

#define EARLYALLOC_MAX_REGIONS 128

typedef struct
{
    uint64_t base;
    uint64_t size;
} earlyalloc_region_t;

typedef struct
{
    size_t cnt;
    size_t max;
    earlyalloc_region_t *regions;
} earlyalloc_type_t;

typedef struct
{
    earlyalloc_type_t memory;
    earlyalloc_type_t reserved;
} earlyalloc_t;

void earlyalloc_init(void);
void *earlyalloc_alloc(size_t size, size_t align);
void earlyalloc_dump_regions(void);
void earlyalloc_add_range(earlyalloc_type_t *target, uint64_t base, uint64_t length);
void earlyalloc_dump_region(earlyalloc_type_t *target);

#endif // EARLYALLOC_H