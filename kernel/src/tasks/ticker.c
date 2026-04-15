#include <task.h>
#include <vsprintf.h>

void ticker_loop()
{
    while (1)
    {
        kprintf("[task/ticker] tick...\n");
        task_sleep(1000);
    }
}