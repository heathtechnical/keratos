#include <stdbool.h>
#include <mm/earlyalloc.h>
#include <mm/page.h>
#include <cpu/cpu.h>
#include <vsprintf.h>
#include <string.h>

/**
 * Map a single 4 KiB page into the given PML4.
 *
 * Returns false if:
 * - the root page table pointer is NULL
 * - allocating an intermediate paging structure fails
 * - the address is already covered by a 1 GiB or 2 MiB large-page mapping
 * - the final PTE is already present
 */
bool map_page(struct pg_pml4_t *p, uintptr_t virt, uintptr_t phys, uint64_t flags)
{
    struct pg_pdpt_t *pdpt;
    struct pg_pd_t *pd;
    struct pg_pt_t *pt;

    if (!p)
        return false;

    if ((uintptr_t)p % PG_PAGE_SIZE)
        return false;

    if ((virt & 0xFFF) || (phys & 0xFFF))
        return false;

    if (!p[PG_PML4_INDEX(virt)].present)
    {
        pdpt = (struct pg_pdpt_t *)earlyalloc_alloc(PG_PAGE_SIZE, PG_PAGE_SIZE);

        if (!pdpt)
            return false;

        memset(pdpt, 0, PG_PAGE_SIZE);
        p[PG_PML4_INDEX(virt)].present = 1;
        p[PG_PML4_INDEX(virt)].writable = 1;
        p[PG_PML4_INDEX(virt)].addr = ((uintptr_t)TO_PHYS(pdpt)) >> 12;
    }
    else
    {
        pdpt = (struct pg_pdpt_t *)(FROM_PHYS(p[PG_PML4_INDEX(virt)].addr << 12));
    }

    if (!pdpt[PG_PDPT_INDEX(virt)].present)
    {
        pd = (struct pg_pd_t *)earlyalloc_alloc(PG_PAGE_SIZE, PG_PAGE_SIZE);

        if (!pd)
            return false;

        memset(pd, 0, PG_PAGE_SIZE);
        pdpt[PG_PDPT_INDEX(virt)].present = 1;
        pdpt[PG_PDPT_INDEX(virt)].writable = 1;
        pdpt[PG_PDPT_INDEX(virt)].addr = ((uintptr_t)TO_PHYS(pd)) >> 12;
    }
    else
    {
        if (pdpt[PG_PDPT_INDEX(virt)].page_size)
            return false;

        pd = (struct pg_pd_t *)(FROM_PHYS(pdpt[PG_PDPT_INDEX(virt)].addr << 12));
    }

    if (!pd[PG_PD_INDEX(virt)].present)
    {
        pt = (struct pg_pt_t *)earlyalloc_alloc(PG_PAGE_SIZE, PG_PAGE_SIZE);

        if (!pt)
            return false;

        memset(pt, 0, PG_PAGE_SIZE);
        pd[PG_PD_INDEX(virt)].present = 1;
        pd[PG_PD_INDEX(virt)].writable = 1;
        pd[PG_PD_INDEX(virt)].addr = ((uintptr_t)TO_PHYS(pt)) >> 12;
    }
    else
    {
        if (pd[PG_PD_INDEX(virt)].page_size)
            return false;

        pt = (struct pg_pt_t *)(FROM_PHYS(pd[PG_PD_INDEX(virt)].addr << 12));
    }

    if (pt[PG_PT_INDEX(virt)].present)
        return false;

    pt[PG_PT_INDEX(virt)].present = !!(flags & PG_PAGE_PRESENT);
    pt[PG_PT_INDEX(virt)].writable = !!(flags & PG_PAGE_WRITABLE);
    pt[PG_PT_INDEX(virt)].user = !!(flags & (1ULL << 2));
    pt[PG_PT_INDEX(virt)].write_through = !!(flags & (1ULL << 3));
    pt[PG_PT_INDEX(virt)].cache_disable = !!(flags & PG_PAGE_NOCACHE);
    pt[PG_PT_INDEX(virt)].nx = !!(flags & (1ULL << 63));
    pt[PG_PT_INDEX(virt)].addr = phys >> 12;

    return true;
}