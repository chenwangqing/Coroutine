
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include <stdlib.h>
#include <stdio.h>
#include "hc32f10x.h"
#include "nplog.h"
#include "Coroutine.h"

extern void         PrintMemory(void);
Coroutine_Semaphore sem1;
Coroutine_Mailbox   mail1;
Coroutine_Mutex     lock;

static uint64_t Task1_func1(uint32_t timeout)
{
    uint64_t ts = Coroutine.GetMillisecond();
    Coroutine.YieldDelay(timeout);
    ts = Coroutine.GetMillisecond() - ts;
    return ts;
}

static void Task1(void *arg)
{
    int i = 0;
    while (true) {
        int      ms = (rand() % 1010) + 10;
        uint64_t ts = Task1_func1(ms);
        LOG_DEBUG("[1][%llu/%d]i = %d", ts, ms, i++);
        const int mlen = 1024;
        char *    str  = Coroutine.Malloc(mlen, __FILE__, __LINE__);
        if (str) {
            int n = Coroutine.PrintInfo(str, mlen);
            UART_Write(0, str, n);
            Coroutine.Free(str, __FILE__, __LINE__);
        }
        PrintMemory();
    }
    // return;
}

void Task2(void *obj)
{
    int  i = 0;
    char str[32];
    while (true) {
        Coroutine.FeedDog(30 * 1000);
        int      s  = snprintf(str, sizeof(str), "Task2:");
        int      ms = (rand() % 250) + 250;
        uint64_t ts = Coroutine.GetMillisecond();
        Coroutine.YieldDelay(ms);
        ts = Coroutine.GetMillisecond() - ts;
        snprintf(str + s, sizeof(str) - s, "%d", i);
        Coroutine_MailData *data = Coroutine.MakeMessage(i & 0xFF, str, strlen(str), 1000);
        LOG_DEBUG("[2][%llu/%d]i = %d %s", ts, ms, i++, str);
        Coroutine.GiveSemaphore(sem1, 1);
        if (!Coroutine.SendMail(mail1, data))
            Coroutine.DeleteMessage(data);
    }
    return;
}

static void *Task3_Func(void *obj)
{
    int *    a   = (int *)obj;
    uint64_t now = Coroutine.GetMillisecond();
    a[1]         = (rand() % 150) + 0;
    Coroutine.YieldDelay(a[1]);
    a[0] = Coroutine.GetMillisecond() - now;
    return a;
}

void Task3(void *obj)
{
    static int      a[2] = {0, 0};
    Coroutine_ASync re   = nullptr;
    while (true) {
        Coroutine.FeedDog(30 * 1000);
        if (!Coroutine.WaitSemaphore(sem1, 1, 100))
            continue;
        LOG_DEBUG("[3]sem1 signal");
        if (re && Coroutine.AsyncWait(re, 100)) {
            int *b = (int *)Coroutine.AsyncGetResultAndDelete(re);
            LOG_DEBUG("[3]async result: %d/%d", b[0], b[1]);
            re = nullptr;
        }
        if (!re)
            re = Coroutine.Async(Task3_Func, a);
    }
    return;
}

void Task4(void *obj)
{
    while (true) {
        Coroutine.FeedDog(30 * 1000);
        int ms = (rand() % 250) + 250;
        Coroutine.YieldDelay(ms);
        Coroutine_MailData *data = Coroutine.ReceiveMail(mail1, 0xFF, 100);
        if (data == NULL) continue;
        LOG_DEBUG("[4]mail1 recv: %llu %.*s",
                  data->eventId,
                  data->size,
                  (char *)data->data);
        Coroutine.DeleteMessage(data);
    }
    return;
}


void Task5(void *obj)
{
    uint64_t ts  = Coroutine.GetMillisecond();
    int *    num = (int *)obj;
    for (int i = 0; i < 100; i++) {
        Coroutine.FeedDog(30 * 1000);
        while (!Coroutine.LockMutex(lock, 1000000))
            ;
        (*num)++;
        Coroutine.YieldDelay(100);
        (*num)++;
        Coroutine.UnlockMutex(lock);
        Coroutine.Yield();
    }
    while (!Coroutine.LockMutex(lock, 1000000))
        ;
    ts = Coroutine.GetMillisecond() - ts;
    LOG_DEBUG("[5]----------------------- num = %d %llu -----------------------\n",  *num, ts);
    Coroutine.UnlockMutex(lock);
    return;
}

void Task6(void *obj)
{
    uint64_t ts  = Coroutine.GetMillisecond();
    int *    num = (int *)obj;
    for (int i = 0; i < 100; i++) {
        Coroutine.FeedDog(30 * 1000);
        while (!Coroutine.LockMutex(lock, 1000000))
            ;
        (*num)++;
        while (!Coroutine.LockMutex(lock, 1000000))
            ;
        Coroutine.YieldDelay(100);
        Coroutine.UnlockMutex(lock);
        (*num)++;
        Coroutine.UnlockMutex(lock);
        Coroutine.Yield();
    }
    while (!Coroutine.LockMutex(lock, 1000000))
        ;
    ts = Coroutine.GetMillisecond() - ts;
    LOG_DEBUG("[6]----------------------- num = %d %llu -----------------------\n",  *num, ts);
    Coroutine.UnlockMutex(lock);
    return;
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    // 启动中断
    __enable_irq();
    // 时钟初始化
    SystemCoreClockUpdate();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                               RCC_APB2Periph_GPIOB |
                               RCC_APB2Periph_GPIOC |
                               RCC_APB2Periph_GPIOD |
                               RCC_APB2Periph_GPIOE,
                           ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    // 初始化定时器
    SysTick_Config(SystemCoreClock / 1000);
    NVIC_EnableIRQ(SysTick_IRQn);
    // 初始化串口
    UART_Init();

    extern const Coroutine_Inter *GetInter(void);
    const Coroutine_Inter *       inter = GetInter();
    Coroutine.SetInter(inter);
    // 初始化信号
    sem1    = Coroutine.CreateSemaphore("sem1", 0);
    mail1   = Coroutine.CreateMailbox("mail1", 1024);
    lock    = Coroutine.CreateMutex("lock");
    int num = 0;

    // 添加任务
    Coroutine.AddTask(-1, Task1, nullptr, TASK_PRI_NORMAL, "Task1");
    Coroutine.AddTask(-1, Task2, nullptr, TASK_PRI_NORMAL, "Task2");
    Coroutine.AddTask(-1, Task3, nullptr, TASK_PRI_NORMAL, "Task3");
    Coroutine.AddTask(-1, Task4, nullptr, TASK_PRI_NORMAL, "Task4");
    Coroutine.AddTask(-1, Task5, &num, TASK_PRI_LOWEST, "Task5-1");
    Coroutine.AddTask(-1, Task6, &num, TASK_PRI_NORMAL, "Task5-2");

    while (true)
        Coroutine.RunTick();
    // return 0;
}
