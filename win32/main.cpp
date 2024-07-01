// Coroutine.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "Coroutine.h"
#include <string>
#include <windows.h>

#undef CreateSemaphore
#undef CreateMutex
#undef Yield

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

void Task1(void *obj)
{
    int   i       = 0;
    char *str_buf = new char[4 * 1024];
    while (true) {
        PrintMemory();
        std::string str = "hello";
        int         ms  = (rand() % 750) + 250;
        uint64_t    ts  = Task1_func1(ms);
        str += std::to_string(i);
        printf("[%llu][1][%llu/%d]i = %d %s\n", Coroutine.GetMillisecond(), ts, ms, i++, str.c_str());
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
        Coroutine.YieldDelay(100);
        (*a) = Coroutine.GetMillisecond() - now;
        return a;
    };
    int            *a  = new int(0);
    Coroutine_ASync re = nullptr;
    while (true) {
        if (!Coroutine.WaitSemaphore(sem1, 1, 100))
            continue;
        printf("[%llu][3]sem1 signal\n", Coroutine.GetMillisecond());
        Sleep(1);
        if (re && Coroutine.AsyncWait(re, 100)) {
            int *b = (int *)Coroutine.AsyncGetResultAndDelete(re);
            printf("[%llu][3]async result: %d\n", Coroutine.GetMillisecond(), *b);
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
        Sleep(2);
    }
    return;
}

void Task5(void *obj)
{
    int *num = (int *)obj;
    for (int i = 0; i < 100; i++) {
        while (!Coroutine.LockMutex(lock, 1000000));
        (*num)++;
        Coroutine.YieldDelay(100);
        (*num)++;
        Coroutine.UnlockMutex(lock);
    }
    while (!Coroutine.LockMutex(lock, 1000000));
    printf("[%llu][5]num = %d\n", Coroutine.GetMillisecond(), *num);
    Coroutine.UnlockMutex(lock);
    return;
}

void Task6(void *obj)
{
    int *num = (int *)obj;
    for (int i = 0; i < 100; i++) {
        while (!Coroutine.LockMutex(lock, 1000000));
        (*num)++;
        Coroutine.YieldDelay(100);
        (*num)++;
        Coroutine.UnlockMutex(lock);
    }
    while (!Coroutine.LockMutex(lock, 1000000));
    printf("[%llu][6]num = %d\n", Coroutine.GetMillisecond(), *num);
    Coroutine.UnlockMutex(lock);
    return;
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
    while (true)
        Coroutine.RunTick();
    return 0;
}

int main()
{
    extern const Coroutine_Inter *GetInter(void);
    auto                          inter = GetInter();
    Coroutine.SetInter(inter);

    sem1    = Coroutine.CreateSemaphore("sem1", 0);
    mail1   = Coroutine.CreateMailbox("mail1", 1024);
    lock    = Coroutine.CreateMutex("lock");
    int num = 0;

    Coroutine.AddTask(-1, Task1, nullptr, "Task1");
    Coroutine.AddTask(-1, Task2, nullptr, "Task2");
    Coroutine.AddTask(-1, Task3, nullptr, "Task3");
    Coroutine.AddTask(-1, Task4, nullptr, "Task4");
    Coroutine.AddTask(-1, Task5, &num, "Task5-1");
    Coroutine.AddTask(-1, Task6, &num, "Task5-2");

    for (int i = 0; i < inter->thread_count; i++) {
        CreateThread(
            NULL,         // 默认安全属性
            0,            // 使用默认堆栈大小
            ThreadProc,   // 线程函数
            NULL,         // 传递给线程函数的参数
            0,            // 使用默认创建标志
            NULL);        // 返回线程ID
    }

    while (true) {
        Sleep(100);
    }
    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
