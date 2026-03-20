#include <cpu/irq.h>
#include <string.h>
#include <vsprintf.h>
#include <panic.h>

idt_t idt;

extern void *_default_interrupt_handlers[];

// TODO: This needs to come from our own GDT when we have it
#define KERNEL_CODE_SELECTOR 0x28

interrupt_frame_t *exception_handler(interrupt_frame_t *frame)
{

    panic("Exception: %s (interrupt number: %d, error code: %d, RIP: 0x%p)",
          (frame->interrupt_number < 32) ? exception_names[frame->interrupt_number] : "Unknown",
          frame->interrupt_number,
          frame->error_code,
          frame->rip);

    return frame;
}

void init_idt(void)
{
    memset(&idt, 0, sizeof(idt));

    idt.idtr.limit = sizeof(idt.entries) - 1;
    idt.idtr.base = (uint64_t)(uintptr_t)&idt.entries;

    for (int i = 0; i < 31; i++)
    {
        uint64_t handler_addr = (uint64_t)(uintptr_t)_default_interrupt_handlers[i];

        idt.entries[i].offset0 = (uint16_t)(handler_addr & 0xFFFF);
        idt.entries[i].selector = KERNEL_CODE_SELECTOR;
        idt.entries[i].ist = 0;
        idt.entries[i].type_attr = 0x8E; // Interrupt gate, present
        idt.entries[i].offset1 = (uint16_t)((handler_addr >> 16) & 0xFFFF);
        idt.entries[i].offset2 = (uint32_t)((handler_addr >> 32) & 0xFFFFFFFF);
        idt.entries[i].zero = 0;
    }

    // Load the IDT
    __asm__ volatile("lidt %0" : : "m"(idt.idtr));
}

void irq_enable(void)
{
    __asm__ volatile("sti");
}

void irq_disable(void)
{
    __asm__ volatile("cli");
}