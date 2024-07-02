
#include <iostream>
#include <string>
#include "Coroutine.h"

extern void Sleep(uint32_t time);
extern void RunTask(void *(*func)(void *arg), void *arg);
extern void PrintMemory(void);

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

void Task1(void *obj)
{
    int   i       = 0;
    char *str_buf = new char[4 * 1024];
    while (true) {
        Coroutine.FeedDog(30 * 1000);
        PrintMemory();
        std::string str = "hello";
        int         ms  = (rand() % 750) + 250;
        uint64_t    ts  = Task1_func1(ms);
        str += std::to_string(i);
        printf("[%llu][1][%llu/%d]i = %d %s\n", Coroutine.GetMillisecond(), ts, ms, i++, str.c_str());
#if 0
        Coroutine_MailData *data = Coroutine.MakeMessage(i & 0xFF, str.c_str(), str.size(), 1000);
        if (!Coroutine.SendMail(mail1, data))
            Coroutine.DeleteMessage(data);
#endif
        //  Sleep(10);
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
        // Sleep(2);
    }
    return;
}

void Task3(void *obj)
{
    auto func = [](void *obj) -> void * {
        int     *a   = (int *)obj;
        uint64_t now = Coroutine.GetMillisecond();
        // Sleep(2);
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
        // Sleep(1);
        if (re && Coroutine.AsyncWait(re, 100)) {
            int *b = (int *)Coroutine.AsyncGetResultAndDelete(re);
            printf("[%llu][3]async result: %d/%d\n", Coroutine.GetMillisecond(), b[0], b[1]);
            re = nullptr;
        }
        if (!re)
            re = Coroutine.Async(func, a);
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
        if (data == NULL) continue;
        printf("[%llu][4]mail1 recv: %llu %.*s\n",
               Coroutine.GetMillisecond(),
               data->eventId,
               data->size,
               (char *)data->data);
        Coroutine.DeleteMessage(data);
        // Sleep(2);
    }
    return;
}

void Task5(void *obj)
{
    uint64_t ts  = Coroutine.GetMillisecond();
    int     *num = (int *)obj;
    for (int i = 0; i < 100; i++) {
        Coroutine.FeedDog(30 * 1000);
        while (!Coroutine.LockMutex(lock, 1000000));
        (*num)++;
        Coroutine.YieldDelay(100);
        (*num)++;
        Coroutine.UnlockMutex(lock);
        Coroutine.Yield();
    }
    while (!Coroutine.LockMutex(lock, 1000000));
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
        while (!Coroutine.LockMutex(lock, 1000000));
        (*num)++;
        while (!Coroutine.LockMutex(lock, 1000000));
        Coroutine.YieldDelay(100);
        Coroutine.UnlockMutex(lock);
        (*num)++;
        Coroutine.UnlockMutex(lock);
        Coroutine.Yield();
    }
    while (!Coroutine.LockMutex(lock, 1000000));
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

int main()
{
    extern const Coroutine_Inter *GetInter(void);
    auto                          inter = GetInter();
    Coroutine.SetInter(inter);

    for (size_t i = 0; i < inter->thread_count; i++)
        RunTask(RUNTask, nullptr);

    sem1    = Coroutine.CreateSemaphore("sem1", 0);
    mail1   = Coroutine.CreateMailbox("mail1", 1024);
    lock    = Coroutine.CreateMutex("lock");
    int num = 0;

    Sleep(rand() % 1000);
    Coroutine.AddTask(-1, Task1, nullptr, TASK_PRI_NORMAL, "Task1");
    Sleep(rand() % 1000);
    Coroutine.AddTask(-1, Task2, nullptr, TASK_PRI_NORMAL, "Task2");
    Sleep(rand() % 1000);
    Coroutine.AddTask(-1, Task3, nullptr, TASK_PRI_NORMAL, "Task3");
    Sleep(rand() % 1000);
    Coroutine.AddTask(-1, Task4, nullptr, TASK_PRI_NORMAL, "Task4");
    Sleep(rand() % 1000);
    Coroutine.AddTask(-1, Task5, &num, TASK_PRI_LOWEST, "Task5-1");
    Sleep(rand() % 1000);
    Coroutine.AddTask(-1, Task6, &num, TASK_PRI_NORMAL, "Task5-2");
    while (true)
        Sleep(1000);
    return 0;
}
