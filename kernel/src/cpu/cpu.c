#include <cpu/irq.h>

void init_cpu_bsp(void)
{
    init_idt();
}