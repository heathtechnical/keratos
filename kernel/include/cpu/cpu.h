#ifndef CPU_CPU_H
#define CPU_CPU_H

#include <stdint.h>

void init_cpu_bsp(void);
void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);

typedef struct
{
    char vendor[13] __attribute__((aligned(4)));
    uint32_t features[4];
} cpuinfo_t;

#define CPUID_FEAT_EDX_APIC (1 << 9)

#endif // CPU_CPU_H