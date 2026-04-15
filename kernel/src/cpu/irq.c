#include <string.h>
#include <vsprintf.h>
#include <panic.h>
#include <cpu/irq.h>
#include <cpu/cpu.h>
#include <cpu/lapic.h>
#include <bootinfo/bootinfo.h>
#include <task.h>

idt_t idt;

extern void *_default_interrupt_handlers[];

void exception_handler(struct interrupt_frame_t *frame)
{
    // Save these because they won't be valid after schedule is called and frame is switched
    uint64_t interrupt_number = frame->interrupt_number;
    uint64_t error_code = frame->error_code;

    switch (interrupt_number)
    {
    case IRQ_SOFT_SLEEP_NUMBER:
        schedule(frame, SCHED_REASON_YIELD);
        break;
    case IRQ_APIC_TIMER_NUMBER:
        schedule(frame, SCHED_REASON_TIMER);
        lapic_send_eoi();
        break;
    default:
        panic("Exception: %s (interrupt number: %d, error code: %d, RIP: 0x%p)\n",
              (interrupt_number < 32) ? exception_names[interrupt_number] : "Unknown",
              interrupt_number,
              error_code,
              frame->rip);
    }
}

void init_idt(void)
{
    memset(&idt, 0, sizeof(idt));

    idt.idtr.limit = sizeof(idt.entries) - 1;
    idt.idtr.base = (uint64_t)(uintptr_t)&idt.entries;

    for (int i = 0; i < 66; i++)
    {
        uint64_t handler_addr = (uint64_t)(uintptr_t)_default_interrupt_handlers[i];

        idt.entries[i].offset0 = (uint16_t)(handler_addr & 0xFFFF);
        idt.entries[i].selector = bootinfo()->code_segment;
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
    __asm__ volatile("sti" ::: "memory");
}

void irq_disable(void)
{
    __asm__ volatile("cli" ::: "memory");
}