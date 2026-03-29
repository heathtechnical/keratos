#include <stdint.h>
#include <mm/page.h>
#include <cpu/cpu.h>
#include <cpu/lapic.h>
#include <panic.h>
#include <vsprintf.h>

void lapic_init(void)
{
    uint64_t lapic_msr = cpu_msr_read(0x1B);

    // Check LAPIC is enabled
    if (!(lapic_msr & LAPIC_BASE_MSR_ENABLE))
        panic("LAPIC is not enabled");

    if (lapic_msr & LAPIC_BASE_X2APIC_ENABLE)
        panic("x2APIC is not supported");

    // Get pml4 base
    struct pg_pml4_t *p = (struct pg_pml4_t *)(FROM_PHYS(cpu_cr3_read()));

    // Map LAPIC_VIRT_ADDR
    if (!map_page(p, LAPIC_VIRT_ADDR, cpu_msr_read(0x1B) & 0xFFFFF000, PG_PAGE_PRESENT | PG_PAGE_WRITABLE | PG_PAGE_NOCACHE))
    {
        panic("Unable to map page");
    }

    kprintf("[cpu/bsp]: Mapped LAPIC MMIO at %p\n", LAPIC_VIRT_ADDR);
}

void lapic_send_eoi(void)
{
    volatile uint32_t *lapic_eoi = (volatile uint32_t *)(LAPIC_VIRT_ADDR + 0xB0);
    *lapic_eoi = 0;
}
