#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdbool.h>
#include <cpu/irq.h>

typedef enum
{
    STATUS_BLOCKED,
    STATUS_RUNNABLE,
    STATUS_RUNNING,
} task_status_t;

struct task_t
{
    char name[255];
    task_status_t status;
    struct interrupt_frame_t frame;
    uint64_t slice_count;
    uint64_t blocked;
};

extern struct task_t *current_task;

typedef enum
{
    SCHED_REASON_TIMER,
    SCHED_REASON_YIELD, // soft sleep / voluntary yield
} sched_reason_t;

void task_create(struct task_t *t, char *name, void *entry);
struct task_t *_get_next_task();
void task_init(void);
void task_sleep(uint64_t ticks);
void schedule(struct interrupt_frame_t *frame, sched_reason_t reason);
#endif
