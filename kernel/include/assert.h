#ifndef KERATOS_ASSERT_H
#define KERATOS_ASSERT_H

#include <vsprintf.h>

static inline void __kernel_assert(const char *expr, const char *file, int line, const char *func)
{
    kprintf("\n[ASSERTION FAILED] %s\n[ASSERTION FAILED] File: %s\n[ASSERTION FAILED] Line: %d\n[ASSERTION FAILED] Function: %s\n", expr, file, line, func);
    for (;;)
    {
        asm("hlt");
    }
}

#define assert(expr) \
    ((expr) ? (void)0 : __kernel_assert(#expr, __FILE__, __LINE__, __func__))

#endif