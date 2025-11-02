#include <io.h>
#include <stddef.h>
#include <stdarg.h>
#include <vsprintf.h>

// Single character output to QEMU serial
void qemu_putchar(char c)
{
    const uint16_t COM1_PORT = 0x3F8;
    const uint16_t COM1_STATUS = 0x3FD;

    // Wait for transmit buffer to be empty
    while ((inb(COM1_STATUS) & 0x20) == 0)
        ;

    // Send the character
    outb(COM1_PORT, c);
}

// String output (keeping for compatibility)
void qemu_serial_write(const char *str, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        qemu_putchar(str[i]);
    }
}

int qemu_printf(const char *fmt, ...)
{
    va_list args;
    int rv;

    va_start(args, fmt);
    rv = vprintf(fmt, args);
    va_end(args);
    return rv;
}
