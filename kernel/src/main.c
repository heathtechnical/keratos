#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <vsprintf.h>
#include <debug/qemu.h>
#include <bootinfo/bootinfo.h>
#include <string.h>
#include <types.h>
#include <mm/earlyalloc.h>
#include <cpu/cpu.h>
#include <cpu/irq.h>
#include <mm/page.h>
#include <task.h>
#include <panic.h>

#include <tasks/fbsplash.h>

// Halt and catch fire function.
static void hcf(void)
{
    for (;;)
    {
        asm("hlt");
    }
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

    // Initialise bootstrap CPU
    cpu_bsp_init();

    // Initialise task subsystem
    task_init();

    // Enable interrupts
    irq_enable();

    // Create demo framebuffer splash screen
    struct task_t task1;
    task_create(&task1, "1111", &fbsplash_loop);

    // We're done, just hang...
    kprintf("[init] Halt!\n");
    hcf();
}
