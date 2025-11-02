/**
 * Keratos : debug/qemu.h - QEMU debug output functions
 */

#ifndef DEBUG_QEMU_H
#define DEBUG_QEMU_H

#include <stddef.h>

void qemu_putchar(char c);
void qemu_printf(const char *fmt, ...);

#endif // DEBUG_QEMU_H