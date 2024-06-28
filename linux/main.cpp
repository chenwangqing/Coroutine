
#include <iostream>
#include <string>
#include "Coroutine.h"

extern void Sleep(uint32_t time);
extern void RunTask(void *(*func)(void *arg), void *arg);
extern void PrintMemory(void);

Coroutine_Semaphore sem1;
Coroutine_Mailbox   mail1;

static uint64_t Task1_func1(uint32_t timeout)
{
    uint64_t ts = Coroutine.GetMillisecond();
    Coroutine.YieldDelay(1000);
    ts = Coroutine.GetMillisecond() - ts;
    return ts;
}

void Task1(void *obj)
{
    int   i       = 0;
    char *str_buf = new char[4 * 1024];
    while (true) {
        PrintMemory();
        std::string str = "hello";
        uint64_t    ts  = Task1_func1(1000);
        str += std::to_string(i);
        printf("[%llu][1][%llu]i = %d %s\n", Coroutine.GetMillisecond(), ts, i++, str.c_str());
#if 1
        Coroutine_MailData *data = Coroutine.MakeMessage(i & 0xFF, str.c_str(), str.size(), 1000);
        if (!Coroutine.SendMail(mail1, data))
            Coroutine.DeleteMessage(data);
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
        std::string str = "hello";
        uint64_t    ts  = Coroutine.GetMillisecond();
        Coroutine.YieldDelay(500);
        ts = Coroutine.GetMillisecond() - ts;
        str += std::to_string(i);
        printf("[%llu][2][%llu]i = %d %s\n", Coroutine.GetMillisecond(), ts, i++, str.c_str());
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
        Coroutine.YieldDelay(100);
        (*a) = Coroutine.GetMillisecond() - now;
        return a;
    };
    int             *a  = new int(0);
    Coroutine_ASync *re = nullptr;
    while (true) {
        if (!Coroutine.WaitSemaphore(sem1, 1, 100))
            continue;
        printf("[%llu][3]sem1 signal\n", Coroutine.GetMillisecond());
        Sleep(1);
        if (re && Coroutine.ASyncWait(re, 100)) {
            int *b = (int *)Coroutine.ASyncGetResultAndDelete(&re);
            printf("[%llu][3]async result: %d\n", Coroutine.GetMillisecond(), *b);
        }
        if (!re)
            re = Coroutine.ASync(func, a);
    }
    return;
}

void Task4(void *obj)
{
    while (true) {
        Coroutine.YieldDelay(500);
        auto data = Coroutine.ReceiveMail(mail1, 0xFF, 100);
        if (data == NULL) continue;
        printf("[%llu][4]mail1 recv: %llu %.*s\n",
               Coroutine.GetMillisecond(),
               data->eventId,
               data->size,
               (char *)data->data);
        Coroutine.DeleteMessage(data);
        Sleep(2);
    }
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

    sem1  = Coroutine.CreateSemaphore("sem1", 0);
    mail1 = Coroutine.CreateMailbox("mail1", 1024);

    Coroutine.AddTask(-1, Task1, nullptr, "Task1");
    Coroutine.AddTask(-1, Task2, nullptr, "Task2");
    Coroutine.AddTask(-1, Task3, nullptr, "Task3");
    Coroutine.AddTask(-1, Task4, nullptr, "Task4");

    for (size_t i = 0; i < inter->thread_count; i++)
        RunTask(RUNTask, nullptr);
    while (true)
        Sleep(1000);
    return 0;
}