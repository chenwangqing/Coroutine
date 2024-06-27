// Coroutine.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "Coroutine.h"
#include <string>

extern void         PrintMemory(void);
Coroutine_Semaphore sem1;

void Task1(Coroutine_Handle coroutine, void *obj)
{
    int i = 0;
    while (true) {
        PrintMemory();
        std::string str = "hello";
        uint64_t    ts  = Coroutine.GetMillisecond();
        Coroutine.YieldDelay(coroutine, 1000);
        ts = Coroutine.GetMillisecond() - ts;
        str += std::to_string(i);
        printf("[%llu][1][%llu]i = %d %s\n", Coroutine.GetMillisecond(), ts, i++, str.c_str());
    }
    return;
}

void Task2(Coroutine_Handle coroutine, void *obj)
{
    int i = 0;
    printf("start: %llx", Coroutine.GetMillisecond());
    while (true) {
        std::string str = "hello";
        uint64_t ts = Coroutine.GetMillisecond();
        Coroutine.YieldDelay(coroutine, 500);
        ts = Coroutine.GetMillisecond() - ts;
        str += std::to_string(i);
        printf("[%llu][2][%llu]i = %d %s\n",Coroutine.GetMillisecond(),ts , i++,str.c_str());
         Coroutine.GiveSemaphore(sem1, 1);
    }
    return;
}

void Task3(Coroutine_Handle coroutine, void *obj)
{
    while (true) {
        if (!Coroutine.WaitSemaphore(coroutine, sem1, 1, 100))
            continue;
        printf("[%llu][3]sem1 signal\n", Coroutine.GetMillisecond());
    }
    return;
}

int main()
{
    extern const Coroutine_Inter  *GetInter(void);
    extern const Coroutine_Events *GetEvents(void);
    auto                           inter  = GetInter();
    auto                           events = GetEvents();
    Coroutine.SetInter(inter);

    Coroutine_Handle coroutine = Coroutine.Create(events);

    sem1 = Coroutine.CreateSemaphore(0);

   Coroutine.AddTask(coroutine, Task1, nullptr);
    Coroutine.AddTask(coroutine, Task2, nullptr);
    Coroutine.AddTask(coroutine, Task3, nullptr);

    volatile void* ptr = &coroutine;
    printf("%p\n", ptr);
    while (true)
        Coroutine.RunTick(coroutine);
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
