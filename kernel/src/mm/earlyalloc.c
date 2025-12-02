#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <bootinfo/bootinfo.h>
#include <panic.h>
#include <vsprintf.h>
#include <types.h>
#include <stdbool.h>
#include <mm/earlyalloc.h>

earlyalloc_t earlyalloc;

earlyalloc_region_t earlyalloc_available[EARLYALLOC_MAX_REGIONS];
earlyalloc_region_t earlyalloc_reserved[EARLYALLOC_MAX_REGIONS];

/**
 * Callback to add bootloader memory map regions to earlyalloc.
 */
static void _earlyalloc_add_bootloader_regions(uint8_t type, uint64_t base, uint64_t length)
{
    earlyalloc_add_range(&earlyalloc.memory, base, length);
    if (type != BOOTINFO_MEMMAP_USABLE)
    {
        earlyalloc_add_range(&earlyalloc.reserved, base, length);
    }
}

/**
 * Check for overlap with reserved regions.
 */
bool _earlyalloc_range_is_free(uint64_t start, uint64_t size)
{
    uint64_t end = start + size;

    for (size_t i = 0; i < earlyalloc.reserved.cnt; i++)
    {
        earlyalloc_region_t *reserved = &earlyalloc.reserved.regions[i];
        uint64_t reserved_end = reserved->base + reserved->size;

        if (!(end <= reserved->base || start >= reserved_end))
        {
            return false;
        }
    }

    return true;
}

/**
 * Initialize earlyalloc system.
 */
void earlyalloc_init(void)
{
    // Initialize earlyalloc structures
    memset(&earlyalloc, 0, sizeof(earlyalloc_t));
    memset(earlyalloc_available, 0, sizeof(earlyalloc_available));
    memset(earlyalloc_reserved, 0, sizeof(earlyalloc_reserved));

    earlyalloc.memory.regions = earlyalloc_available;
    earlyalloc.memory.max = EARLYALLOC_MAX_REGIONS;

    earlyalloc.reserved.regions = earlyalloc_reserved;
    earlyalloc.reserved.max = EARLYALLOC_MAX_REGIONS;

    // Populate earlyalloc from bootinfo memory map
    bootinfo_memmap_iter(_earlyalloc_add_bootloader_regions);
}

/**
 * Find a suitable memory range for allocation.
 */
void *_earlyalloc_find_range(size_t size, size_t align)
{
    for (size_t i = 0; i < earlyalloc.memory.cnt; i++)
    {
        earlyalloc_region_t *region = &earlyalloc.memory.regions[i];
        uint64_t region_start = region->base;
        uint64_t region_end = region->base + region->size;

        uint64_t candidate_start = (region_start + align - 1) & ~(align - 1);
        while (candidate_start + size <= region_end)
        {
            if (_earlyalloc_range_is_free(candidate_start, size))
            {
                return (void *)(uintptr_t)candidate_start;
            }
            candidate_start += align;
        }
    }
    return NULL;
}

/**
 * Double the size of the regions array for the specified earlyalloc type.
 */
void earlyalloc_double_array(earlyalloc_type_t *target)
{
    size_t new_max = target->max * 2;
    earlyalloc_region_t *new_regions = (earlyalloc_region_t *)_earlyalloc_find_range(new_max * sizeof(earlyalloc_region_t), sizeof(earlyalloc_region_t));
    if (!new_regions)
    {
        panic("Failed to allocate memory for earlyalloc region expansion!");
    }

    memcpy(new_regions, target->regions, target->cnt * sizeof(earlyalloc_region_t));
    target->regions = new_regions;
    target->max = new_max;

    earlyalloc_add_range(&earlyalloc.reserved, (uint64_t)(uintptr_t)new_regions, new_max * sizeof(earlyalloc_region_t));
}

/**
 * Add a memory range to the specified earlyalloc type.
 */
void earlyalloc_add_range(earlyalloc_type_t *target, uint64_t base, uint64_t length)
{
    if (target->cnt >= target->max)
    {
        earlyalloc_double_array(target);
    }

    target->regions[target->cnt].base = base;
    target->regions[target->cnt].size = length;
    target->cnt++;
}

void earlyalloc_dump_region(earlyalloc_type_t *target)
{
    for (size_t i = 0; i < target->cnt; i++)
    {
        earlyalloc_region_t *region = &target->regions[i];
        kprintf("  Region - Base: %p, Size: %d bytes\n",
                (void *)(uintptr_t)region->base,
                region->size);
    }
}

/**
 * Allocate a memory region of given size and alignment.
 */
void *earlyalloc_alloc(size_t size, size_t align)
{
    void *addr = _earlyalloc_find_range(size, align);
    if (addr)
    {
        earlyalloc_add_range(&earlyalloc.reserved, (uint64_t)addr, size);
        return addr;
    }

    return NULL;
}
