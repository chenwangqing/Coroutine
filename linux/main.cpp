
#include <iostream>
#include <string>
#include "Coroutine.h"

extern void Sleep(uint32_t time);
extern void RunTask(void *(*func)(void *arg), void *arg);
extern void PrintMemory(void);

Coroutine_Semaphore sem1;
Coroutine_Mailbox   mail1;
Coroutine_Mutex     lock;
Coroutine_Channel   ch1;

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
        printf("[%llu][1][%llu/%d]i = %d %s\n", Coroutine.GetMillisecond(), ts, ms, i++, str.c_str());
#if 01
        char *buf = (char *)Coroutine.Malloc(str.size() + 1, __FILE__, __LINE__);
        memcpy(buf, str.c_str(), str.size() + 1);
        if (!Coroutine.SendMail(mail1, i & 0xFF, (uint64_t)buf, str.size() + 1, 1000))
            Coroutine.Free(buf, __FILE__, __LINE__);
#endif
        Sleep(10);
        Coroutine.PrintInfo(str_buf, 4 * 1024);
        printf("%s", str_buf);
    }
    return;
}

void Task2(void *obj)
{
    int i = 0;
    printf("start: %llx", Coroutine.GetMillisecond());
    while (true) {
        Coroutine.FeedDog(30 * 1000);
        std::string str = "hello";
        int         ms  = (rand() % 250) + 250;
        uint64_t    ts  = Coroutine.GetMillisecond();
        Coroutine.YieldDelay(ms);
        ts = Coroutine.GetMillisecond() - ts;
        str += std::to_string(i);
        printf("[%llu][2][%llu/%d]i = %d %s\n", Coroutine.GetMillisecond(), ts, ms, i++, str.c_str());
        Coroutine.GiveSemaphore(sem1, 1);
        Sleep(2);
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
        Sleep(1);
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
        auto data = Coroutine.ReceiveMail(mail1, 0xFF, 100);
        if (data.data == 0 || data.isOk == false) continue;
        printf("[%llu][4]mail1 recv: %llu %.*s\n",
               Coroutine.GetMillisecond(),
               data.id,
               data.size,
               (char *)data.data);
        Coroutine.Free((void *)data.data, __FILE__, __LINE__);
        Sleep(2);
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

void Task6(void *obj)
{
    uint64_t ts  = Coroutine.GetMillisecond();
    int     *num = (int *)obj;
    for (int i = 0; i < 100; i++) {
        Coroutine.FeedDog(30 * 1000);
        Coroutine.LockMutex(lock, UINT32_MAX);
        (*num)++;
        Coroutine.LockMutex(lock, UINT32_MAX);
        Coroutine.YieldDelay(100);
        Coroutine.UnlockMutex(lock);
        (*num)++;
        Coroutine.UnlockMutex(lock);
        Coroutine.Yield();
    }
    Coroutine.LockMutex(lock, UINT32_MAX);
    ts = Coroutine.GetMillisecond() - ts;
    printf("[%llu][6]----------------------- num = %d %llu -----------------------\n", Coroutine.GetMillisecond(), *num, ts);
    Coroutine.UnlockMutex(lock);
    return;
}

static void *RUNTask(void *obj)
{
    while (true)
        Coroutine.RunTick();
    return nullptr;
}

static void TestTask(void *func)
{
    printf("a = %p", func);
    return;
}


volatile uint64_t g_count = 0;

static void Task7(void *obj)
{
    while (true) {
        g_count++;
        Coroutine.Yield();
    }
    return;
}

static void Task8(void *obj)
{
    uint64_t last_count = 0;
    while (true) {
        Coroutine.YieldDelay(1000);
        uint64_t t  = g_count;
        uint64_t tv = t - last_count;
        last_count  = t;
        printf("[%llu][7]count = %llu\n", Coroutine.GetMillisecond(), tv);
    }
}

static void Task_Channel1(void *obj)
{
    uint64_t num = 0;
    uint64_t now = Coroutine.GetMillisecond();
    while (true) {
        Coroutine.WriteChannel(ch1, num + 1, UINT32_MAX);
        Coroutine.ReadChannel(ch1, &num, UINT32_MAX);
        if (Coroutine.GetMillisecond() - now >= 1000) {
            now = Coroutine.GetMillisecond();
            printf("[%llu][ch]num = %llu\n", Coroutine.GetMillisecond(), num);
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

int main()
{
    setbuf(stdout,NULL);
    TestTask((void *)TestTask);
    extern const Coroutine_Inter *GetInter(void);
    auto                          inter = GetInter();
    Coroutine.SetInter(inter);

    for (size_t i = 0; i < inter->thread_count; i++)
        RunTask(RUNTask, nullptr);

    sem1    = Coroutine.CreateSemaphore("sem1", 0);
    mail1   = Coroutine.CreateMailbox("mail1", 1024);
    lock    = Coroutine.CreateMutex("lock");
    ch1     = Coroutine.CreateChannel("ch1", 0);
    int num = 0;

    Coroutine_TaskAttribute atr;
    memset(&atr, 0, sizeof(Coroutine_TaskAttribute));
    atr.co_idx     = -1;
    atr.stack_size = 1024 * 16;
    atr.pri        = TASK_PRI_NORMAL;

    Sleep(rand() % 1000);
    Coroutine.AddTask(Task1, nullptr, "Task1", &atr);
    Sleep(rand() % 1000);
    Coroutine.AddTask(Task2, nullptr, "Task2", &atr);
    Sleep(rand() % 1000);
    Coroutine.AddTask(Task3, nullptr, "Task3", &atr);
    Sleep(rand() % 1000);

    Coroutine.AddTask(Task4, nullptr, "Task4", &atr);
    Sleep(rand() % 1000);
    Coroutine.AddTask(Task5, &num, "Task5-1", &atr);
    Sleep(rand() % 1000);
    Coroutine.AddTask(Task6, &num, "Task5-2", &atr);

    Coroutine.AddTask(Task7, nullptr, "Task7", &atr);
    Coroutine.AddTask(Task8, nullptr, "Task8", &atr);

    Coroutine.AddTask(Task_Channel1, nullptr, "Channel1", &atr);
    Coroutine.AddTask(Task_Channel2, nullptr, "Channel2", &atr);
    while (true)
        Sleep(1000);
    return 0;
}
