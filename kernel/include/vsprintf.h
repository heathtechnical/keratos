#ifndef VSPRINTF_H
#define VSPRINTF_H

#include <stdarg.h>
#include <stddef.h>

// Function pointer type for character output
typedef void (*putchar_func_t)(char c);

// Printf functions that use function pointer for output
int vprintf_func(putchar_func_t putchar_func, const char *fmt, va_list args);
// int printf_func(putchar_func_t putchar_func, const char *fmt, ...);

// Traditional buffer-based functions
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int snprintf(char *buf, size_t size, const char *fmt, ...);

// Set the global putchar function and use printf normally
void set_putchar_func(putchar_func_t func);
// int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list args);
int kprintf(const char *fmt, ...);

#endif // VSPRINTF_H