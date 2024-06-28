
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
    uint32_t used;
    uint32_t max_used;
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
    size_t *p = (size_t *)ptr;
    p--;
    EnterCriticalSection(&critical_section);
    memory.used -= *p + sizeof(size_t);
    LeaveCriticalSection(&critical_section);
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

static Coroutine_Events events = {
    nullptr,
    nullptr,
    nullptr,
    [](uint32_t time, void *object) -> void {
        Sleep(1);
        return;
    },
    nullptr,
};

static const Coroutine_Inter Inter = {
    2,
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
    return &Inter;
}

void PrintMemory(void)
{
    printf("Memory used: %d bytes, max used: %d bytes\n", memory.used, memory.max_used);
    return;
}
