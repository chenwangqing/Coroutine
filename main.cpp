// Coroutine.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "Coroutine.h"
#include <string>
#include <windows.h>

#undef CreateSemaphore

extern void         PrintMemory(void);
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
        Coroutine_MailData *data = Coroutine.MakeMessage(i & 0xFF, str.c_str(), str.size(), 1000);
        if (!Coroutine.SendMail(mail1, data))
            Coroutine.DeleteMessage(data);
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
    while (true) {
        if (!Coroutine.WaitSemaphore(sem1, 1, 100))
            continue;
        printf("[%llu][3]sem1 signal\n", Coroutine.GetMillisecond());
        Sleep(1);
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

int main()
{
    extern const Coroutine_Inter *GetInter(void);
    auto                          inter = GetInter();
    Coroutine.SetInter(inter);

    sem1  = Coroutine.CreateSemaphore("sem1", 0);
    mail1 = Coroutine.CreateMailbox("mail1", 1024);

    Coroutine.AddTask(0, Task1, nullptr, "Task1");
    Coroutine.AddTask(0, Task2, nullptr, "Task2");
    Coroutine.AddTask(0, Task3, nullptr, "Task3");
    Coroutine.AddTask(0, Task4, nullptr, "Task4");

    while (true) {
        Coroutine.RunTick();
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
