
#include <iostream>
#include <string>
#include "Coroutine.h"
#include "Coroutine.hpp"
#include "nplog.h"
#include <unistd.h>
#include <functional>

NPLOG_DEFINE(main, NPLOG_LEVEL_DEBUG);
#undef LOG_PRINTF_Array
#undef LOG_DEBUG
#undef LOG_PRINTF
#undef LOG_WARNING
#undef LOG_ERROR
#define LOG_PRINTF_Array(level, data, len) LOG_TAG_Array(main, level, data, len)
#define LOG_DEBUG(format, ...)             LOG_TAG_DBUG(main, format, ##__VA_ARGS__)
#define LOG_PRINTF(format, ...)            LOG_TAG_INFO(main, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...)           LOG_TAG_WARN(main, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)             LOG_TAG_FAIL(main, format, ##__VA_ARGS__)


extern void Sleep(uint32_t time);
extern void RunTask(void *(*func)(void *arg), void *arg);
extern void PrintMemory(void);

Coroutine_Semaphore sem1;
Coroutine_Mailbox   mail1;
Coroutine_Mutex     lock;
Coroutine_Channel   ch1;
Coroutine_Channel   ch2;

static uint64_t Task1_func1(uint32_t timeout)
{
    uint64_t ts = Coroutine.GetMillisecond();
    Coroutine.YieldDelay(timeout);
    ts = Coroutine.GetMillisecond() - ts;
    return ts;
}

void Task1(void *obj)
{
    int   i       = 0;
    char *str_buf = new char[4 * 1024];
    while (true) {
        Coroutine.FeedDog(30 * 1000);
        PrintMemory();
        volatile int a1  = 12345678;
        std::string  str = "hello";
        volatile int a2  = 72345671;
        int          ms  = (rand() % 750) + 250;
        uint64_t     ts  = Task1_func1(ms);
        str += std::to_string(i);
        a1++;
        a2++;
        LOG_PRINTF("[1][%llu/%d]i = %d %s\n", ts, ms, i++, str.c_str());
#if 01
        char *buf = (char *)Coroutine.Malloc(str.size() + 1, __FILE__, __LINE__);
        memcpy(buf, str.c_str(), str.size() + 1);
        if (!Coroutine.SendMail(mail1, (i & 0xFF) | 0x01, (uint64_t)buf, str.size() + 1, 1000))
            Coroutine.Free(buf, __FILE__, __LINE__);
#endif
        Sleep(10);
        Coroutine.PrintInfo(str_buf, 4 * 1024);
        puts(str_buf);
    }
    return;
}

void Task2(void *obj)
{
    int i = 0;
    while (true) {
        Coroutine.FeedDog(30 * 1000);
        std::string str = "hello";
        int         ms  = (rand() % 250) + 250;
        uint64_t    ts  = Coroutine.GetMillisecond();
        Coroutine.YieldDelay(ms);
        ts = Coroutine.GetMillisecond() - ts;
        str += std::to_string(i);
        LOG_PRINTF("[%llu][2][%llu/%d]i = %d %s\n", Coroutine.GetMillisecond(), ts, ms, i++, str.c_str());
        Coroutine.GiveSemaphore(sem1, 1);
        Sleep(5);
    }
    return;
}

void Task3(void *obj)
{
    auto func = [](void *obj) -> void * {
        int     *a   = (int *)obj;
        uint64_t now = Coroutine.GetMillisecond();
        Sleep(2);
        a[1] = (rand() % 150) + 0;
        Coroutine.YieldDelay(a[1]);
        a[0] = Coroutine.GetMillisecond() - now;
        return a;
    };
    int            *a  = new int[2];
    Coroutine_ASync re = nullptr;
    while (true) {
        Coroutine.FeedDog(30 * 1000);
        if (!Coroutine.WaitSemaphore(sem1, 1, 100))
            continue;
        printf("[%llu][3]sem1 signal\n", Coroutine.GetMillisecond());
        Sleep(4);
        if (re && Coroutine.AsyncWait(re, 100)) {
            int *b = (int *)Coroutine.AsyncGetResultAndDelete(re);
            printf("[%llu][3]async result: %d/%d\n", Coroutine.GetMillisecond(), b[0], b[1]);
            re = nullptr;
        }
        if (!re)
            re = Coroutine.Async(func, a, 0);
    }
    return;
}

void Task4(void *obj)
{
    while (true) {
        Coroutine.FeedDog(30 * 1000);
        int ms = (rand() % 250) + 250;
        Coroutine.YieldDelay(ms);
        while (true) {
            auto data = Coroutine.ReceiveMail(mail1, 0xFF, 100);
            if (data.data == 0 || data.isOk == false) break;
            printf("[%llu][4]mail1 recv: %llu %.*s\n",
                   Coroutine.GetMillisecond(),
                   data.id,
                   data.size,
                   (char *)data.data);
            Coroutine.Free((void *)data.data, __FILE__, __LINE__);
            Sleep(5);
        }
    }
    return;
}

void Task5(void *obj)
{
    uint64_t ts  = Coroutine.GetMillisecond();
    int     *num = (int *)obj;
    for (int i = 0; i < 100; i++) {
        Coroutine.FeedDog(30 * 1000);
        Coroutine.LockMutex(lock, UINT32_MAX);
        (*num)++;
        Coroutine.YieldDelay(100);
        (*num)++;
        Coroutine.UnlockMutex(lock);
        Coroutine.Yield();
    }
    Coroutine.LockMutex(lock, UINT32_MAX);
    ts = Coroutine.GetMillisecond() - ts;
    printf("[%llu][5]----------------------- num = %d %llu -----------------------\n", Coroutine.GetMillisecond(), *num, ts);
    Coroutine.UnlockMutex(lock);
    return;
}

static void *RUNTask(void *obj)
{
    while (true)
        Coroutine.RunTick(1000);
    return nullptr;
}

volatile uint64_t g_count1 = 0;
volatile uint64_t g_count2 = 0;

static void Task7(void *obj)
{
    while (true) {
        g_count1++;
        Coroutine.Yield();
    }
    return;
}

void *RUNTask_TestCount(void *obj)
{
    while (true) {
        g_count2++;
        sched_yield();
    }
}

void PrintCount(void)
{
    static uint64_t last_count1 = 0;
    static uint64_t last_count2 = 0;
    uint64_t        t           = g_count1;
    uint64_t        tv1         = t - last_count1;
    last_count1                 = t;
    t                           = g_count2;
    uint64_t tv2                = t - last_count2;
    last_count2                 = t;
    printf("[%llu][7]count1 = %llu count2 = %llu\n", Coroutine.GetMillisecond(), tv1, tv2);
}

static void Task_Channel1(void *obj)
{
    uint64_t num   = 0;
    uint64_t now   = Coroutine.GetMillisecond();
    uint64_t total = 0, count = 0;
    while (true) {
        Coroutine.WriteChannel(ch1, num + 1, UINT32_MAX);
        Coroutine.ReadChannel(ch1, &num, UINT32_MAX);
        if (Coroutine.GetMillisecond() - now >= 1000) {
            now = Coroutine.GetMillisecond();
            total += num;
            count++;
            printf("[%llu][ch]num = %llu Avg = %llu\n", Coroutine.GetMillisecond(), (uint64_t)num, total / count);
            num = 0;
        }
    }
}

static void Task_Channel2(void *obj)
{
    uint64_t num = 0;
    while (true) {
        Coroutine.ReadChannel(ch1, &num, UINT32_MAX);
        Coroutine.WriteChannel(ch1, num + 1, UINT32_MAX);
    }
}


static void Task_Channel3(void *obj)
{
    uint64_t num   = 0;
    uint64_t now   = Coroutine.GetMillisecond();
    uint64_t total = 0, count = 0;
    while (true) {
        Coroutine.WriteChannel(ch2, num + 1, UINT32_MAX);
        Coroutine.ReadChannel(ch2, &num, UINT32_MAX);
        if (Coroutine.GetMillisecond() - now >= 1000) {
            now = Coroutine.GetMillisecond();
            total += num;
            count++;
            printf("[%llu][ch2]num = %llu Avg = %llu\n", Coroutine.GetMillisecond(), (uint64_t)num, total / count);
            num = 0;
        }
    }
}

static void Task_Channel4(void *obj)
{
    uint64_t num = 0;
    while (true) {
        Coroutine.ReadChannel(ch2, &num, UINT32_MAX);
        Coroutine.WriteChannel(ch2, num + 1, UINT32_MAX);
    }
}


void *RUNTask_Test(void *obj)
{
    int         i   = 0;
    std::string str = "hello";
    while (true) {
        Sleep(rand() % 1000);
        str       = "hello - " + std::to_string(i);
        char *buf = (char *)Coroutine.Malloc(str.size() + 1, __FILE__, __LINE__);
        memcpy(buf, str.c_str(), str.size() + 1);
        if (!Coroutine.SendMail(mail1, (i & 0xFF) | 0x01, (uint64_t)buf, str.size() + 1, 1000))
            Coroutine.Free(buf, __FILE__, __LINE__);
        i++;
    }
}

COROUTINE_INIT_REG_TASK("Task1", Task1, NULL, 16 << 10);

static void *RUNTask_Init(void *obj)
{
    sem1  = Coroutine.CreateSemaphore("sem1", 0);
    mail1 = Coroutine.CreateMailbox("mail1", 1024);
    lock  = Coroutine.CreateMutex("lock");
    ch1   = Coroutine.CreateChannel("ch1", 0);
    ch2   = Coroutine.CreateChannel("ch2", 0);


    static int num        = 0;
    int        stack_size = 1024 * 32;

    GO[stack_size]()
    {
        LOG_DEBUG("stack_size: %d\n", stack_size);
    };

    Sleep(rand() % 1000);
    Coroutine.AddTask(Task2, nullptr, TASK_PRI_NORMAL, stack_size, "Task2", nullptr);
    Sleep(rand() % 1000);
    Coroutine.AddTask(Task3, nullptr, TASK_PRI_NORMAL, stack_size, "Task3", nullptr);
    Sleep(rand() % 1000);

    Coroutine.AddTask(Task4, nullptr, TASK_PRI_NORMAL, stack_size, "Task4", nullptr);
    Sleep(rand() % 1000);
    Coroutine.AddTask(Task5, &num, TASK_PRI_NORMAL, stack_size, "Task5-1", nullptr);
    Coroutine.AddTask(Task5, &num, TASK_PRI_NORMAL, stack_size, "Task5-2", nullptr);

    Coroutine.AddTask(Task7, nullptr, TASK_PRI_NORMAL, stack_size, "Task7", nullptr);

    Coroutine.AddTask(Task_Channel1, nullptr, TASK_PRI_NORMAL, stack_size, "Channel1", nullptr);
    Coroutine.AddTask(Task_Channel2, nullptr, TASK_PRI_NORMAL, stack_size, "Channel2", nullptr);
    Coroutine.AddTask(Task_Channel3, nullptr, TASK_PRI_NORMAL, stack_size, "Channel3", nullptr);
    Coroutine.AddTask(Task_Channel4, nullptr, TASK_PRI_NORMAL, stack_size, "Channel4", nullptr);
    return nullptr;
}

int main()
{
    setbuf(stdout, NULL);

    extern const Coroutine_Inter *GetInter(void);
    auto                          inter = GetInter();
    Coroutine.SetInter(inter);

    for (size_t i = 0; i < inter->thread_count; i++)
        RunTask(RUNTask, nullptr);

    RunTask(RUNTask_Init, nullptr);   // 初始化任务

    RunTask(RUNTask_Test, nullptr);
    // RunTask(RUNTask_TestCount, nullptr);

    struct timespec tv;
    tv.tv_sec    = 0;
    tv.tv_nsec   = 1000000;   // 1毫秒
    uint64_t now = Coroutine.GetMillisecond();
    while (true) {
        nanosleep(&tv, NULL);
        Coroutine.MillisecondInterrupt();
        if (Coroutine.GetMillisecond() - now >= 1000) {
            now = Coroutine.GetMillisecond();
            PrintCount();
        }
    }
    return 0;
}
