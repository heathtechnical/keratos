#include <task.h>
#include <vsprintf.h>

void tocker_loop()
{
    while (1)
    {
        kprintf("[task/tocker] tock\n");
        task_sleep(2000);
    }
}