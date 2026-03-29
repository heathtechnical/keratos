#ifndef CPU_LAPIC_H
#define CPU_LAPIC_H

void lapic_send_eoi(void);
void lapic_init(void);

#define LAPIC_VIRT_ADDR 0xFFFFFFFFFEE00000ULL
#define LAPIC_BASE_MSR_ENABLE 0x800
#define LAPIC_BASE_X2APIC_ENABLE 0x400

#endif // CPU_LAPIC_H
