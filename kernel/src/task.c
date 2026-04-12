#include <task.h>
#include <string.h>
#include <vsprintf.h>
#include <mm/earlyalloc.h>
#include <bootinfo/bootinfo.h>
#include <panic.h>
#include <stdbool.h>
#include <cpu/irq.h>

#define TASKS_MAX 254
#define TASK_STACK_SIZE 8192

// Use a static list of tasks for now
struct task_t *tasks[TASKS_MAX];
struct task_t *current_task;
struct task_t task0;

void task_init()
{
    // Create boot task
    memset(&task0, 0, sizeof(struct task_t));
    memcpy(task0.name, "boot", 4);
    task0.slice_count = 1;
    task0.status = STATUS_RUNNABLE;
    tasks[0] = &task0;
    current_task = &task0;
}

void task_create(struct task_t *t, char *name, void *entry)
{

    memset(t, 0, sizeof(struct task_t));
    memcpy(t->name, name, 4);

    t->status = STATUS_RUNNABLE;

    uint8_t *stack = earlyalloc_alloc(TASK_STACK_SIZE, 4096);
    memset(stack, 0, TASK_STACK_SIZE);

    t->frame.rip = (uintptr_t)entry;
    t->frame.rsp = (uint64_t)stack + 8192;
    t->frame.cs = bootinfo()->code_segment;
    t->frame.ss = bootinfo()->data_segment;
    t->frame.rflags = 0x202;

    // Find next free slot
    int free_idx = -1;

    for (int i = 0; i < TASKS_MAX; i++)
    {
        if (!tasks[i])
        {
            free_idx = i;
            break;
        }
    }

    if (free_idx < 0)
    {
        panic("No free task slots available\n");
    }

    tasks[free_idx] = t;

    kprintf("[task]: New task created at slot %d\n", free_idx);
}

void _slice_accounting()
{
    if (current_task)
        current_task->slice_count++;

    for (int i = 0; i < TASKS_MAX; i++)
    {
        if (!tasks[i])
            continue;

        if (tasks[i]->status == STATUS_BLOCKED)
        {
            if (tasks[i]->blocked > 0)
                tasks[i]->blocked--;

            if (tasks[i]->blocked == 0)
            {
                tasks[i]->status = STATUS_RUNNABLE;
            }
        }
    }
}

struct task_t *_get_next_task()
{
    struct task_t *next = NULL;
    uint64_t min = 0;

    for (int i = 0; i < TASKS_MAX; i++)
    {
        if (!tasks[i])
            continue;

        if (tasks[i]->status != STATUS_RUNNABLE)
            continue;

        if (!next || tasks[i]->slice_count < min)
        {
            next = tasks[i];
            min = tasks[i]->slice_count;
        }
    }

    if (next)
        return next;

    if (current_task && current_task->status == STATUS_RUNNABLE)
        return current_task;

    if (tasks[0] && tasks[0]->status == STATUS_RUNNABLE)
        return tasks[0];

    return current_task;
}

void task_sleep(uint64_t ticks)
{
    if (ticks)
    {
        irq_disable();
        current_task->status = STATUS_BLOCKED;
        current_task->blocked = ticks;
        irq_enable();
    }

    // Call soft sleep interrupt
    asm volatile("int $0x41" ::: "memory");
}

void schedule(struct interrupt_frame_t *frame, sched_reason_t reason)
{
    if (current_task)
    {
        // Save state of current task
        current_task->frame = *frame;
        current_task->frame.interrupt_number = 0;
        current_task->frame.error_code = 0;

        // Switch task to RUNNABLE
        if (current_task->status == STATUS_RUNNING)
            current_task->status = STATUS_RUNNABLE;
    }

    if (reason == SCHED_REASON_TIMER)
        _slice_accounting();

    // Find next task to run
    struct task_t *task = _get_next_task();

    if (!task)
        panic("No runnable task available");

    // Restore state of next task
    *frame = task->frame;

    // NOTE: inspecting frame.{interrupt_number, error_code} is not
    //       valid once we return.

    // Set new status to RUNNING
    task->status = STATUS_RUNNING;

    // Set global current_task
    current_task = task;
}