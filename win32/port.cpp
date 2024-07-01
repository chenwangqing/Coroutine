
#include <stdlib.h>
#include "Coroutine.h"

#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "ws2_32.lib")
#include <Windows.h>
#include <Mmsystem.h>
#include <process.h>
#include <dbghelp.h>
#include <time.h>
#include <sys/timeb.h>
#include <io.h>
#include <WS2tcpip.h>
#include <processthreadsapi.h>

static CRITICAL_SECTION critical_section;
static CRITICAL_SECTION memory_critical_section;

struct
{
    int64_t used;
    int64_t max_used;
} memory;

static void _Lock(const char *file, int line)
{
    EnterCriticalSection(&critical_section);
    return;
}

static void _Unlock(const char *file, int line)
{
    LeaveCriticalSection(&critical_section);
    return;
}

static void _Free(void *ptr, const char *file, int line)
{
    if (ptr == nullptr)
        return;
    size_t *p = (size_t *)ptr;
    p--;
    EnterCriticalSection(&critical_section);
    memory.used -= (*p) + sizeof(size_t);
    if (memory.used < 0) {
        while (true);
    }
    LeaveCriticalSection(&critical_section);
    free(p);
    return;
}

static void *_Malloc(size_t size, const char *file, int line)
{
    size_t *ptr = (size_t *)malloc(size + sizeof(size_t));
    *ptr        = size;
    EnterCriticalSection(&critical_section);
    memory.used += size + sizeof(size_t);
    if (memory.used > memory.max_used)
        memory.max_used = memory.used;
    LeaveCriticalSection(&critical_section);
    return ptr + 1;
}

static uint64_t GetMillisecond()
{
    return timeGetTime();
}

static size_t GetThreadId(void)
{
    return GetCurrentThreadId();
}

static void Coroutine_WatchdogTimeout(void *object, Coroutine_TaskId taskId, const char *name)
{
    while (true) {
        printf("\nWatchdogTimeout: %p %s\n", taskId, name);
        Sleep(1000);
    }
}

#define MAX_THREADS 3

static HANDLE sem_sleep[MAX_THREADS];
static bool   is_sem_sleep[MAX_THREADS];

static Coroutine_Events events = {
    nullptr,
    nullptr,
    nullptr,
    [](uint32_t time, void *object) -> void {
        // Sleep(1); 模拟休眠
        int idx = Coroutine.GetCurrentCoroutineIdx();
        EnterCriticalSection(&critical_section);
        is_sem_sleep[idx] = 1;
        LeaveCriticalSection(&critical_section);
        WaitForSingleObject(sem_sleep[idx], time);
        EnterCriticalSection(&critical_section);
        is_sem_sleep[idx] = 0;
        LeaveCriticalSection(&critical_section);
        return;
    },
    [](uint16_t co_id, void *object) -> void {
        EnterCriticalSection(&critical_section);
        bool isWait = is_sem_sleep[co_id];
        LeaveCriticalSection(&critical_section);
        if (isWait)
            ReleaseSemaphore(sem_sleep[co_id], 1, NULL);
        return;
    },
    Coroutine_WatchdogTimeout,
};

static const Coroutine_Inter Inter = {
    MAX_THREADS,
    _Lock,
    _Unlock,
    _Malloc,
    _Free,
    GetMillisecond,
    GetThreadId,
    &events,
};

const Coroutine_Inter *GetInter(void)
{
    InitializeCriticalSection(&critical_section);
    InitializeCriticalSection(&memory_critical_section);
    for (int i = 0; i < MAX_THREADS; i++) {
        sem_sleep[i] = CreateSemaphore(
            NULL,    // 默认安全属性
            0,       // 初始计数
            1024,    // 最大计数
            NULL);   // 未命名信号量
        is_sem_sleep[i] = false;
    }
    return &Inter;
}

void PrintMemory(void)
{
    printf("Memory used: %lld bytes, max used: %lld bytes\n", memory.used, memory.max_used);
    return;
}
