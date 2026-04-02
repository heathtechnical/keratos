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

struct framebuffer
{
    void *address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint16_t bpp; // Bits per pixel
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
    uint8_t unused[7];
    uint64_t edid_size;
    void *edid;

    /* Response revision 1 */
    uint64_t mode_count;
    void **modes;
};

// Halt and catch fire function.
static void hcf(void)
{
    for (;;)
    {
        asm("hlt");
    }
}

static void fb_put_pixel(struct framebuffer *fb, uint64_t x, uint64_t y, uint32_t color)
{
    if (x >= fb->width || y >= fb->height)
        return;

    uint32_t *base = (uint32_t *)fb->address;
    uint64_t stride = fb->pitch / 4;
    base[y * stride + x] = color;
}

static void fb_fill_rect(struct framebuffer *fb, uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t color)
{
    for (uint64_t yy = 0; yy < h; yy++)
    {
        for (uint64_t xx = 0; xx < w; xx++)
        {
            fb_put_pixel(fb, x + xx, y + yy, color);
        }
    }
}

static void spin_delay(uint64_t cycles)
{
    for (volatile uint64_t i = 0; i < cycles; i++)
        __asm__ volatile("" ::: "memory");
}

void task1_code()
{
    uint64_t fb_count = bootinfo()->framebuffers_count;

    if (fb_count == 0)
        panic("No framebuffer available");

    struct framebuffer *fb = (struct framebuffer *)bootinfo()->framebuffers[0];

    if (fb->bpp != 32)
        panic("Only 32bpp framebuffer is supported");

    kprintf("[task/task1]: framebuffer %ux%u pitch=%u bpp=%u addr=%p\n",
            (unsigned)fb->width,
            (unsigned)fb->height,
            (unsigned)fb->pitch,
            (unsigned)fb->bpp,
            fb->address);

    const uint32_t bg = 0x00101010;
    const uint32_t fg = 0x00ff0040;
    const uint32_t dim = 0x00003512;
    const uint64_t dot_size = 10;
    const int32_t radius = 24;

    const int32_t offsets[8][2] = {
        {0, -radius},
        {radius, -radius},
        {radius, 0},
        {radius, radius},
        {0, radius},
        {-radius, radius},
        {-radius, 0},
        {-radius, -radius},
    };

    uint64_t cx = fb->width / 2;
    uint64_t cy = fb->height / 2;
    uint64_t frame = 0;

    fb_fill_rect(fb, 0, 0, fb->width, fb->height, bg);

    while (1)
    {
        for (uint64_t i = 0; i < 8; i++)
        {
            int64_t x = (int64_t)cx + offsets[i][0] - (int64_t)(dot_size / 2);
            int64_t y = (int64_t)cy + offsets[i][1] - (int64_t)(dot_size / 2);

            if (x >= 0 && y >= 0)
            {
                uint32_t color = (i == frame) ? fg : dim;
                fb_fill_rect(fb, (uint64_t)x, (uint64_t)y, dot_size, dot_size, color);
            }
        }

        frame = (frame + 1) % 8;
        task_sleep(100);
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

    struct task_t task1;

    task_init();

    task_create(&task1, "1111", &task1_code);

    irq_enable();

    // We're done, just hang...
    kprintf("[init] Halt!\n");
    hcf();
}
