#include <stdint.h>
#include <bootinfo/bootinfo.h>
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

void fbsplash_loop()
{
    uint64_t fb_count = bootinfo()->framebuffers_count;

    if (fb_count == 0)
        panic("No framebuffer available");

    struct framebuffer *fb = (struct framebuffer *)bootinfo()->framebuffers[0];

    if (fb->bpp != 32)
        panic("Only 32bpp framebuffer is supported");

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
