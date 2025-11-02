#ifndef DEBUG_H
#define DEBUG_H

// QEMU serial debug functions
void qemu_putchar(char c);
void qemu_serial_write(const char *str, size_t len);

#endif // DEBUG_H