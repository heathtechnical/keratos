#include <cpu/cpu.h>
#include <cpu/irq.h>
#include <cpu/lapic.h>
#include <vsprintf.h>
#include <assert.h>
#include <string.h>
#include <bootinfo/bootinfo.h>
#include <stdint.h>
#include <mm/earlyalloc.h>
#include <mm/page.h>
#include <io.h>
#include <panic.h>

cpuinfo_t cpu_bsp_info;

void cpu_bsp_init(void)
{
    uint32_t eax, ebx, ecx, edx;
    volatile uint32_t *lapic = (volatile uint32_t *)LAPIC_VIRT_ADDR;

    // Initialise IDT
    init_idt();

    // Get vendor string
    cpu_cpuid(0, &eax, &ebx, &ecx, &edx);
    memset(cpu_bsp_info.vendor, 0, 13);

    // Extract vendor string
    *(uint32_t *)&cpu_bsp_info.vendor[0] = ebx;
    *(uint32_t *)&cpu_bsp_info.vendor[4] = edx;
    *(uint32_t *)&cpu_bsp_info.vendor[8] = ecx;

    kprintf("[cpu/bsp]: Vendor is %s\n", cpu_bsp_info.vendor);

    // Get feature flags
    cpu_cpuid(1, &eax, &ebx, &ecx, &edx);
    cpu_bsp_info.features[0] = eax;
    cpu_bsp_info.features[1] = ebx;
    cpu_bsp_info.features[2] = ecx;
    cpu_bsp_info.features[3] = edx;

    // Check for APIC
    assert(cpu_bsp_info.features[3] & CPUID_FEAT_EDX_APIC);

    lapic_init();

    outb(0x21, 0xFF); // Master PIC
    outb(0xA1, 0xFF); // Slave PIC

    lapic[0x320 / 4] = 0x20040; // 0x40 | (1 << 17)
    lapic[0x3E0 / 4] = 0x3;     // Divide by 16
    lapic[0x380 / 4] = 100000;  // Initial count (tune as needed)
}

void cpu_cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    __asm__ volatile("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(leaf));
}

uint64_t cpu_msr_read(uint32_t msr)
{
    uint32_t low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

void cpu_msr_write(uint32_t msr, uint64_t value)
{
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)(value >> 32);
    __asm__ volatile("wrmsr"
                     :
                     : "c"(msr), "a"(low), "d"(high));
}

uint64_t cpu_cr3_read(void)
{
    uint64_t value;
    __asm__ volatile("mov %%cr3, %0" : "=r"(value));
    return value;
}