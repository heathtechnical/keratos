#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <vsprintf.h>

// Global putchar function pointer
static putchar_func_t global_putchar = NULL;

// Set the global putchar function
void set_putchar_func(putchar_func_t func)
{
    global_putchar = func;
}

// Printf implementation using function pointer for output
int vprintf_func(putchar_func_t putchar_func, const char *fmt, va_list args)
{
    int count = 0;

    for (; *fmt; fmt++)
    {
        if (*fmt != '%')
        {
            putchar_func(*fmt);
            count++;
            continue;
        }

        fmt++;
        switch (*fmt)
        {
        case 'c':
        {
            char c = (char)va_arg(args, int);
            putchar_func(c);
            count++;
            break;
        }
        case 's':
        {
            const char *s = va_arg(args, const char *);
            while (*s)
            {
                putchar_func(*s++);
                count++;
            }
            break;
        }
        case 'd':
        {
            int value = va_arg(args, int);
            char numbuf[12];
            char *nump = numbuf + sizeof(numbuf) - 1;
            int is_negative = 0;

            *nump = '\0';

            if (value < 0)
            {
                is_negative = 1;
                value = -value;
            }

            if (value == 0)
            {
                *(--nump) = '0';
            }
            else
            {
                while (value > 0)
                {
                    *(--nump) = '0' + (value % 10);
                    value /= 10;
                }
            }

            if (is_negative)
            {
                *(--nump) = '-';
            }

            while (*nump)
            {
                putchar_func(*nump++);
                count++;
            }
            break;
        }
        case 'x':
        {
            unsigned int value = va_arg(args, unsigned int);
            char numbuf[9];
            char *nump = numbuf + sizeof(numbuf) - 1;

            *nump = '\0';

            if (value == 0)
            {
                *(--nump) = '0';
            }
            else
            {
                while (value > 0)
                {
                    unsigned int digit = value & 0xF;
                    *(--nump) = (digit < 10) ? (digit + '0') : (digit - 10 + 'a');
                    value >>= 4;
                }
            }

            while (*nump)
            {
                putchar_func(*nump++);
                count++;
            }
            break;
        }
        case 'p':
        {
            void *ptr = va_arg(args, void *);
            uintptr_t value = (uintptr_t)ptr;
            char numbuf[2 + sizeof(uintptr_t) * 2 + 1]; // "0x" + hex digits + null
            char *nump = numbuf + sizeof(numbuf) - 1;
            *nump = '\0';

            if (value == 0)
            {
                *(--nump) = '0';
            }
            else
            {
                while (value > 0)
                {
                    unsigned int digit = value & 0xF;
                    *(--nump) = (digit < 10) ? (digit + '0') : (digit - 10 + 'a');
                    value >>= 4;
                }
            }
            *(--nump) = 'x';
            *(--nump) = '0';

            while (*nump)
            {
                putchar_func(*nump++);
                count++;
            }
            break;
        }
        case '%':
            putchar_func('%');
            count++;
            break;
        default:
            putchar_func('%');
            putchar_func(*fmt);
            count += 2;
            break;
        }
    }

    return count;
}

// Printf with function pointer
// int printf_func(putchar_func_t putchar_func, const char *fmt, ...)
// {
//     va_list args;
//     va_start(args, fmt);
//     int result = vprintf_func(putchar_func, fmt, args);
//     va_end(args);
//     return result;
// }

// // Global printf using set putchar function
// int printf(const char *fmt, ...)
// {
//     if (!global_putchar)
//         return 0;

//     va_list args;
//     va_start(args, fmt);
//     int result = vprintf_func(global_putchar, fmt, args);
//     va_end(args);
//     return result;
// }

int vprintf(const char *fmt, va_list args)
{
    if (!global_putchar)
        return 0;
    return vprintf_func(global_putchar, fmt, args);
}

int kprintf(const char *fmt, ...)
{
    va_list args;
    int rv;

    va_start(args, fmt);
    rv = vprintf(fmt, args);
    va_end(args);
    return rv;
}
