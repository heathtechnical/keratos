#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <vsprintf.h>
#include <debug/qemu.h>
#include <bootinfo/bootinfo.h>
#include <string.h>
#include <bitmap.h>
#include <types.h>
#include <mm/earlyalloc.h>

// Halt and catch fire function.
static void hcf(void)
{
    for (;;)
    {
        asm("hlt");
    }
}

void print_mmap_entry(uint8_t type, uint64_t base, uint64_t length)
{
    if (type != BOOTINFO_MEMMAP_USABLE)
        return;

    kprintf("[init] Memory Region - Base: %p, Length: %d bytes\n", base, length);

    size_t size = length / 4096;

    // Initialise each region's page bitmap
    bitmap_zero((uint64_t *)base, size);

    // Allocate bitmap pages for bitmap
    bitmap_set((uint64_t *)base, size, 0, size / BITMAP_WORD_SIZE / 8 + 1);

    ssize_t first_free = bitmap_find_first_zero((const uint64_t *)base, size);
    kprintf("[init]   First free page at index: %d\n", first_free);
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void)
{
    // Sync boot information from boot loader
    bootinfo_sync();

    // Initialise QEMU serial debug output
    set_putchar_func(qemu_putchar);

    // Initialise early memory allocator
    earlyalloc_init();

    // Log kernel load message
    kprintf("[init] Loading Keratos (bootloader=%s, version=%s) ...\n",
            bootinfo()->name,
            bootinfo()->version);

    // We're done, just hang...
    kprintf("[init] Halt!\n");
    hcf();
}
