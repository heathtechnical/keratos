#ifndef PAGE_H
#define PAGE_H

#include <stdbool.h>
#include <bootinfo/bootinfo.h>

#define FROM_PHYS(p) ((void *)((uintptr_t)(p) + bootinfo()->hhdm_offset))
#define TO_PHYS(v) ((uintptr_t)(v) - bootinfo()->hhdm_offset)

#define PG_PML4_INDEX(v) ((((uintptr_t)(v)) >> 39) & 0x1FFULL)
#define PG_PDPT_INDEX(v) ((((uintptr_t)(v)) >> 30) & 0x1FFULL)
#define PG_PD_INDEX(v) ((((uintptr_t)(v)) >> 21) & 0x1FFULL)
#define PG_PT_INDEX(v) ((((uintptr_t)(v)) >> 12) & 0x1FFULL)

#define PG_PAGE_PRESENT (1ULL << 0)
#define PG_PAGE_WRITABLE (1ULL << 1)
#define PG_PAGE_NOCACHE (1ULL << 4)

#define PG_PAGE_SIZE 4096

struct pg_pml4_t
{
    uint64_t present : 1;
    uint64_t writable : 1;
    uint64_t user : 1;
    uint64_t write_through : 1;
    uint64_t cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t ignored1 : 1;
    uint64_t page_size : 1;
    uint64_t ignored2 : 4;
    uint64_t addr : 40;
    uint64_t ignored3 : 11;
    uint64_t nx : 1;
};

struct pg_pdpt_t
{
    uint64_t present : 1;       // bit 0
    uint64_t writable : 1;      // bit 1
    uint64_t user : 1;          // bit 2
    uint64_t write_through : 1; // bit 3
    uint64_t cache_disable : 1; // bit 4
    uint64_t accessed : 1;      // bit 5
    uint64_t ignored1 : 1;      // bit 6
    uint64_t page_size : 1;     // bit 7 (0 = points to PD, 1 = 1GiB page)
    uint64_t ignored2 : 4;      // bits 8-11
    uint64_t addr : 40;         // bits 12-51: physical address >> 12
    uint64_t ignored3 : 11;     // bits 52-62
    uint64_t nx : 1;            // bit 63
};

struct pg_pd_t
{
    uint64_t present : 1;       // bit 0
    uint64_t writable : 1;      // bit 1
    uint64_t user : 1;          // bit 2
    uint64_t write_through : 1; // bit 3
    uint64_t cache_disable : 1; // bit 4
    uint64_t accessed : 1;      // bit 5
    uint64_t ignored1 : 1;      // bit 6
    uint64_t page_size : 1;     // bit 7 = 0
    uint64_t ignored2 : 4;      // bits 8-11
    uint64_t addr : 40;         // bits 12-51 (phys addr >> 12)
    uint64_t ignored3 : 11;     // bits 52-62
    uint64_t nx : 1;            // bit 63
};

struct pg_pt_t
{
    uint64_t present : 1;       // bit 0
    uint64_t writable : 1;      // bit 1
    uint64_t user : 1;          // bit 2
    uint64_t write_through : 1; // bit 3
    uint64_t cache_disable : 1; // bit 4
    uint64_t accessed : 1;      // bit 5
    uint64_t dirty : 1;         // bit 6
    uint64_t pat : 1;           // bit 7 (PAT index bit 2)
    uint64_t global : 1;        // bit 8
    uint64_t ignored1 : 3;      // bits 9-11
    uint64_t addr : 40;         // bits 12-51 (phys addr >> 12)
    uint64_t ignored2 : 11;     // bits 52-62
    uint64_t nx : 1;            // bit 63
};

bool map_page(struct pg_pml4_t *p, uintptr_t virt, uintptr_t phys, uint64_t flags);
void pt_dump(void);

#endif // PAGE_H