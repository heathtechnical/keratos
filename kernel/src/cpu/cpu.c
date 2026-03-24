#include <cpu/cpu.h>
#include <cpu/irq.h>
#include <vsprintf.h>
#include <assert.h>
#include <string.h>

cpuinfo_t cpu_bsp_info;

void init_cpu_bsp(void)
{
    uint32_t eax, ebx, ecx, edx;
    init_idt();

    // Get vendor string
    cpuid(0, &eax, &ebx, &ecx, &edx);
    memset(cpu_bsp_info.vendor, 0, 13);

    // Extract vendor string
    *(uint32_t *)&cpu_bsp_info.vendor[0] = ebx;
    *(uint32_t *)&cpu_bsp_info.vendor[4] = edx;
    *(uint32_t *)&cpu_bsp_info.vendor[8] = ecx;

    // Get feature flags
    cpuid(1, &eax, &ebx, &ecx, &edx);
    cpu_bsp_info.features[0] = eax;
    cpu_bsp_info.features[1] = ebx;
    cpu_bsp_info.features[2] = ecx;
    cpu_bsp_info.features[3] = edx;

    // Check for APIC
    assert(cpu_bsp_info.features[3] & CPUID_FEAT_EDX_APIC);
}

void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    __asm__ volatile("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(leaf));
}