#include <stdarg.h>
#include <vsprintf.h>

void panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    kprintf("\n*** KERNEL PANIC ***\n");
    vprintf(fmt, args);
    kprintf("\nSystem halted.\n");
    va_end(args);

    for (;;)
        asm("hlt");
}