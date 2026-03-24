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

// Halt and catch fire function.
static void hcf(void)
{
    for (;;)
    {
        asm("hlt");
    }
}

#define KERNEL_SIGNING_KEY 0xDEADBEEF
uint64_t kernel_key = KERNEL_SIGNING_KEY;

struct kernel_token_t
{
    char resource[32];
    uint32_t permissions;
    uint32_t expiry;
    uint32_t nonce;
    uint64_t signature;
};

uint64_t sign_token(struct kernel_token_t *t)
{
    uint64_t hash = 0;
    for (int i = 0; i < sizeof(t->resource); i++)
        hash += t->resource[i];
    hash += t->permissions;
    hash += t->expiry;
    hash += t->nonce;
    hash ^= kernel_key;
    return hash;
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
    init_cpu_bsp();

    // struct kernel_token_t mytok = {
    //     resource : "file:///foo",
    //     permissions : 1,
    //     expiry : 0,
    //     nonce : 123,
    //     signature : 0
    // };

    // mytok.signature = sign_token(&mytok);

    // kprintf("[toktest] signature=0x%x", mytok.signature);

    // We're done, just hang...
    kprintf("[init] Halt!\n");
    hcf();
}
