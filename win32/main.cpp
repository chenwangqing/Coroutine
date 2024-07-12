﻿// Coroutine.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "Coroutine.h"
#include "Coroutine.hpp"
#include <string>
#include <windows.h>
#include <functional>
#include <thread>

#undef CreateSemaphore
#undef CreateMutex
#undef Yield

#define EN_SLEEP 1

extern void         PrintMemory(void);
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
        std::string str = "hello";
        int         ms  = (rand() % 550) + 250;
        uint64_t    ts  = Task1_func1(ms);
        str += std::to_string(i);
        printf("[%llu][1][%llu/%d]i = %d %s\n", Coroutine.GetMillisecond(), ts, ms, i++, str.c_str());
#if 01
        char *buf = (char *)Coroutine.Malloc(str.size() + 1, __FILE__, __LINE__);
        memcpy(buf, str.c_str(), str.size() + 1);
        if (!Coroutine.SendMail(mail1, i & 0xFF, (uint64_t)buf, str.size() + 1, 1000))
            Coroutine.Free(buf, __FILE__, __LINE__);
#endif
#if EN_SLEEP
        Sleep(10);
#endif
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
        int         ms  = (rand() % 150) + 250;
        uint64_t    ts  = Coroutine.GetMillisecond();
        Coroutine.YieldDelay(ms);
        ts = Coroutine.GetMillisecond() - ts;
        str += std::to_string(i);
        printf("[%llu][2][%llu/%d]i = %d %s\n", Coroutine.GetMillisecond(), ts, ms, i++, str.c_str());
        Coroutine.GiveSemaphore(sem1, 1);
#if EN_SLEEP
        Sleep(2);
#endif
    }
    return;
}

void Task3(void *obj)
{
    auto func = [](void *obj) -> void * {
        int     *a   = (int *)obj;
        uint64_t now = Coroutine.GetMillisecond();
#if EN_SLEEP
        Sleep(2);
#endif
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
#if EN_SLEEP
        Sleep(1);
#endif
        if (re && Coroutine.AsyncWait(re, 100)) {
            int *b = (int *)Coroutine.AsyncGetResultAndDelete(re);
            printf("[%llu][3]async result: %d/%d\n", Coroutine.GetMillisecond(), b[0], b[1]);
            re = nullptr;
        }
        if (!re) {
            re = Coroutine.Async(func, a, 0);
        }
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
        if (!data.isOk || data.size == 0) continue;
        printf("[%llu][4]mail1 recv: %llu %.*s\n",
               Coroutine.GetMillisecond(),
               data.id,
               data.size,
               (char *)data.data);
        Coroutine.Free((void *)data.data, __FILE__, __LINE__);
#if EN_SLEEP
        Sleep(2);
#endif
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

volatile uint64_t g_count  = 0;
volatile uint64_t g_count2 = 0;

static void Task7(void *obj)
{
    while (true) {
        g_count++;
        Coroutine.Yield();
    }
    return;
}

DWORD WINAPI ThreadProc_test2(LPVOID lpParam)
{
    uint64_t last_count  = 0;
    uint64_t last_count2 = 0;
    while (true) {
        Sleep(1000);
        uint64_t t = g_count, a = g_count2;
        uint64_t tv = t - last_count, tv2 = a - last_count2;
        last_count  = t;
        last_count2 = a;
        printf("[%llu][7]count = %llu(%llu) %llu(%llu)\n",
               Coroutine.GetMillisecond(),
               tv,
               t,
               tv2,
               a);
    }
}

static void Task_Channel1(void *obj)
{
    uint64_t num = 0;
    uint64_t now = Coroutine.GetMillisecond();
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

DWORD WINAPI ThreadProc_test(LPVOID lpParam)
{
    while (true) {
        g_count2++;
        Sleep(0);
    }
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
    while (true)
        Coroutine.RunTick();
    return 0;
}

static void Task10(void)
{
    int  num     = 0;
    auto ch      = new CO::Channel<int>("ch2");
    auto ch_func = [](CO::Channel<int> *ch) -> void {
        while (true) {
            Coroutine.FeedDog(30 * 1000);
            int num = ch->Read();
            ch->Write(num + 1);
        }
    };
    CO::Task::Start("ch_func", ch_func, ch);
    while (true) {
        Coroutine.FeedDog(30 * 1000);
        uint64_t ts = Coroutine.GetMillisecond();
        uint32_t ms = rand() % 2000 + 0;
        int      a = rand(), b = rand();
        CO::Task::Sleep(ms);
        ts        = Coroutine.GetMillisecond() - ts;
        auto func = [](int a, int b) -> int {
            return a + b;
        };
        CO::Async<int> tmp(0, func, a, b);
        ch->Write(num + 1);
        tmp.Wait();
        num = ch->Read();
        printf("[%llu][10]sleep %llu/%d %d + %d = %d num = %d\n",
               Coroutine.GetMillisecond(),
               ts,
               ms,
               a,
               b,
               tmp.GetResult(),
               num);
    }
}

int main()
{
    extern const Coroutine_Inter *GetInter(void);
    auto                          inter = GetInter();
    Coroutine.SetInter(inter);

    for (int i = 0; i < inter->thread_count; i++) {
        CreateThread(
            NULL,         // 默认安全属性
            0,            // 使用默认堆栈大小
            ThreadProc,   // 线程函数
            NULL,         // 传递给线程函数的参数
            0,            // 使用默认创建标志
            NULL);        // 返回线程ID
    }

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

    Coroutine.AddTask(Task_Channel1, nullptr, "Channel1", &atr);
    Coroutine.AddTask(Task_Channel2, nullptr, "Channel2", &atr);

    CO::Task::SetDefaultTaskStackSize(16 << 10);
    CO::Task::Start("Task10", Task10);

    //  CreateThread(NULL, 0, ThreadProc_test, NULL, 0, NULL);
    CreateThread(NULL, 0, ThreadProc_test2, NULL, 0, NULL);
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
