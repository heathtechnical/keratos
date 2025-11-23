/**
 * bootinfo/limine.c
 * Limine boot information handling
 */
#include <limine.h>
#include <vsprintf.h>
#include <panic.h>
#include <stdbool.h>
#include <bootinfo/bootinfo.h>

bootinfo_info_t bootinfo_info;

__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests"))) volatile struct limine_bootloader_info_request bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 0,
};

__attribute__((used, section(".limine_requests"))) volatile struct limine_executable_cmdline_request executable_cmdline_request = {
    .id = LIMINE_EXECUTABLE_CMDLINE_REQUEST,
    .revision = 0,
};

__attribute__((used, section(".limine_requests"))) volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

__attribute__((used, section(".limine_requests"))) volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

/**
 * Sync boot information from Limine responses into bootinfo_info_t structure.
 */
void bootinfo_sync(void)
{
    if (!LIMINE_BASE_REVISION_SUPPORTED)
        panic("Limine base revision not supported!");

    if (bootloader_info_request.response == NULL)
        panic("Limine bootloader info response is NULL!");

    if (executable_cmdline_request.response == NULL)
        panic("Limine executable cmdline response is NULL!");

    if (hhdm_request.response == NULL)
        panic("Limine HHDM response is NULL!");

    bootinfo_info.name = (char *)(uintptr_t)bootloader_info_request.response->name;
    bootinfo_info.version = (char *)(uintptr_t)bootloader_info_request.response->version;
    bootinfo_info.cmdline = (char *)(uintptr_t)executable_cmdline_request.response->cmdline;
    bootinfo_info.hhdm_offset = hhdm_request.response->offset;
}

/**
 * Get a pointer to the boot information structure.
 */
bootinfo_info_t *bootinfo(void)
{
    return &bootinfo_info;
}

/**
 * Iterate over memory map entries, invoking the callback for each entry.
 * The callback receives the type, base address, and length of each entry.
 */
void bootinfo_memmap_iter(void (*callback)(uint8_t type, uint64_t base, uint64_t length))
{
    struct limine_memmap_entry *entry;
    uint8_t type;

    for (size_t i = 0; i < memmap_request.response->entry_count; i++)
    {
        entry = memmap_request.response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE)
            type = BOOTINFO_MEMMAP_USABLE;
        else
            type = BOOTINFO_MEMMAP_RESERVED;

        callback(type, entry->base + hhdm_request.response->offset, entry->length);
    }
}
